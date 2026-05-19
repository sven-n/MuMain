use std::collections::BTreeMap;
use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};

const TABLE_EXTENSIONS: &[&str] = &["bmd", "txt"];
const ASSET_EXTENSIONS: &[&str] = &[
    "bmp", "csr", "jpg", "jpeg", "mp3", "ozj", "ozt", "png", "tga", "wav",
];

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum GameDataKind {
    Table {
        stem: String,
        extension: String,
    },
    Asset {
        parent: Option<String>,
        extension: String,
    },
    Other {
        extension: String,
    },
}

impl GameDataKind {
    pub fn label(&self) -> &'static str {
        match self {
            Self::Table { .. } => "table",
            Self::Asset { .. } => "asset",
            Self::Other { .. } => "other",
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct GameDataFile {
    pub path: Utf8PathBuf,
    pub kind: GameDataKind,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct GameDataReport {
    pub source_root: Utf8PathBuf,
    pub total_files: usize,
    pub by_kind: BTreeMap<String, usize>,
    pub files: Vec<GameDataFile>,
}

pub fn scan_game_data(source_root: &Utf8Path) -> io::Result<GameDataReport> {
    let mut report = GameDataReport {
        source_root: source_root.to_path_buf(),
        total_files: 0,
        by_kind: BTreeMap::new(),
        files: Vec::new(),
    };

    visit_dir(source_root, source_root, &mut report)?;
    Ok(report)
}

pub fn classify_game_data_path(path: &Utf8Path) -> GameDataKind {
    let extension = normalized_extension(path);
    let lower_path = path.as_str().to_ascii_lowercase();

    if TABLE_EXTENSIONS.contains(&extension.as_str()) && !contains_part_charge(&lower_path) {
        return GameDataKind::Table {
            stem: normalized_stem(path),
            extension,
        };
    }

    if ASSET_EXTENSIONS.contains(&extension.as_str()) || contains_part_charge(&lower_path) {
        return GameDataKind::Asset {
            parent: path
                .parent()
                .and_then(|parent| parent.file_name())
                .map(|name| name.to_ascii_lowercase()),
            extension,
        };
    }

    GameDataKind::Other { extension }
}

pub fn format_game_data_report(report: &GameDataReport) -> String {
    let mut out = String::new();
    out.push_str(&format!("source_root: {}\n", report.source_root));
    out.push_str(&format!("total_files: {}\n", report.total_files));
    out.push_str("\n[kinds]\n");
    for (kind, count) in &report.by_kind {
        out.push_str(&format!("{kind}: {count}\n"));
    }
    out.push_str("\n[files]\n");
    for file in &report.files {
        out.push_str(&format!("{}: {}\n", file.kind.label(), file.path));
    }
    out
}

fn visit_dir(root: &Utf8Path, current: &Utf8Path, report: &mut GameDataReport) -> io::Result<()> {
    for entry in fs::read_dir(current)? {
        let entry = entry?;
        let path = match Utf8PathBuf::from_path_buf(entry.path()) {
            Ok(path) => path,
            Err(_) => continue,
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

fn record_file(root: &Utf8Path, path: &Utf8Path, report: &mut GameDataReport) {
    report.total_files += 1;

    let relative = path.strip_prefix(root).unwrap_or(path).to_path_buf();
    let kind = classify_game_data_path(&relative);
    *report.by_kind.entry(kind.label().to_owned()).or_default() += 1;
    report.files.push(GameDataFile {
        path: relative,
        kind,
    });
}

fn normalized_extension(path: &Utf8Path) -> String {
    path.extension()
        .map(str::to_ascii_lowercase)
        .unwrap_or_else(|| "<none>".to_owned())
}

fn normalized_stem(path: &Utf8Path) -> String {
    path.file_stem()
        .map(str::to_ascii_lowercase)
        .unwrap_or_else(|| "<none>".to_owned())
}

fn contains_part_charge(path: &str) -> bool {
    path.contains("/partcharge") || path.starts_with("partcharge")
}

#[cfg(test)]
mod tests {
    use std::fs::{self, File};

    use camino::Utf8PathBuf;

    use super::{classify_game_data_path, scan_game_data, GameDataKind};

    #[test]
    fn classifies_local_game_data_paths() {
        assert!(matches!(
            classify_game_data_path(&Utf8PathBuf::from("Local/Item.bmd")),
            GameDataKind::Table { stem, extension } if stem == "item" && extension == "bmd"
        ));

        assert!(matches!(
            classify_game_data_path(&Utf8PathBuf::from("Local/Eng/item_eng.bmd")),
            GameDataKind::Table { stem, extension } if stem == "item_eng" && extension == "bmd"
        ));

        assert!(matches!(
            classify_game_data_path(&Utf8PathBuf::from("Local/partCharge1/ShopUI.bmd")),
            GameDataKind::Asset { parent, extension } if parent.as_deref() == Some("partcharge1") && extension == "bmd"
        ));

        assert!(matches!(
            classify_game_data_path(&Utf8PathBuf::from("Local/everyone.ozj")),
            GameDataKind::Asset { extension, .. } if extension == "ozj"
        ));
    }

    #[test]
    fn scan_game_data_counts_tables_and_assets() {
        let root = test_root();
        fs::create_dir_all(root.join("Local")).unwrap();
        fs::create_dir_all(root.join("Local").join("Eng")).unwrap();
        fs::create_dir_all(root.join("Local").join("partCharge1")).unwrap();

        File::create(root.join("Local").join("Item.bmd")).unwrap();
        File::create(root.join("Local").join("Eng").join("item_eng.bmd")).unwrap();
        File::create(root.join("Local").join("Eng").join("NpcName_Eng.txt")).unwrap();
        File::create(root.join("Local").join("partCharge1").join("ShopUI.bmd")).unwrap();
        File::create(root.join("Local").join("everyone.ozj")).unwrap();

        let report = scan_game_data(&root).unwrap();

        assert_eq!(report.total_files, 5);
        assert_eq!(report.by_kind["table"], 3);
        assert_eq!(report.by_kind["asset"], 2);
        assert!(report
            .files
            .iter()
            .any(|entry| entry.path.as_str() == "Local/partCharge1/ShopUI.bmd"));
    }

    fn test_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_game_data_{}_{}",
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
