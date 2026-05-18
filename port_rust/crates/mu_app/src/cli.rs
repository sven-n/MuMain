use camino::Utf8PathBuf;
use clap::Parser;

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

    #[arg(long)]
    pub evidence_dir: Option<Utf8PathBuf>,
}
