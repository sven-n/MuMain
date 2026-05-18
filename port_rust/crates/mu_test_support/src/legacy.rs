use crate::evidence::{EvidenceArtifact, EvidenceKind, EvidencePaths};
use crate::fixtures::{artifact_file_name, slugify};
use camino::{Utf8Path, Utf8PathBuf};

const DIFFS_DIR: &str = "diffs";
const LEGACY_DIR: &str = "legacy";
const RUST_DIR: &str = "rust";
const DIFF_EXTENSION: &str = "diff";

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct LegacyComparisonPaths {
    case_root: Utf8PathBuf,
    legacy: EvidencePaths,
    rust: EvidencePaths,
}

impl LegacyComparisonPaths {
    pub fn new(root: impl Into<Utf8PathBuf>, case_name: impl AsRef<str>) -> Self {
        let case_root = root.into().join(slugify(case_name.as_ref()));
        Self::from_case_root(case_root)
    }

    pub fn from_case_root(case_root: impl Into<Utf8PathBuf>) -> Self {
        let case_root = case_root.into();
        let legacy = EvidencePaths::from_case_root(case_root.join(LEGACY_DIR));
        let rust = EvidencePaths::from_case_root(case_root.join(RUST_DIR));

        Self {
            case_root,
            legacy,
            rust,
        }
    }

    pub fn case_root(&self) -> &Utf8Path {
        self.case_root.as_ref()
    }

    pub fn legacy(&self) -> &EvidencePaths {
        &self.legacy
    }

    pub fn rust(&self) -> &EvidencePaths {
        &self.rust
    }

    pub fn diffs_dir(&self) -> Utf8PathBuf {
        self.case_root.join(DIFFS_DIR)
    }

    pub fn diff(&self, label: impl AsRef<str>) -> EvidenceArtifact {
        let label = label.as_ref();
        EvidenceArtifact {
            kind: EvidenceKind::Diff,
            label: label.to_string(),
            path: self
                .diffs_dir()
                .join(artifact_file_name(label, DIFF_EXTENSION)),
        }
    }

    pub fn legacy_log(&self, label: impl AsRef<str>) -> EvidenceArtifact {
        self.legacy.log(label)
    }

    pub fn rust_log(&self, label: impl AsRef<str>) -> EvidenceArtifact {
        self.rust.log(label)
    }

    pub fn legacy_screenshot(&self, label: impl AsRef<str>) -> EvidenceArtifact {
        self.legacy.screenshot(label)
    }

    pub fn rust_screenshot(&self, label: impl AsRef<str>) -> EvidenceArtifact {
        self.rust.screenshot(label)
    }

    pub fn legacy_report(
        &self,
        label: impl AsRef<str>,
        extension: impl AsRef<str>,
    ) -> EvidenceArtifact {
        self.legacy.report(label, extension)
    }

    pub fn rust_report(
        &self,
        label: impl AsRef<str>,
        extension: impl AsRef<str>,
    ) -> EvidenceArtifact {
        self.rust.report(label, extension)
    }
}

#[cfg(test)]
mod tests {
    use super::{EvidenceKind, LegacyComparisonPaths};
    use camino::Utf8Path;

    #[test]
    fn comparison_paths_partition_legacy_and_rust_outputs() {
        let paths = LegacyComparisonPaths::new("/tmp/comparisons", "Login/Main Window");

        assert_eq!(
            paths.case_root(),
            Utf8Path::new("/tmp/comparisons/login-main-window")
        );
        assert_eq!(
            paths.legacy().case_root(),
            Utf8Path::new("/tmp/comparisons/login-main-window/legacy")
        );
        assert_eq!(
            paths.rust().case_root(),
            Utf8Path::new("/tmp/comparisons/login-main-window/rust")
        );

        let legacy_log = paths.legacy_log("Boot Finished");
        assert_eq!(legacy_log.kind, EvidenceKind::Log);
        assert_eq!(
            legacy_log.path,
            Utf8Path::new("/tmp/comparisons/login-main-window/legacy/logs/boot-finished.log")
        );

        let rust_screenshot = paths.rust_screenshot("Loading Screen");
        assert_eq!(rust_screenshot.kind, EvidenceKind::Screenshot);
        assert_eq!(
            rust_screenshot.path,
            Utf8Path::new("/tmp/comparisons/login-main-window/rust/screenshots/loading-screen.png",)
        );

        let diff = paths.diff("Legacy vs Rust");
        assert_eq!(diff.kind, EvidenceKind::Diff);
        assert_eq!(
            diff.path,
            Utf8Path::new("/tmp/comparisons/login-main-window/diffs/legacy-vs-rust.diff")
        );
    }
}
