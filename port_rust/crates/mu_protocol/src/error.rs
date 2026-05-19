use std::error::Error;
use std::fmt;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PacketCodecError {
    EmptyPacket,
    HeaderTooShort { expected: usize, actual: usize },
    PacketTooSmall { size: usize, minimum: usize },
    PacketBodyTruncated { expected: usize, actual: usize },
    PacketTooLarge { size: usize },
}

impl fmt::Display for PacketCodecError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::EmptyPacket => f.write_str("packet is empty"),
            Self::HeaderTooShort { expected, actual } => {
                write!(
                    f,
                    "packet header too short: expected {expected} bytes, got {actual}"
                )
            }
            Self::PacketTooSmall { size, minimum } => {
                write!(
                    f,
                    "packet size {size} is smaller than the minimum header size {minimum}"
                )
            }
            Self::PacketBodyTruncated { expected, actual } => {
                write!(
                    f,
                    "packet body truncated: expected {expected} bytes, got {actual}"
                )
            }
            Self::PacketTooLarge { size } => write!(f, "packet too large: {size}"),
        }
    }
}

impl Error for PacketCodecError {}
