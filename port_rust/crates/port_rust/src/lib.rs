pub const STATUS_MESSAGE: &str =
    "port_rust initialized: incremental Rust port scaffold, not playable";

pub fn status_message() -> &'static str {
    STATUS_MESSAGE
}

#[cfg(test)]
mod tests {
    use super::status_message;

    #[test]
    fn status_message_declares_initial_port_state() {
        let message = status_message();

        assert!(message.contains("initialized"));
        assert!(message.contains("incremental"));
        assert!(message.contains("not playable"));
    }
}
