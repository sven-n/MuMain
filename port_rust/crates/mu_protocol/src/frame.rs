pub const SHORT_PACKET_HEADER_LEN: usize = 4;
pub const LONG_PACKET_HEADER_LEN: usize = 5;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PacketFrame<'a> {
    pub code: u8,
    pub size: usize,
    pub headcode: u8,
    pub subcode: u8,
    pub payload: &'a [u8],
}

impl<'a> PacketFrame<'a> {
    pub const fn new(code: u8, size: usize, headcode: u8, subcode: u8, payload: &'a [u8]) -> Self {
        Self {
            code,
            size,
            headcode,
            subcode,
            payload,
        }
    }

    pub const fn is_short(code: u8) -> bool {
        code % 2 == 1
    }

    pub const fn header_len(code: u8) -> usize {
        if Self::is_short(code) {
            SHORT_PACKET_HEADER_LEN
        } else {
            LONG_PACKET_HEADER_LEN
        }
    }

    pub fn payload_len(&self) -> usize {
        self.payload.len()
    }
}

#[cfg(test)]
mod tests {
    use super::{PacketFrame, LONG_PACKET_HEADER_LEN, SHORT_PACKET_HEADER_LEN};

    #[test]
    fn reports_header_lengths_by_code_parity() {
        assert_eq!(PacketFrame::header_len(0xC1), SHORT_PACKET_HEADER_LEN);
        assert_eq!(PacketFrame::header_len(0xC2), LONG_PACKET_HEADER_LEN);
        assert!(PacketFrame::is_short(0xC1));
        assert!(!PacketFrame::is_short(0xC2));
    }
}
