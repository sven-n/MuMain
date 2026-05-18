use mu_assets::validate_asset_root;

use crate::Cli;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AppState {
    Boot,
    AssetCheckFailed,
    ReadyForLogin,
    Exit,
}

impl AppState {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Boot => "boot",
            Self::AssetCheckFailed => "asset-check-failed",
            Self::ReadyForLogin => "ready-for-login",
            Self::Exit => "exit",
        }
    }
}

pub(crate) fn boot(cli: &Cli) -> (AppState, Option<String>) {
    if let Some(asset_root) = &cli.asset_root {
        if let Err(error) = validate_asset_root(asset_root) {
            return (AppState::AssetCheckFailed, Some(error.to_string()));
        }
    }

    if cli.headless {
        return (AppState::ReadyForLogin, None);
    }

    (AppState::Exit, None)
}

pub fn boot_state(cli: &Cli) -> AppState {
    boot(cli).0
}

#[cfg(test)]
mod tests {
    use super::{boot_state, AppState};
    use crate::Cli;
    use camino::Utf8PathBuf;
    use mu_assets::{
        sha256_hex, AssetManifest, AssetManifestEntry, MANIFEST_FILE_NAME, SUPPORTED_SCHEMA_VERSION,
    };
    use std::fs;

    fn cli() -> Cli {
        Cli {
            asset_root: None,
            server: None,
            config: None,
            editor_admin: false,
            offline_fixture: None,
            headless: true,
            control_http: None,
            evidence_dir: None,
        }
    }

    fn temp_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_app_boot_{}_{}",
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

    fn write_valid_manifest(root: &Utf8PathBuf) {
        let converted_path = root.join("converted/boot.bin");
        fs::create_dir_all(converted_path.parent().unwrap()).unwrap();
        fs::write(&converted_path, b"boot-bytes").unwrap();

        let manifest = AssetManifest {
            schema_version: SUPPORTED_SCHEMA_VERSION,
            source_client_version: "1.0.0".to_string(),
            content_hash: String::new(),
            generated_at: "2026-05-18T00:00:00Z".to_string(),
            entries: vec![AssetManifestEntry {
                source_path: "legacy/boot.bin".to_string(),
                converted_path: "converted/boot.bin".to_string(),
                kind: "data".to_string(),
                source_hash: sha256_hex(b"legacy-boot"),
                converted_hash: sha256_hex(b"boot-bytes"),
                dependencies: vec![],
            }],
        }
        .with_computed_content_hash();

        fs::write(
            root.join(MANIFEST_FILE_NAME),
            manifest.to_json_string().unwrap(),
        )
        .unwrap();
    }

    #[test]
    fn headless_boot_reaches_login_state() {
        assert_eq!(boot_state(&cli()), AppState::ReadyForLogin);
    }

    #[test]
    fn missing_asset_root_blocks_login() {
        let mut cli = cli();
        cli.asset_root = Some("__missing_mu_asset_root__".into());

        assert_eq!(boot_state(&cli), AppState::AssetCheckFailed);
    }

    #[test]
    fn missing_manifest_blocks_login() {
        let mut cli = cli();
        cli.asset_root = Some(temp_root());

        assert_eq!(boot_state(&cli), AppState::AssetCheckFailed);
    }

    #[test]
    fn valid_manifest_allows_headless_boot() {
        let mut cli = cli();
        let root = temp_root();
        write_valid_manifest(&root);
        cli.asset_root = Some(root);

        assert_eq!(boot_state(&cli), AppState::ReadyForLogin);
    }

    #[test]
    fn non_headless_boot_exits_until_bevy_runtime_exists() {
        let mut cli = cli();
        cli.headless = false;

        assert_eq!(boot_state(&cli), AppState::Exit);
    }
}
