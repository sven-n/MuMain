#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ErrorCatalogEntry {
    pub component: &'static str,
    pub error_id: &'static str,
    pub user_message: &'static str,
    pub diagnostic: &'static str,
}

impl ErrorCatalogEntry {
    pub const fn new(
        component: &'static str,
        error_id: &'static str,
        user_message: &'static str,
        diagnostic: &'static str,
    ) -> Self {
        Self {
            component,
            error_id,
            user_message,
            diagnostic,
        }
    }

    pub fn matches_error_id(&self, error_id: &str) -> bool {
        self.error_id == error_id
    }
}

pub const ASSET_VALIDATION_FAILED: ErrorCatalogEntry = ErrorCatalogEntry::new(
    "boot",
    "asset-validation-failed",
    "The client assets could not be verified.",
    "asset validation failed",
);
pub const CONTROL_HTTP_BIND_FAILED: ErrorCatalogEntry = ErrorCatalogEntry::new(
    "control-http",
    "control-http-bind-failed",
    "The local control server could not start.",
    "control-http bind failed",
);
pub const CLIENT_STATE_WRITE_FAILED: ErrorCatalogEntry = ErrorCatalogEntry::new(
    "runtime",
    "client-state-write-failed",
    "The client state could not be written.",
    "failed to write client state",
);
pub const CONTROL_HTTP_ADDRESS_WRITE_FAILED: ErrorCatalogEntry = ErrorCatalogEntry::new(
    "control-http",
    "control-http-address-write-failed",
    "The control server address could not be reported.",
    "failed to write control-http address",
);
pub const CONTROL_HTTP_SERVER_FAILED: ErrorCatalogEntry = ErrorCatalogEntry::new(
    "control-http",
    "control-http-server-failed",
    "The local control server stopped unexpectedly.",
    "control-http server failed",
);

pub const ERROR_CATALOG: [ErrorCatalogEntry; 5] = [
    ASSET_VALIDATION_FAILED,
    CONTROL_HTTP_BIND_FAILED,
    CLIENT_STATE_WRITE_FAILED,
    CONTROL_HTTP_ADDRESS_WRITE_FAILED,
    CONTROL_HTTP_SERVER_FAILED,
];

pub fn error_catalog() -> &'static [ErrorCatalogEntry] {
    &ERROR_CATALOG
}

pub fn lookup_error(error_id: &str) -> Option<&'static ErrorCatalogEntry> {
    ERROR_CATALOG
        .iter()
        .find(|entry| entry.matches_error_id(error_id))
}

pub fn user_message(error_id: &str) -> Option<&'static str> {
    lookup_error(error_id).map(|entry| entry.user_message)
}

pub fn diagnostic_message(error_id: &str) -> Option<&'static str> {
    lookup_error(error_id).map(|entry| entry.diagnostic)
}

#[cfg(test)]
mod tests {
    use super::{
        diagnostic_message, error_catalog, lookup_error, user_message, ASSET_VALIDATION_FAILED,
        CLIENT_STATE_WRITE_FAILED, CONTROL_HTTP_ADDRESS_WRITE_FAILED, CONTROL_HTTP_BIND_FAILED,
        CONTROL_HTTP_SERVER_FAILED,
    };

    #[test]
    fn error_catalog_contains_the_expected_entries() {
        let catalog = error_catalog();

        assert_eq!(catalog.len(), 5);
        assert_eq!(catalog[0], ASSET_VALIDATION_FAILED);
        assert_eq!(catalog[1], CONTROL_HTTP_BIND_FAILED);
        assert_eq!(catalog[2], CLIENT_STATE_WRITE_FAILED);
        assert_eq!(catalog[3], CONTROL_HTTP_ADDRESS_WRITE_FAILED);
        assert_eq!(catalog[4], CONTROL_HTTP_SERVER_FAILED);
    }

    #[test]
    fn lookup_error_returns_safe_message_and_diagnostic() {
        let entry = lookup_error("control-http-bind-failed").expect("catalog entry missing");

        assert_eq!(entry.component, "control-http");
        assert_eq!(entry.error_id, "control-http-bind-failed");
        assert_eq!(
            entry.user_message,
            "The local control server could not start."
        );
        assert_eq!(entry.diagnostic, "control-http bind failed");
        assert_eq!(
            user_message("control-http-bind-failed"),
            Some("The local control server could not start.")
        );
        assert_eq!(
            diagnostic_message("control-http-bind-failed"),
            Some("control-http bind failed")
        );
    }

    #[test]
    fn unknown_error_ids_are_rejected() {
        assert!(lookup_error("missing").is_none());
        assert_eq!(user_message("missing"), None);
        assert_eq!(diagnostic_message("missing"), None);
    }

    #[test]
    fn user_messages_stay_separate_from_diagnostics() {
        for entry in error_catalog() {
            assert!(!entry.user_message.is_empty());
            assert!(!entry.diagnostic.is_empty());
            assert_ne!(entry.user_message, entry.diagnostic);
        }
    }
}
