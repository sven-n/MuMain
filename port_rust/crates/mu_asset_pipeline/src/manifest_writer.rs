use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use mu_assets::{manifest_path, AssetManifest, AssetManifestEntry, SUPPORTED_SCHEMA_VERSION};
use thiserror::Error;

pub fn build_manifest(
    source_client_version: impl Into<String>,
    generated_at: impl Into<String>,
    entries: Vec<AssetManifestEntry>,
) -> AssetManifest {
    AssetManifest {
        schema_version: SUPPORTED_SCHEMA_VERSION,
        source_client_version: source_client_version.into(),
        content_hash: String::new(),
        generated_at: generated_at.into(),
        entries,
    }
    .with_computed_content_hash()
}

pub fn write_manifest(
    manifest_path: &Utf8Path,
    manifest: &AssetManifest,
) -> Result<Utf8PathBuf, ManifestWriterError> {
    if let Some(parent) = manifest_path.parent() {
        fs::create_dir_all(parent).map_err(|source| ManifestWriterError::CreateParent {
            path: parent.to_path_buf(),
            source,
        })?;
    }

    let payload = manifest
        .to_json_string()
        .map_err(|source| ManifestWriterError::Serialize {
            path: manifest_path.to_path_buf(),
            source,
        })?;

    fs::write(manifest_path, payload).map_err(|source| ManifestWriterError::Write {
        path: manifest_path.to_path_buf(),
        source,
    })?;

    Ok(manifest_path.to_path_buf())
}

pub fn write_manifest_for_root(
    asset_root: &Utf8Path,
    manifest: &AssetManifest,
) -> Result<Utf8PathBuf, ManifestWriterError> {
    let manifest_path = manifest_path(asset_root);
    write_manifest(&manifest_path, manifest)
}

#[derive(Debug, Error)]
pub enum ManifestWriterError {
    #[error("failed to create manifest parent directory {path}: {source}")]
    CreateParent {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
    #[error("failed to serialize asset manifest {path}: {source}")]
    Serialize {
        path: Utf8PathBuf,
        #[source]
        source: serde_json::Error,
    },
    #[error("failed to write asset manifest {path}: {source}")]
    Write {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
}

#[cfg(test)]
mod tests {
    use std::fs;
    use std::io::Cursor;

    use camino::Utf8PathBuf;
    use image::{DynamicImage, ImageFormat, RgbaImage};

    use super::{build_manifest, write_manifest_for_root};
    use crate::textures::convert_textures;
    use mu_assets::{load_manifest, manifest_path, validate_manifest};

    fn temp_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_asset_manifest_writer_{}_{}",
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

    fn write_image(path: &Utf8PathBuf, format: ImageFormat, rgba: [u8; 4]) {
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent).unwrap();
        }

        let mut bytes = Cursor::new(Vec::new());
        DynamicImage::ImageRgba8(RgbaImage::from_raw(1, 1, rgba.to_vec()).unwrap())
            .write_to(&mut bytes, format)
            .unwrap();
        fs::write(path, bytes.into_inner()).unwrap();
    }

    #[test]
    fn writes_manifest_from_converted_textures() {
        let source_root = temp_root().join("source");
        let output_root = temp_root().join("output");
        write_image(
            &source_root.join("Interface/icon.bmp"),
            ImageFormat::Bmp,
            [9, 8, 7, 255],
        );

        let report = convert_textures(&source_root, &output_root).unwrap();
        let manifest = build_manifest("1.0.0", "2026-05-18T00:00:00Z", report.converted.clone());

        let written_path = write_manifest_for_root(&output_root, &manifest).unwrap();
        let loaded = load_manifest(&manifest_path(&output_root)).unwrap();

        assert_eq!(written_path, manifest_path(&output_root));
        assert_eq!(loaded.entries.len(), 1);
        validate_manifest(&loaded, &manifest_path(&output_root)).unwrap();
    }

    #[test]
    fn build_and_write_manifest_round_trips() {
        let asset_root = temp_root();
        let manifest = build_manifest("1.0.0", "2026-05-18T00:00:00Z", Vec::new());

        let written_path = write_manifest_for_root(&asset_root, &manifest).unwrap();
        let loaded = load_manifest(&manifest_path(&asset_root)).unwrap();

        assert_eq!(written_path, manifest_path(&asset_root));
        assert_eq!(loaded.content_hash, manifest.content_hash);
        validate_manifest(&loaded, &manifest_path(&asset_root)).unwrap();
    }
}
