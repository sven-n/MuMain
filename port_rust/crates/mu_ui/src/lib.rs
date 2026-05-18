pub const CRATE_NAME: &str = env!("CARGO_PKG_NAME");

mod fixture_routes;

pub use fixture_routes::{fixture_routes, UiFixtureRoute};
