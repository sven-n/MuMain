use mu_core::error::{diagnostic_message as core_diagnostic_message, lookup_error, user_message};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ErrorPresentation {
    pub component: &'static str,
    pub error_id: &'static str,
    pub user_message: &'static str,
    pub diagnostic: &'static str,
}

impl From<mu_core::error::ErrorCatalogEntry> for ErrorPresentation {
    fn from(entry: mu_core::error::ErrorCatalogEntry) -> Self {
        Self {
            component: entry.component,
            error_id: entry.error_id,
            user_message: entry.user_message,
            diagnostic: entry.diagnostic,
        }
    }
}

pub fn error_presentation(error_id: &str) -> Option<ErrorPresentation> {
    lookup_error(error_id).copied().map(ErrorPresentation::from)
}

pub fn user_message_for(error_id: &str) -> Option<&'static str> {
    user_message(error_id)
}

pub fn diagnostic_message_for(error_id: &str) -> Option<&'static str> {
    core_diagnostic_message(error_id)
}

#[cfg(test)]
mod tests {
    use super::{diagnostic_message_for, error_presentation, user_message_for, ErrorPresentation};

    #[test]
    fn presentation_exposes_safe_text_and_technical_diagnostic() {
        let presentation =
            error_presentation("asset-validation-failed").expect("catalog entry missing");

        assert_eq!(
            presentation,
            ErrorPresentation {
                component: "boot",
                error_id: "asset-validation-failed",
                user_message: "The client assets could not be verified.",
                diagnostic: "asset validation failed",
            }
        );
        assert_eq!(
            user_message_for("asset-validation-failed"),
            Some("The client assets could not be verified.")
        );
        assert_eq!(
            diagnostic_message_for("asset-validation-failed"),
            Some("asset validation failed")
        );
    }

    #[test]
    fn unknown_error_ids_have_no_presentation() {
        assert!(error_presentation("missing").is_none());
        assert_eq!(user_message_for("missing"), None);
        assert_eq!(diagnostic_message_for("missing"), None);
    }

    #[test]
    fn safe_text_differs_from_diagnostic_text() {
        let presentation =
            error_presentation("control-http-server-failed").expect("catalog entry missing");

        assert_ne!(presentation.user_message, presentation.diagnostic);
    }
}
