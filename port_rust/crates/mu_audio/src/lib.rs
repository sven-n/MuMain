pub mod assets;
pub mod settings;

pub use assets::{AudioAssets, AudioAssetsError};
pub use settings::Settings;

pub const CRATE_NAME: &str = env!("CARGO_PKG_NAME");
