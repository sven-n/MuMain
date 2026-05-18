use std::process::ExitCode;

use camino::Utf8PathBuf;
use clap::Parser;
use mu_asset_pipeline::inventory::{format_report, scan_inventory, write_report};

#[derive(Debug, Parser)]
#[command(name = "mu_asset_pipeline")]
struct Cli {
    #[arg(long)]
    source: Utf8PathBuf,

    #[arg(long)]
    report: Option<Utf8PathBuf>,
}

fn main() -> ExitCode {
    let cli = Cli::parse();
    let report = match scan_inventory(&cli.source) {
        Ok(report) => report,
        Err(error) => {
            eprintln!("failed to scan assets: {error}");
            return ExitCode::from(2);
        }
    };

    if let Some(report_path) = &cli.report {
        if let Err(error) = write_report(&report, report_path) {
            eprintln!("failed to write report: {error}");
            return ExitCode::from(3);
        }
    } else {
        print!("{}", format_report(&report));
    }

    if report.rejected.is_empty() {
        ExitCode::SUCCESS
    } else {
        ExitCode::from(4)
    }
}
