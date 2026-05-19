pub mod codec;
pub mod connect_server;
pub mod error;
pub mod frame;

pub const CRATE_NAME: &str = env!("CARGO_PKG_NAME");

pub use codec::{decode_packet, encode_packet};
pub use connect_server::ServerEntry;
pub use error::PacketCodecError;
pub use frame::PacketFrame;
