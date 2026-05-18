use std::collections::BTreeMap;
use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};

const ANOMALOUS_EXTENSIONS: &[&str] = &["att1", "ozj2", "rar"];

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct InventoryReport {
    pub source_root: Utf8PathBuf,
    pub total_files: usize,
    pub by_extension: BTreeMap<String, usize>,
    pub by_top_level_dir: BTreeMap<String, usize>,
    pub rejected: Vec<RejectedAsset>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct RejectedAsset {
    pub path: Utf8PathBuf,
    pub reason: RejectionReason,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RejectionReason {
    AnomalousExtension,
    AnomalousWorldFixture,
    NonUtf8Path,
}

impl RejectionReason {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::AnomalousExtension => "anomalous-extension",
            Self::AnomalousWorldFixture => "anomalous-world-fixture",
            Self::NonUtf8Path => "non-utf8-path",
        }
    }
}

pub fn scan_inventory(source_root: &Utf8Path) -> io::Result<InventoryReport> {
    let mut report = InventoryReport {
        source_root: source_root.to_path_buf(),
        total_files: 0,
        by_extension: BTreeMap::new(),
        by_top_level_dir: BTreeMap::new(),
        rejected: Vec::new(),
    };

    visit_dir(source_root, source_root, &mut report)?;
    Ok(report)
}

pub fn write_report(report: &InventoryReport, output: &Utf8Path) -> io::Result<()> {
    fs::write(output, format_report(report))
}

pub fn format_report(report: &InventoryReport) -> String {
    let mut out = String::new();

    out.push_str(&format!("source_root: {}\n", report.source_root));
    out.push_str(&format!("total_files: {}\n", report.total_files));
    out.push_str("\n[extensions]\n");
    for (extension, count) in &report.by_extension {
        out.push_str(&format!("{extension}: {count}\n"));
    }

    out.push_str("\n[top_level_dirs]\n");
    for (dir, count) in &report.by_top_level_dir {
        out.push_str(&format!("{dir}: {count}\n"));
    }

    out.push_str("\n[rejected]\n");
    for rejected in &report.rejected {
        out.push_str(&format!(
            "{}: {}\n",
            rejected.reason.as_str(),
            rejected.path
        ));
    }

    out
}

fn visit_dir(root: &Utf8Path, current: &Utf8Path, report: &mut InventoryReport) -> io::Result<()> {
    for entry in fs::read_dir(current)? {
        let entry = entry?;
        let path = match Utf8PathBuf::from_path_buf(entry.path()) {
            Ok(path) => path,
            Err(path) => {
                report.rejected.push(RejectedAsset {
                    path: path.to_string_lossy().into_owned().into(),
                    reason: RejectionReason::NonUtf8Path,
                });
                continue;
            }
        };

        if path.is_dir() {
            visit_dir(root, &path, report)?;
            continue;
        }

        if !path.is_file() {
            continue;
        }

        record_file(root, &path, report);
    }

    Ok(())
}

fn record_file(root: &Utf8Path, path: &Utf8Path, report: &mut InventoryReport) {
    report.total_files += 1;

    let relative = path.strip_prefix(root).unwrap_or(path);
    let extension = normalized_extension(path);
    *report.by_extension.entry(extension.clone()).or_default() += 1;

    if let Some(top_level_dir) = relative.components().next() {
        *report
            .by_top_level_dir
            .entry(top_level_dir.as_str().to_ascii_lowercase())
            .or_default() += 1;
    }

    if ANOMALOUS_EXTENSIONS.contains(&extension.as_str()) {
        report.rejected.push(RejectedAsset {
            path: relative.to_path_buf(),
            reason: RejectionReason::AnomalousExtension,
        });
        return;
    }

    if relative.as_str().eq_ignore_ascii_case("world1/xxx") {
        report.rejected.push(RejectedAsset {
            path: relative.to_path_buf(),
            reason: RejectionReason::AnomalousWorldFixture,
        });
    }
}

fn normalized_extension(path: &Utf8Path) -> String {
    path.extension()
        .map(str::to_ascii_lowercase)
        .unwrap_or_else(|| "<none>".to_owned())
}

#[cfg(test)]
mod tests {
    use std::fs::{self, File};

    use camino::Utf8PathBuf;

    use super::{format_report, scan_inventory, RejectionReason};

    #[test]
    fn scanner_counts_extensions_dirs_and_rejections() {
        let root = test_root();
        fs::create_dir_all(root.join("World1")).unwrap();
        fs::create_dir_all(root.join("Object1")).unwrap();
        File::create(root.join("World1").join("Terrain.MAP")).unwrap();
        File::create(root.join("World1").join("EncTerrain1.att1")).unwrap();
        File::create(root.join("World1").join("xxx")).unwrap();
        File::create(root.join("Object1").join("Object01.bmd")).unwrap();

        let report = scan_inventory(&root).unwrap();

        assert_eq!(report.total_files, 4);
        assert_eq!(report.by_extension["map"], 1);
        assert_eq!(report.by_extension["att1"], 1);
        assert_eq!(report.by_extension["<none>"], 1);
        assert_eq!(report.by_top_level_dir["world1"], 3);
        assert_eq!(report.by_top_level_dir["object1"], 1);
        assert!(report
            .rejected
            .iter()
            .any(|entry| entry.reason == RejectionReason::AnomalousExtension));
        assert!(report
            .rejected
            .iter()
            .any(|entry| entry.reason == RejectionReason::AnomalousWorldFixture));

        let formatted = format_report(&report);
        assert!(formatted.contains("[extensions]"));
        assert!(formatted.contains("anomalous-extension: World1/EncTerrain1.att1"));
    }

    fn test_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_asset_inventory_{}_{}",
            std::process::id(),
            unique_suffix()
        ));
        let root = Utf8PathBuf::from_path_buf(root).unwrap();
        fs::create_dir_all(&root).unwrap();
        root
    }

    fn unique_suffix() -> u128 {
        std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_nanos()
    }
}
