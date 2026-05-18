use std::error::Error;
use std::fmt;

pub const CONNECT_SERVER_HEADCODE: u8 = 0xF4;
pub const SERVER_LIST_REQUEST_SUBCODE: u8 = 0x06;
pub const SERVER_LIST_RESPONSE_SUBCODE: u8 = 0x06;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ServerEntry {
    pub connect_index: u16,
    pub percent: u8,
}

impl ServerEntry {
    pub const fn new(connect_index: u16, percent: u8) -> Self {
        Self {
            connect_index,
            percent,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PacketView<'a> {
    pub code: u8,
    pub size: usize,
    pub headcode: u8,
    pub subcode: u8,
    pub payload: &'a [u8],
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EncodeError {
    PacketTooLarge { size: usize },
}

impl fmt::Display for EncodeError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::PacketTooLarge { size } => {
                write!(f, "connect-server packet too large: {size}")
            }
        }
    }
}

impl Error for EncodeError {}

pub fn decode_packet(bytes: &[u8]) -> Option<PacketView<'_>> {
    let code = *bytes.first()?;
    if code % 2 == 1 {
        let size = usize::from(*bytes.get(1)?);
        if size < 4 || bytes.len() < size {
            return None;
        }

        Some(PacketView {
            code,
            size,
            headcode: *bytes.get(2)?,
            subcode: *bytes.get(3)?,
            payload: &bytes[4..size],
        })
    } else {
        let size = usize::from(u16::from_be_bytes([*bytes.get(1)?, *bytes.get(2)?]));
        if size < 5 || bytes.len() < size {
            return None;
        }

        Some(PacketView {
            code,
            size,
            headcode: *bytes.get(3)?,
            subcode: *bytes.get(4)?,
            payload: &bytes[5..size],
        })
    }
}

pub fn is_server_list_request(bytes: &[u8]) -> bool {
    decode_packet(bytes).is_some_and(|packet| {
        packet.headcode == CONNECT_SERVER_HEADCODE && packet.subcode == SERVER_LIST_REQUEST_SUBCODE
    })
}

pub fn encode_server_list_response(entries: &[ServerEntry]) -> Result<Vec<u8>, EncodeError> {
    let payload_bytes = entries
        .len()
        .checked_mul(3)
        .and_then(|value| 7usize.checked_add(value))
        .ok_or(EncodeError::PacketTooLarge { size: usize::MAX })?;
    let size = u16::try_from(payload_bytes).map_err(|_| EncodeError::PacketTooLarge {
        size: payload_bytes,
    })?;
    let count = u16::try_from(entries.len()).map_err(|_| EncodeError::PacketTooLarge {
        size: entries.len(),
    })?;

    let mut packet = Vec::with_capacity(usize::from(size));
    packet.push(0xC2);
    packet.extend_from_slice(&size.to_be_bytes());
    packet.push(CONNECT_SERVER_HEADCODE);
    packet.push(SERVER_LIST_RESPONSE_SUBCODE);
    packet.extend_from_slice(&count.to_be_bytes());

    for entry in entries {
        packet.extend_from_slice(&entry.connect_index.to_le_bytes());
        packet.push(entry.percent);
    }

    Ok(packet)
}

#[cfg(test)]
mod tests {
    use super::{decode_packet, encode_server_list_response, is_server_list_request, ServerEntry};

    #[test]
    fn decodes_server_list_request() {
        let packet = decode_packet(&[0xC1, 0x04, 0xF4, 0x06]).unwrap();

        assert_eq!(packet.code, 0xC1);
        assert_eq!(packet.size, 4);
        assert_eq!(packet.headcode, 0xF4);
        assert_eq!(packet.subcode, 0x06);
        assert!(packet.payload.is_empty());
        assert!(is_server_list_request(&[0xC1, 0x04, 0xF4, 0x06]));
    }

    #[test]
    fn encodes_server_list_response() {
        let packet = encode_server_list_response(&[ServerEntry::new(7, 42)]).unwrap();

        assert_eq!(
            packet,
            vec![0xC2, 0x00, 0x0A, 0xF4, 0x06, 0x00, 0x01, 0x07, 0x00, 0x2A]
        );
    }
}
