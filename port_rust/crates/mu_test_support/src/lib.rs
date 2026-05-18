//! Shared helpers for legacy-vs-Rust validation tests.

pub mod evidence;
pub mod fixtures;
pub mod legacy;

pub use evidence::{EvidenceArtifact, EvidenceKind, EvidencePaths};
pub use fixtures::{slugify, FixturePaths};
pub use legacy::LegacyComparisonPaths;

pub const CRATE_NAME: &str = env!("CARGO_PKG_NAME");
