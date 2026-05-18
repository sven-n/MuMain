use clap::Parser;
use mu_fake_server::{run, Cli};

fn main() -> std::process::ExitCode {
    match run(Cli::parse().into_config()) {
        Ok(()) => std::process::ExitCode::SUCCESS,
        Err(error) => {
            eprintln!("mu_fake_server failed: {error}");
            std::process::ExitCode::from(1)
        }
    }
}
