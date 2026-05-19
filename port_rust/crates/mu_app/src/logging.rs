use std::fmt;

use mu_network::redaction::redact;

pub const COMPONENT_BOOT: &str = "boot";
pub const COMPONENT_CONTROL_HTTP: &str = "control-http";
pub const COMPONENT_RUNTIME: &str = "runtime";

pub const ERROR_ID_ASSET_VALIDATION_FAILED: &str = "asset-validation-failed";
pub const ERROR_ID_BIND_FAILED: &str = "control-http-bind-failed";
pub const ERROR_ID_CLIENT_STATE_WRITE_FAILED: &str = "client-state-write-failed";
pub const ERROR_ID_LISTENING_ADDRESS_WRITE_FAILED: &str = "control-http-address-write-failed";
pub const ERROR_ID_SERVER_FAILED: &str = "control-http-server-failed";

pub fn install() {
    let _ = tracing_subscriber::fmt()
        .with_ansi(false)
        .without_time()
        .with_target(false)
        .compact()
        .try_init();
}

pub fn error(component: &'static str, error_id: &'static str, message: impl fmt::Display) {
    tracing::error!(
        component = component,
        error_id = error_id,
        message = %message
    );
}

pub fn error_with_secret(
    component: &'static str,
    error_id: &'static str,
    message: impl fmt::Display,
    secret: impl AsRef<str>,
) {
    tracing::error!(
        component = component,
        error_id = error_id,
        message = %message,
        secret = %redact(secret.as_ref())
    );
}

#[cfg(test)]
mod tests {
    use super::{
        error, error_with_secret, install, COMPONENT_BOOT, COMPONENT_CONTROL_HTTP,
        ERROR_ID_ASSET_VALIDATION_FAILED, ERROR_ID_BIND_FAILED,
    };
    use std::io::{self, Write};
    use std::sync::{Arc, Mutex};
    use tracing_subscriber::fmt::MakeWriter;

    struct BufferWriter(Arc<Mutex<Vec<u8>>>);

    impl<'a> MakeWriter<'a> for BufferWriter {
        type Writer = BufferSink;

        fn make_writer(&'a self) -> Self::Writer {
            BufferSink(Arc::clone(&self.0))
        }
    }

    struct BufferSink(Arc<Mutex<Vec<u8>>>);

    impl Write for BufferSink {
        fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
            self.0
                .lock()
                .expect("log buffer poisoned")
                .extend_from_slice(buf);
            Ok(buf.len())
        }

        fn flush(&mut self) -> io::Result<()> {
            Ok(())
        }
    }

    fn capture_logs(f: impl FnOnce()) -> String {
        let buffer = Arc::new(Mutex::new(Vec::new()));
        let subscriber = tracing_subscriber::fmt()
            .with_ansi(false)
            .without_time()
            .with_target(false)
            .compact()
            .with_writer(BufferWriter(Arc::clone(&buffer)))
            .finish();

        tracing::subscriber::with_default(subscriber, f);

        let output = buffer.lock().expect("log buffer poisoned").clone();
        String::from_utf8(output).expect("log output is utf-8")
    }

    #[test]
    fn install_is_idempotent() {
        install();
        install();
    }

    #[test]
    fn structured_error_logs_include_component_and_error_id() {
        let output = capture_logs(|| {
            error(
                COMPONENT_BOOT,
                ERROR_ID_ASSET_VALIDATION_FAILED,
                "asset validation failed",
            );
        });

        assert!(output.contains("component=\"boot\""), "{output}");
        assert!(
            output.contains("error_id=\"asset-validation-failed\""),
            "{output}"
        );
        assert!(output.contains("asset validation failed"), "{output}");
    }

    #[test]
    fn secret_fields_are_redacted() {
        let output = capture_logs(|| {
            error_with_secret(
                COMPONENT_CONTROL_HTTP,
                ERROR_ID_BIND_FAILED,
                "control-http bind failed",
                "top-secret-token",
            );
        });

        assert!(output.contains("component=\"control-http\""), "{output}");
        assert!(
            output.contains("error_id=\"control-http-bind-failed\""),
            "{output}"
        );
        assert!(output.contains("[redacted]"), "{output}");
        assert!(!output.contains("top-secret-token"), "{output}");
    }
}
