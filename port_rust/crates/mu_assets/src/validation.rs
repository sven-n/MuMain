use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use thiserror::Error;

use crate::{AssetManifest, MANIFEST_FILE_NAME, SUPPORTED_SCHEMA_VERSION};

pub fn manifest_path(asset_root: &Utf8Path) -> Utf8PathBuf {
    asset_root.join(MANIFEST_FILE_NAME)
}

#[derive(Debug, Error)]
pub enum AssetManifestError {
    #[error("asset manifest missing: {path}")]
    MissingManifest { path: Utf8PathBuf },
    #[error("failed to read asset manifest {path}: {source}")]
    ReadManifest {
        path: Utf8PathBuf,
        source: io::Error,
    },
    #[error("failed to parse asset manifest {path}: {source}")]
    ParseManifest {
        path: Utf8PathBuf,
        source: serde_json::Error,
    },
    #[error("asset manifest schema mismatch at {path}: expected {expected}, found {found}")]
    SchemaMismatch {
        path: Utf8PathBuf,
        expected: u32,
        found: u32,
    },
    #[error("asset manifest content hash mismatch at {path}: expected {expected}, found {actual}")]
    ContentHashMismatch {
        path: Utf8PathBuf,
        expected: String,
        actual: String,
    },
}

pub fn load_manifest(manifest_path: &Utf8Path) -> Result<AssetManifest, AssetManifestError> {
    if !manifest_path.exists() {
        return Err(AssetManifestError::MissingManifest {
            path: manifest_path.to_path_buf(),
        });
    }

    let manifest_text =
        fs::read_to_string(manifest_path).map_err(|source| AssetManifestError::ReadManifest {
            path: manifest_path.to_path_buf(),
            source,
        })?;

    serde_json::from_str(&manifest_text).map_err(|source| AssetManifestError::ParseManifest {
        path: manifest_path.to_path_buf(),
        source,
    })
}

pub fn validate_manifest(
    manifest: &AssetManifest,
    manifest_path: &Utf8Path,
) -> Result<(), AssetManifestError> {
    if manifest.schema_version != SUPPORTED_SCHEMA_VERSION {
        return Err(AssetManifestError::SchemaMismatch {
            path: manifest_path.to_path_buf(),
            expected: SUPPORTED_SCHEMA_VERSION,
            found: manifest.schema_version,
        });
    }

    let expected = manifest.compute_content_hash();
    if manifest.content_hash != expected {
        return Err(AssetManifestError::ContentHashMismatch {
            path: manifest_path.to_path_buf(),
            expected,
            actual: manifest.content_hash.clone(),
        });
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use std::fs;

    use camino::Utf8PathBuf;

    use super::{load_manifest, manifest_path, validate_manifest, AssetManifestError};
    use crate::{sha256_hex, AssetManifest, AssetManifestEntry, SUPPORTED_SCHEMA_VERSION};

    fn temp_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_asset_manifest_{}_{}",
            std::process::id(),
            unique_suffix()
        ));
        let root = Utf8PathBuf::from_path_buf(root).unwrap();
        fs::create_dir_all(&root).unwrap();
        root
    }

    fn unique_suffix() -> u128 {
        std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_nanos()
    }

    fn manifest_fixture() -> AssetManifest {
        AssetManifest {
            schema_version: SUPPORTED_SCHEMA_VERSION,
            source_client_version: "1.0.0".to_string(),
            content_hash: String::new(),
            generated_at: "2026-05-18T00:00:00Z".to_string(),
            entries: vec![AssetManifestEntry {
                source_path: "legacy/data.bin".to_string(),
                converted_path: "converted/data.bin".to_string(),
                kind: "data".to_string(),
                source_hash: sha256_hex(b"legacy"),
                converted_hash: sha256_hex(b"converted"),
                dependencies: vec!["converted/shared.bin".to_string()],
            }],
        }
        .with_computed_content_hash()
    }

    #[test]
    fn load_manifest_reports_missing_file() {
        let root = temp_root();
        let manifest_path = manifest_path(&root);

        let error = load_manifest(&manifest_path).unwrap_err();

        assert!(matches!(
            error,
            AssetManifestError::MissingManifest { ref path } if path == &manifest_path
        ));
    }

    #[test]
    fn validate_manifest_rejects_schema_mismatch() {
        let root = temp_root();
        let manifest_path = manifest_path(&root);
        let mut manifest = manifest_fixture();
        manifest.schema_version = SUPPORTED_SCHEMA_VERSION + 1;

        let error = validate_manifest(&manifest, &manifest_path).unwrap_err();

        assert!(matches!(
            error,
            AssetManifestError::SchemaMismatch { expected, found, ref path }
                if expected == SUPPORTED_SCHEMA_VERSION
                    && found == SUPPORTED_SCHEMA_VERSION + 1
                    && path == &manifest_path
        ));
    }

    #[test]
    fn validate_manifest_rejects_content_hash_mismatch() {
        let root = temp_root();
        let manifest_path = manifest_path(&root);
        let mut manifest = manifest_fixture();
        manifest.content_hash = "bad".repeat(16);

        let error = validate_manifest(&manifest, &manifest_path).unwrap_err();

        assert!(matches!(
            error,
            AssetManifestError::ContentHashMismatch { ref path, .. }
                if path == &manifest_path
        ));
    }
}
