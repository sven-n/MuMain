use std::env;
use std::process::ExitCode;

use port_rust::status_message;

fn main() -> ExitCode {
    let mut args = env::args().skip(1);

    match args.next().as_deref() {
        Some("--status") if args.next().is_none() => {
            println!("{}", status_message());
            ExitCode::SUCCESS
        }
        _ => {
            eprintln!("usage: port_rust --status");
            ExitCode::from(2)
        }
    }
}
