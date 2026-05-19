//! Shared helpers for legacy-vs-Rust validation tests.

pub mod evidence;
pub mod fixtures;
pub mod legacy;
pub mod source_inventory;

pub use evidence::{EvidenceArtifact, EvidenceKind, EvidencePaths};
pub use fixtures::{slugify, FixturePaths};
pub use legacy::LegacyComparisonPaths;
pub use source_inventory::{
    source_inventory_report, source_inventory_roots, SourceClassification, SourceEntry, SourceRoot,
};

pub const CRATE_NAME: &str = env!("CARGO_PKG_NAME");
