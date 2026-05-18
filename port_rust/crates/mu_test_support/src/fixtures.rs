use camino::{Utf8Path, Utf8PathBuf};

const DEFAULT_SLUG: &str = "unnamed";
const GOLDENS_DIR: &str = "goldens";
const INPUTS_DIR: &str = "inputs";
const MANIFEST_FILE: &str = "manifest.toml";

pub fn slugify(input: &str) -> String {
    let mut slug = String::with_capacity(input.len());
    let mut pending_dash = false;

    for ch in input.chars() {
        let normalized = match ch {
            'a'..='z' | '0'..='9' => Some(ch),
            'A'..='Z' => Some(ch.to_ascii_lowercase()),
            _ => None,
        };

        if let Some(ch) = normalized {
            if pending_dash {
                slug.push('-');
                pending_dash = false;
            }
            slug.push(ch);
        } else if !slug.is_empty() {
            pending_dash = true;
        }
    }

    if slug.is_empty() {
        DEFAULT_SLUG.to_string()
    } else {
        slug
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct FixturePaths {
    case_root: Utf8PathBuf,
}

impl FixturePaths {
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

    pub fn inputs_dir(&self) -> Utf8PathBuf {
        self.case_root.join(INPUTS_DIR)
    }

    pub fn goldens_dir(&self) -> Utf8PathBuf {
        self.case_root.join(GOLDENS_DIR)
    }

    pub fn manifest_path(&self) -> Utf8PathBuf {
        self.case_root.join(MANIFEST_FILE)
    }

    pub fn input_path(&self, label: impl AsRef<str>, extension: impl AsRef<str>) -> Utf8PathBuf {
        self.inputs_dir()
            .join(artifact_file_name(label.as_ref(), extension.as_ref()))
    }

    pub fn golden_path(&self, label: impl AsRef<str>, extension: impl AsRef<str>) -> Utf8PathBuf {
        self.goldens_dir()
            .join(artifact_file_name(label.as_ref(), extension.as_ref()))
    }
}

pub(crate) fn artifact_file_name(label: &str, extension: &str) -> String {
    let extension = extension.trim_start_matches('.');
    let label = slugify(label);

    if extension.is_empty() {
        label
    } else {
        format!("{label}.{extension}")
    }
}

#[cfg(test)]
mod tests {
    use super::{artifact_file_name, slugify, FixturePaths};
    use camino::Utf8Path;

    #[test]
    fn slugify_normalizes_labels() {
        assert_eq!(slugify("Login/Main Window"), "login-main-window");
        assert_eq!(slugify("  mixed__Case!!  "), "mixed-case");
        assert_eq!(slugify(""), "unnamed");
    }

    #[test]
    fn fixture_paths_build_stable_case_layout() {
        let paths = FixturePaths::new("/tmp/fixtures", "Login/Main Window");

        assert_eq!(
            paths.case_root(),
            Utf8Path::new("/tmp/fixtures/login-main-window")
        );
        assert_eq!(
            paths.inputs_dir(),
            Utf8Path::new("/tmp/fixtures/login-main-window/inputs")
        );
        assert_eq!(
            paths.goldens_dir(),
            Utf8Path::new("/tmp/fixtures/login-main-window/goldens")
        );
        assert_eq!(
            paths.manifest_path(),
            Utf8Path::new("/tmp/fixtures/login-main-window/manifest.toml")
        );
        assert_eq!(
            paths.input_path("Captured Frame", ".png"),
            Utf8Path::new("/tmp/fixtures/login-main-window/inputs/captured-frame.png")
        );
        assert_eq!(
            paths.golden_path("State Snapshot", "json"),
            Utf8Path::new("/tmp/fixtures/login-main-window/goldens/state-snapshot.json")
        );
    }

    #[test]
    fn artifact_file_name_trims_extension_separators() {
        assert_eq!(artifact_file_name("Legacy Diff", ".txt"), "legacy-diff.txt");
    }
}
