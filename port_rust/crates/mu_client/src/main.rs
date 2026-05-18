use std::process::ExitCode;

use clap::Parser;
use mu_app::{boot_state, AppState, Cli};

fn main() -> ExitCode {
    let cli = Cli::parse();
    let state = boot_state(&cli);

    println!("{}", state.as_str());

    match state {
        AppState::AssetCheckFailed => ExitCode::from(3),
        AppState::Boot | AppState::ReadyForLogin | AppState::Exit => ExitCode::SUCCESS,
    }
}
