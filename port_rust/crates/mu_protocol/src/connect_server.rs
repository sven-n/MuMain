use std::convert::TryFrom;

pub use crate::codec::{decode_packet, encode_packet};

pub const CONNECT_SERVER_HEADCODE: u8 = 0xF4;
pub const SERVER_LIST_REQUEST_SUBCODE: u8 = 0x06;
pub const SERVER_LIST_RESPONSE_SUBCODE: u8 = 0x06;
const SERVER_LIST_RESPONSE_CODE: u8 = 0xC2;

pub use crate::error::PacketCodecError as EncodeError;
pub use crate::frame::PacketFrame as PacketView;

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

pub fn is_server_list_request(bytes: &[u8]) -> bool {
    matches!(decode_packet(bytes), Ok(packet) if {
        packet.headcode == CONNECT_SERVER_HEADCODE && packet.subcode == SERVER_LIST_REQUEST_SUBCODE
    })
}

pub fn encode_server_list_response(entries: &[ServerEntry]) -> Result<Vec<u8>, EncodeError> {
    let payload = build_server_list_payload(entries)?;
    encode_packet(
        SERVER_LIST_RESPONSE_CODE,
        CONNECT_SERVER_HEADCODE,
        SERVER_LIST_RESPONSE_SUBCODE,
        &payload,
    )
}

fn build_server_list_payload(entries: &[ServerEntry]) -> Result<Vec<u8>, EncodeError> {
    let payload_bytes = entries
        .len()
        .checked_mul(3)
        .and_then(|value| value.checked_add(2))
        .ok_or(EncodeError::PacketTooLarge { size: usize::MAX })?;
    let packet_bytes = payload_bytes
        .checked_add(5)
        .ok_or(EncodeError::PacketTooLarge { size: usize::MAX })?;
    let count = u16::try_from(entries.len())
        .map_err(|_| EncodeError::PacketTooLarge { size: packet_bytes })?;

    let mut payload = Vec::with_capacity(payload_bytes);
    payload.extend_from_slice(&count.to_be_bytes());

    for entry in entries {
        payload.extend_from_slice(&entry.connect_index.to_le_bytes());
        payload.push(entry.percent);
    }

    Ok(payload)
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
