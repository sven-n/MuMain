use std::io::{self, Write};
use std::process::ExitCode;

use crate::{boot_state, control_http, AppState, Cli};

pub fn run(cli: Cli) -> ExitCode {
    let state = boot_state(&cli);

    if let Some(address) = cli.control_http {
        if let Err(error) = write_line(state.as_str()) {
            eprintln!("failed to write client state: {error}");
            return ExitCode::from(1);
        }

        match control_http::spawn(address, state) {
            Ok(handle) => {
                if let Err(error) = write_line(&format!(
                    "control-http listening on http://{}",
                    handle.address()
                )) {
                    eprintln!("failed to write control-http address: {error}");
                    handle.request_shutdown();
                    let _ = handle.join();
                    return ExitCode::from(1);
                }

                match handle.join() {
                    Ok(_) => ExitCode::SUCCESS,
                    Err(error) => {
                        eprintln!("control-http server failed: {error}");
                        ExitCode::from(1)
                    }
                }
            }
            Err(error) => {
                eprintln!("control-http bind failed: {error}");
                ExitCode::from(1)
            }
        }
    } else {
        if let Err(error) = write_line(state.as_str()) {
            eprintln!("failed to write client state: {error}");
            return ExitCode::from(1);
        }

        match state {
            AppState::AssetCheckFailed => ExitCode::from(3),
            AppState::Boot | AppState::ReadyForLogin | AppState::Exit => ExitCode::SUCCESS,
        }
    }
}

fn write_line(value: &str) -> io::Result<()> {
    let mut stdout = io::stdout().lock();
    writeln!(stdout, "{value}")?;
    stdout.flush()
}
