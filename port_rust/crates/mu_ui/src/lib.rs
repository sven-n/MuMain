pub mod i18n;

pub const CRATE_NAME: &str = env!("CARGO_PKG_NAME");

mod fixture_routes;

pub use fixture_routes::{fixture_routes, UiFixtureRoute};
pub use i18n::Translator;
pub use mu_assets::TranslationDomain;
