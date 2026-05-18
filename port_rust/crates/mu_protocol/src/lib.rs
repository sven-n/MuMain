pub mod connect_server;

pub const CRATE_NAME: &str = env!("CARGO_PKG_NAME");

pub use connect_server::ServerEntry;
