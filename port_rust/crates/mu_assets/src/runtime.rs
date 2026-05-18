use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use thiserror::Error;

use crate::{sha256_hex, validate_manifest, AssetManifest, AssetManifestError};

#[derive(Debug, Error)]
pub enum AssetRuntimeError {
    #[error("asset root missing: {path}")]
    MissingAssetRoot { path: Utf8PathBuf },
    #[error(transparent)]
    Manifest(#[from] AssetManifestError),
    #[error("converted asset path escapes asset root: {path}")]
    InvalidConvertedPath { path: String },
    #[error("converted asset missing: {path}")]
    MissingConvertedAsset { path: Utf8PathBuf },
    #[error("converted asset hash mismatch at {path}: expected {expected}, found {actual}")]
    ConvertedHashMismatch {
        path: Utf8PathBuf,
        expected: String,
        actual: String,
    },
    #[error("failed to read converted asset {path}: {source}")]
    ReadConvertedAsset {
        path: Utf8PathBuf,
        source: io::Error,
    },
}

pub fn validate_asset_root(asset_root: &Utf8Path) -> Result<AssetManifest, AssetRuntimeError> {
    if !asset_root.exists() || !asset_root.is_dir() {
        return Err(AssetRuntimeError::MissingAssetRoot {
            path: asset_root.to_path_buf(),
        });
    }

    let manifest_path = crate::manifest_path(asset_root);
    let manifest = crate::load_manifest(&manifest_path)?;
    validate_manifest(&manifest, &manifest_path)?;
    validate_converted_assets(asset_root, &manifest)?;

    Ok(manifest)
}

fn validate_converted_assets(
    asset_root: &Utf8Path,
    manifest: &AssetManifest,
) -> Result<(), AssetRuntimeError> {
    for entry in &manifest.entries {
        let converted_path = converted_asset_path(asset_root, &entry.converted_path)?;
        if !converted_path.exists() {
            return Err(AssetRuntimeError::MissingConvertedAsset {
                path: converted_path,
            });
        }

        let actual_hash = hash_file(&converted_path)?;
        if actual_hash != entry.converted_hash {
            return Err(AssetRuntimeError::ConvertedHashMismatch {
                path: converted_path,
                expected: entry.converted_hash.clone(),
                actual: actual_hash,
            });
        }
    }

    Ok(())
}

fn converted_asset_path(
    asset_root: &Utf8Path,
    entry_path: &str,
) -> Result<Utf8PathBuf, AssetRuntimeError> {
    let path = Utf8Path::new(entry_path);
    if path.is_absolute()
        || path
            .components()
            .any(|component| component.as_str() == "..")
    {
        return Err(AssetRuntimeError::InvalidConvertedPath {
            path: entry_path.to_string(),
        });
    }

    Ok(asset_root.join(path))
}

fn hash_file(path: &Utf8Path) -> Result<String, AssetRuntimeError> {
    let bytes = fs::read(path).map_err(|source| AssetRuntimeError::ReadConvertedAsset {
        path: path.to_path_buf(),
        source,
    })?;

    Ok(sha256_hex(&bytes))
}

#[cfg(test)]
mod tests {
    use std::fs;

    use camino::Utf8PathBuf;

    use super::{validate_asset_root, AssetRuntimeError};
    use crate::{
        sha256_hex, AssetManifest, AssetManifestEntry, MANIFEST_FILE_NAME, SUPPORTED_SCHEMA_VERSION,
    };

    fn temp_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_asset_runtime_{}_{}",
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

    fn write_manifest(root: &Utf8PathBuf, manifest: &AssetManifest) {
        fs::write(
            root.join(MANIFEST_FILE_NAME),
            manifest.to_json_string().unwrap(),
        )
        .unwrap();
    }

    fn manifest_fixture(root: &Utf8PathBuf, converted_bytes: &[u8]) -> AssetManifest {
        let converted_path = root.join("converted/data.bin");
        fs::create_dir_all(converted_path.parent().unwrap()).unwrap();
        fs::write(&converted_path, converted_bytes).unwrap();

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
                converted_hash: sha256_hex(converted_bytes),
                dependencies: vec![],
            }],
        }
        .with_computed_content_hash()
    }

    #[test]
    fn valid_asset_root_passes_validation() {
        let root = temp_root();
        let manifest = manifest_fixture(&root, b"converted");
        write_manifest(&root, &manifest);

        let loaded = validate_asset_root(&root).unwrap();

        assert_eq!(loaded.entries.len(), 1);
        assert_eq!(loaded.content_hash, manifest.content_hash);
    }

    #[test]
    fn missing_asset_root_is_rejected() {
        let root = Utf8PathBuf::from("/tmp/mu_asset_runtime_missing_root");

        let error = validate_asset_root(&root).unwrap_err();

        assert!(matches!(
            error,
            AssetRuntimeError::MissingAssetRoot { ref path } if path == &root
        ));
    }

    #[test]
    fn converted_hash_mismatch_is_rejected() {
        let root = temp_root();
        let manifest = manifest_fixture(&root, b"converted");
        write_manifest(&root, &manifest);
        fs::write(root.join("converted/data.bin"), b"changed").unwrap();

        let error = validate_asset_root(&root).unwrap_err();

        assert!(matches!(
            error,
            AssetRuntimeError::ConvertedHashMismatch { .. }
        ));
    }
}
