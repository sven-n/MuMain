use std::convert::TryFrom;

use crate::error::PacketCodecError;
use crate::frame::{PacketFrame, LONG_PACKET_HEADER_LEN, SHORT_PACKET_HEADER_LEN};

pub fn decode_packet(bytes: &[u8]) -> Result<PacketFrame<'_>, PacketCodecError> {
    let code = *bytes.first().ok_or(PacketCodecError::EmptyPacket)?;

    if PacketFrame::is_short(code) {
        decode_short_packet(bytes)
    } else {
        decode_long_packet(bytes)
    }
}

pub fn encode_packet(
    code: u8,
    headcode: u8,
    subcode: u8,
    payload: &[u8],
) -> Result<Vec<u8>, PacketCodecError> {
    if PacketFrame::is_short(code) {
        encode_short_packet(code, headcode, subcode, payload)
    } else {
        encode_long_packet(code, headcode, subcode, payload)
    }
}

fn decode_short_packet(bytes: &[u8]) -> Result<PacketFrame<'_>, PacketCodecError> {
    if bytes.len() < SHORT_PACKET_HEADER_LEN {
        return Err(PacketCodecError::HeaderTooShort {
            expected: SHORT_PACKET_HEADER_LEN,
            actual: bytes.len(),
        });
    }

    let size = usize::from(bytes[1]);
    if size < SHORT_PACKET_HEADER_LEN {
        return Err(PacketCodecError::PacketTooSmall {
            size,
            minimum: SHORT_PACKET_HEADER_LEN,
        });
    }

    if bytes.len() < size {
        return Err(PacketCodecError::PacketBodyTruncated {
            expected: size,
            actual: bytes.len(),
        });
    }

    Ok(PacketFrame::new(
        bytes[0],
        size,
        bytes[2],
        bytes[3],
        &bytes[4..size],
    ))
}

fn decode_long_packet(bytes: &[u8]) -> Result<PacketFrame<'_>, PacketCodecError> {
    if bytes.len() < LONG_PACKET_HEADER_LEN {
        return Err(PacketCodecError::HeaderTooShort {
            expected: LONG_PACKET_HEADER_LEN,
            actual: bytes.len(),
        });
    }

    let size = usize::from(u16::from_be_bytes([bytes[1], bytes[2]]));
    if size < LONG_PACKET_HEADER_LEN {
        return Err(PacketCodecError::PacketTooSmall {
            size,
            minimum: LONG_PACKET_HEADER_LEN,
        });
    }

    if bytes.len() < size {
        return Err(PacketCodecError::PacketBodyTruncated {
            expected: size,
            actual: bytes.len(),
        });
    }

    Ok(PacketFrame::new(
        bytes[0],
        size,
        bytes[3],
        bytes[4],
        &bytes[5..size],
    ))
}

fn encode_short_packet(
    code: u8,
    headcode: u8,
    subcode: u8,
    payload: &[u8],
) -> Result<Vec<u8>, PacketCodecError> {
    let size = SHORT_PACKET_HEADER_LEN
        .checked_add(payload.len())
        .ok_or(PacketCodecError::PacketTooLarge { size: usize::MAX })?;
    let size = u8::try_from(size).map_err(|_| PacketCodecError::PacketTooLarge { size })?;

    let mut packet = Vec::with_capacity(usize::from(size));
    packet.push(code);
    packet.push(size);
    packet.push(headcode);
    packet.push(subcode);
    packet.extend_from_slice(payload);
    Ok(packet)
}

fn encode_long_packet(
    code: u8,
    headcode: u8,
    subcode: u8,
    payload: &[u8],
) -> Result<Vec<u8>, PacketCodecError> {
    let size = LONG_PACKET_HEADER_LEN
        .checked_add(payload.len())
        .ok_or(PacketCodecError::PacketTooLarge { size: usize::MAX })?;
    let size = u16::try_from(size).map_err(|_| PacketCodecError::PacketTooLarge { size })?;

    let mut packet = Vec::with_capacity(usize::from(size));
    packet.push(code);
    packet.extend_from_slice(&size.to_be_bytes());
    packet.push(headcode);
    packet.push(subcode);
    packet.extend_from_slice(payload);
    Ok(packet)
}

#[cfg(test)]
mod tests {
    use super::{decode_packet, encode_packet};
    use crate::error::PacketCodecError;

    #[test]
    fn decodes_short_packet() {
        let packet = decode_packet(&[0xC1, 0x04, 0xF4, 0x06]).unwrap();

        assert_eq!(packet.code, 0xC1);
        assert_eq!(packet.size, 4);
        assert_eq!(packet.headcode, 0xF4);
        assert_eq!(packet.subcode, 0x06);
        assert!(packet.payload.is_empty());
    }

    #[test]
    fn decodes_long_packet() {
        let packet =
            decode_packet(&[0xC2, 0x00, 0x0A, 0xF4, 0x06, 0x00, 0x01, 0x07, 0x00, 0x2A]).unwrap();

        assert_eq!(packet.code, 0xC2);
        assert_eq!(packet.size, 10);
        assert_eq!(packet.headcode, 0xF4);
        assert_eq!(packet.subcode, 0x06);
        assert_eq!(packet.payload, &[0x00, 0x01, 0x07, 0x00, 0x2A]);
    }

    #[test]
    fn rejects_truncated_and_invalid_packets() {
        assert!(matches!(
            decode_packet(&[]),
            Err(PacketCodecError::EmptyPacket)
        ));
        assert!(matches!(
            decode_packet(&[0xC1, 0x04, 0xF4]),
            Err(PacketCodecError::HeaderTooShort {
                expected: 4,
                actual: 3
            })
        ));
        assert!(matches!(
            decode_packet(&[0xC1, 0x03, 0xF4, 0x06]),
            Err(PacketCodecError::PacketTooSmall {
                size: 3,
                minimum: 4
            })
        ));
        assert!(matches!(
            decode_packet(&[0xC2, 0x00, 0x04, 0xF4, 0x06]),
            Err(PacketCodecError::PacketTooSmall {
                size: 4,
                minimum: 5
            })
        ));
        assert!(matches!(
            decode_packet(&[0xC2, 0x00, 0x0A, 0xF4, 0x06, 0x00, 0x01, 0x07]),
            Err(PacketCodecError::PacketBodyTruncated {
                expected: 10,
                actual: 8
            })
        ));
    }

    #[test]
    fn encodes_short_and_long_packets() {
        assert_eq!(
            encode_packet(0xC1, 0xF4, 0x06, &[]).unwrap(),
            vec![0xC1, 0x04, 0xF4, 0x06]
        );
        assert_eq!(
            encode_packet(0xC2, 0xF4, 0x06, &[0x00, 0x01]).unwrap(),
            vec![0xC2, 0x00, 0x07, 0xF4, 0x06, 0x00, 0x01]
        );
    }

    #[test]
    fn rejects_packets_that_do_not_fit_the_short_header() {
        let payload = vec![0u8; 252];

        assert!(matches!(
            encode_packet(0xC1, 0xF4, 0x06, &payload),
            Err(PacketCodecError::PacketTooLarge { size: 256 })
        ));
    }
}
