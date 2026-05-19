pub mod redaction;

pub const CRATE_NAME: &str = env!("CARGO_PKG_NAME");

pub use redaction::{redact, Redacted, REDACTED};
