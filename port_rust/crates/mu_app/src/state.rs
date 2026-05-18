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

pub fn boot_state(cli: &Cli) -> AppState {
    if let Some(asset_root) = &cli.asset_root {
        if !asset_root.exists() {
            return AppState::AssetCheckFailed;
        }
    }

    if cli.headless {
        return AppState::ReadyForLogin;
    }

    AppState::Exit
}

#[cfg(test)]
mod tests {
    use super::{boot_state, AppState};
    use crate::Cli;

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
    fn non_headless_boot_exits_until_bevy_runtime_exists() {
        let mut cli = cli();
        cli.headless = false;

        assert_eq!(boot_state(&cli), AppState::Exit);
    }
}
