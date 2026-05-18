use camino::Utf8PathBuf;
use clap::Parser;
use std::net::SocketAddr;

#[derive(Debug, Clone, Parser, PartialEq, Eq)]
#[command(name = "mu_client")]
pub struct Cli {
    #[arg(long)]
    pub asset_root: Option<Utf8PathBuf>,

    #[arg(long)]
    pub server: Option<String>,

    #[arg(long)]
    pub config: Option<Utf8PathBuf>,

    #[arg(long)]
    pub editor_admin: bool,

    #[arg(long)]
    pub offline_fixture: Option<String>,

    #[arg(long)]
    pub headless: bool,

    #[arg(long, value_name = "ADDR")]
    pub control_http: Option<SocketAddr>,

    #[arg(long)]
    pub evidence_dir: Option<Utf8PathBuf>,
}

#[cfg(test)]
mod tests {
    use super::Cli;
    use clap::Parser;
    use std::net::SocketAddr;

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
}
