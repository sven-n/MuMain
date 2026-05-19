use std::io::{self, Write};
use std::process::ExitCode;

use crate::state::boot;
use crate::{control_http, logging, AppState, Cli};

pub fn run(cli: Cli) -> ExitCode {
    let (state, asset_error) = boot(&cli);

    if let Some(error) = asset_error {
        logging::error(
            logging::COMPONENT_BOOT,
            logging::ERROR_ID_ASSET_VALIDATION_FAILED,
            &error,
        );
        eprintln!("asset validation failed: {error}");
    }

    if let Some(address) = cli.control_http {
        if let Err(error) = write_line(state.as_str()) {
            logging::error(
                logging::COMPONENT_RUNTIME,
                logging::ERROR_ID_CLIENT_STATE_WRITE_FAILED,
                &error,
            );
            eprintln!("failed to write client state: {error}");
            return ExitCode::from(1);
        }

        match control_http::spawn(address, state) {
            Ok(handle) => {
                if let Err(error) = write_line(&format!(
                    "control-http listening on http://{}",
                    handle.address()
                )) {
                    logging::error(
                        logging::COMPONENT_CONTROL_HTTP,
                        logging::ERROR_ID_LISTENING_ADDRESS_WRITE_FAILED,
                        &error,
                    );
                    eprintln!("failed to write control-http address: {error}");
                    handle.request_shutdown();
                    let _ = handle.join();
                    return ExitCode::from(1);
                }

                match handle.join() {
                    Ok(_) => ExitCode::SUCCESS,
                    Err(error) => {
                        logging::error(
                            logging::COMPONENT_CONTROL_HTTP,
                            logging::ERROR_ID_SERVER_FAILED,
                            &error,
                        );
                        eprintln!("control-http server failed: {error}");
                        ExitCode::from(1)
                    }
                }
            }
            Err(error) => {
                logging::error(
                    logging::COMPONENT_CONTROL_HTTP,
                    logging::ERROR_ID_BIND_FAILED,
                    &error,
                );
                eprintln!("control-http bind failed: {error}");
                ExitCode::from(1)
            }
        }
    } else {
        if let Err(error) = write_line(state.as_str()) {
            logging::error(
                logging::COMPONENT_RUNTIME,
                logging::ERROR_ID_CLIENT_STATE_WRITE_FAILED,
                &error,
            );
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
