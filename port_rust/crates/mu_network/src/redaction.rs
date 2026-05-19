use std::fmt;

pub const REDACTED: &str = "[redacted]";

#[derive(Clone, Copy, Default, PartialEq, Eq)]
pub struct Redacted;

impl fmt::Debug for Redacted {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(REDACTED)
    }
}

impl fmt::Display for Redacted {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(REDACTED)
    }
}

pub fn redact(_: &str) -> Redacted {
    Redacted
}

#[cfg(test)]
mod tests {
    use super::{redact, REDACTED};

    #[test]
    fn redact_masks_display_and_debug() {
        let secret = redact("super-secret");

        assert_eq!(secret.to_string(), REDACTED);
        assert_eq!(format!("{secret:?}"), REDACTED);
    }
}
