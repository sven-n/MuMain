use camino::Utf8PathBuf;
use clap::Parser;
use std::net::SocketAddr;

#[derive(Debug, Clone, Parser, PartialEq, Eq)]
#[command(name = "mu_client")]
pub struct Cli {
    #[arg(long, env = "MU_ASSET_ROOT")]
    pub asset_root: Option<Utf8PathBuf>,

    #[arg(long, env = "MU_SERVER")]
    pub server: Option<String>,

    #[arg(long, env = "MU_CONFIG")]
    pub config: Option<Utf8PathBuf>,

    #[arg(long)]
    pub editor_admin: bool,

    #[arg(long, env = "MU_OFFLINE_FIXTURE")]
    pub offline_fixture: Option<String>,

    #[arg(long)]
    pub headless: bool,

    #[arg(long, value_name = "ADDR")]
    pub control_http: Option<SocketAddr>,

    #[arg(long, env = "MU_E2E_ARTIFACT_DIR")]
    pub evidence_dir: Option<Utf8PathBuf>,
}

impl Cli {
    pub fn config_path(&self) -> Utf8PathBuf {
        self.config
            .clone()
            .unwrap_or_else(crate::config::Config::default_path)
    }
}

#[cfg(test)]
mod tests {
    use super::Cli;
    use crate::config::Config;
    use camino::Utf8PathBuf;
    use clap::Parser;
    use std::ffi::OsString;
    use std::net::SocketAddr;
    use std::sync::{Mutex, MutexGuard, OnceLock};

    static ENV_LOCK: OnceLock<Mutex<()>> = OnceLock::new();

    fn env_lock() -> MutexGuard<'static, ()> {
        ENV_LOCK.get_or_init(|| Mutex::new(())).lock().unwrap()
    }

    struct EnvVar {
        key: &'static str,
        previous: Option<OsString>,
    }

    impl EnvVar {
        fn set(key: &'static str, value: &str) -> Self {
            let previous = std::env::var_os(key);
            std::env::set_var(key, value);
            Self { key, previous }
        }
    }

    impl Drop for EnvVar {
        fn drop(&mut self) {
            match self.previous.take() {
                Some(previous) => std::env::set_var(self.key, previous),
                None => std::env::remove_var(self.key),
            }
        }
    }

    #[test]
    fn parses_control_http_address() {
        let cli = Cli::try_parse_from([
            "mu_client",
            "--headless",
            "--control-http",
            "127.0.0.1:12345",
        ])
        .unwrap();

        assert_eq!(
            cli.control_http,
            Some("127.0.0.1:12345".parse::<SocketAddr>().unwrap())
        );
        assert!(cli.headless);
    }

    #[test]
    fn env_values_fill_missing_cli_arguments() {
        let _lock = env_lock();
        let _asset_root = EnvVar::set("MU_ASSET_ROOT", "/env/assets");
        let _server = EnvVar::set("MU_SERVER", "env.example:44405");
        let _config = EnvVar::set("MU_CONFIG", "/env/config/client.toml");
        let _offline_fixture = EnvVar::set("MU_OFFLINE_FIXTURE", "env-fixture");
        let _evidence_dir = EnvVar::set("MU_E2E_ARTIFACT_DIR", "/env/evidence");

        let cli = Cli::try_parse_from(["mu_client"]).unwrap();

        assert_eq!(cli.asset_root, Some(Utf8PathBuf::from("/env/assets")));
        assert_eq!(cli.server.as_deref(), Some("env.example:44405"));
        assert_eq!(
            cli.config_path(),
            Utf8PathBuf::from("/env/config/client.toml")
        );
        assert_eq!(cli.offline_fixture.as_deref(), Some("env-fixture"));
        assert_eq!(cli.evidence_dir, Some(Utf8PathBuf::from("/env/evidence")));
    }

    #[test]
    fn cli_arguments_override_environment_values() {
        let _lock = env_lock();
        let _asset_root = EnvVar::set("MU_ASSET_ROOT", "/env/assets");
        let _server = EnvVar::set("MU_SERVER", "env.example:44405");
        let _config = EnvVar::set("MU_CONFIG", "/env/config/client.toml");
        let _offline_fixture = EnvVar::set("MU_OFFLINE_FIXTURE", "env-fixture");
        let _evidence_dir = EnvVar::set("MU_E2E_ARTIFACT_DIR", "/env/evidence");

        let cli = Cli::try_parse_from([
            "mu_client",
            "--asset-root",
            "/cli/assets",
            "--server",
            "cli.example:44406",
            "--config",
            "/cli/config/client.toml",
            "--offline-fixture",
            "cli-fixture",
            "--evidence-dir",
            "/cli/evidence",
        ])
        .unwrap();

        assert_eq!(cli.asset_root, Some(Utf8PathBuf::from("/cli/assets")));
        assert_eq!(cli.server.as_deref(), Some("cli.example:44406"));
        assert_eq!(
            cli.config_path(),
            Utf8PathBuf::from("/cli/config/client.toml")
        );
        assert_eq!(cli.offline_fixture.as_deref(), Some("cli-fixture"));
        assert_eq!(cli.evidence_dir, Some(Utf8PathBuf::from("/cli/evidence")));
    }

    #[test]
    fn config_path_defaults_to_client_toml() {
        let _lock = env_lock();
        let cli = Cli::try_parse_from(["mu_client"]).unwrap();

        assert_eq!(cli.config_path(), Config::default_path());
    }
}
