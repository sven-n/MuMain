use clap::Parser;
use mu_app::{run, Cli};

fn main() -> std::process::ExitCode {
    run(Cli::parse())
}
