//! Stable evidence artifact layout for legacy-vs-Rust comparison tests.

use crate::fixtures::{artifact_file_name, slugify};
use camino::{Utf8Path, Utf8PathBuf};

const DIFFS_DIR: &str = "diffs";
const DIFF_EXTENSION: &str = "diff";
const LOGS_DIR: &str = "logs";
const LOG_EXTENSION: &str = "log";
const REPORTS_DIR: &str = "reports";
const SCREENSHOTS_DIR: &str = "screenshots";
const SCREENSHOT_EXTENSION: &str = "png";

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EvidenceKind {
    Diff,
    Log,
    Report,
    Screenshot,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct EvidenceArtifact {
    pub kind: EvidenceKind,
    pub label: String,
    pub path: Utf8PathBuf,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct EvidencePaths {
    case_root: Utf8PathBuf,
}

impl EvidencePaths {
    pub fn new(root: impl Into<Utf8PathBuf>, case_name: impl AsRef<str>) -> Self {
        let case_root = root.into().join(slugify(case_name.as_ref()));
        Self::from_case_root(case_root)
    }

    pub fn from_case_root(case_root: impl Into<Utf8PathBuf>) -> Self {
        Self {
            case_root: case_root.into(),
        }
    }

    pub fn case_root(&self) -> &Utf8Path {
        self.case_root.as_ref()
    }

    pub fn logs_dir(&self) -> Utf8PathBuf {
        self.case_root.join(LOGS_DIR)
    }

    pub fn screenshots_dir(&self) -> Utf8PathBuf {
        self.case_root.join(SCREENSHOTS_DIR)
    }

    pub fn reports_dir(&self) -> Utf8PathBuf {
        self.case_root.join(REPORTS_DIR)
    }

    pub fn diffs_dir(&self) -> Utf8PathBuf {
        self.case_root.join(DIFFS_DIR)
    }

    pub fn record(
        &self,
        kind: EvidenceKind,
        label: impl AsRef<str>,
        extension: impl AsRef<str>,
    ) -> EvidenceArtifact {
        let label = label.as_ref();
        let path = self
            .kind_dir(kind)
            .join(artifact_file_name(label, extension.as_ref()));

        EvidenceArtifact {
            kind,
            label: label.to_string(),
            path,
        }
    }

    pub fn log(&self, label: impl AsRef<str>) -> EvidenceArtifact {
        self.record(EvidenceKind::Log, label, LOG_EXTENSION)
    }

    pub fn screenshot(&self, label: impl AsRef<str>) -> EvidenceArtifact {
        self.record(EvidenceKind::Screenshot, label, SCREENSHOT_EXTENSION)
    }

    pub fn report(&self, label: impl AsRef<str>, extension: impl AsRef<str>) -> EvidenceArtifact {
        self.record(EvidenceKind::Report, label, extension)
    }

    pub fn diff(&self, label: impl AsRef<str>) -> EvidenceArtifact {
        self.record(EvidenceKind::Diff, label, DIFF_EXTENSION)
    }

    fn kind_dir(&self, kind: EvidenceKind) -> Utf8PathBuf {
        match kind {
            EvidenceKind::Diff => self.diffs_dir(),
            EvidenceKind::Log => self.logs_dir(),
            EvidenceKind::Report => self.reports_dir(),
            EvidenceKind::Screenshot => self.screenshots_dir(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{EvidenceKind, EvidencePaths};
    use camino::Utf8Path;

    #[test]
    fn evidence_paths_build_case_artifacts() {
        let paths = EvidencePaths::new("/tmp/evidence", "Login/Main Window");

        assert_eq!(
            paths.case_root(),
            Utf8Path::new("/tmp/evidence/login-main-window")
        );

        let log = paths.log("Boot Finished");
        assert_eq!(log.kind, EvidenceKind::Log);
        assert_eq!(log.label, "Boot Finished");
        assert_eq!(
            log.path,
            Utf8Path::new("/tmp/evidence/login-main-window/logs/boot-finished.log")
        );

        let screenshot = paths.screenshot("Loading Screen");
        assert_eq!(screenshot.kind, EvidenceKind::Screenshot);
        assert_eq!(
            screenshot.path,
            Utf8Path::new("/tmp/evidence/login-main-window/screenshots/loading-screen.png")
        );

        let report = paths.report("State Snapshot", ".json");
        assert_eq!(report.kind, EvidenceKind::Report);
        assert_eq!(
            report.path,
            Utf8Path::new("/tmp/evidence/login-main-window/reports/state-snapshot.json")
        );

        let diff = paths.diff("Legacy vs Rust");
        assert_eq!(diff.kind, EvidenceKind::Diff);
        assert_eq!(
            diff.path,
            Utf8Path::new("/tmp/evidence/login-main-window/diffs/legacy-vs-rust.diff")
        );
    }
}
