use camino::{Utf8Path, Utf8PathBuf};
use thiserror::Error;

use mu_assets::{
    collect_audio_assets, validate_asset_root, AssetManifest, AssetRuntimeError, AudioAssetCatalog,
    AudioAssetEntry,
};

#[derive(Debug, Clone)]
pub struct AudioAssets {
    asset_root: Utf8PathBuf,
    manifest: AssetManifest,
    catalog: AudioAssetCatalog,
}

impl AudioAssets {
    pub fn load(asset_root: impl AsRef<Utf8Path>) -> Result<Self, AudioAssetsError> {
        let asset_root = asset_root.as_ref().to_path_buf();
        let manifest = validate_asset_root(&asset_root)?;
        let catalog = collect_audio_assets(&manifest);

        Ok(Self {
            asset_root,
            manifest,
            catalog,
        })
    }

    pub fn asset_root(&self) -> &Utf8Path {
        &self.asset_root
    }

    pub fn manifest(&self) -> &AssetManifest {
        &self.manifest
    }

    pub fn catalog(&self) -> &AudioAssetCatalog {
        &self.catalog
    }

    pub fn music(&self) -> &[AudioAssetEntry] {
        &self.catalog.music
    }

    pub fn sound_effects(&self) -> &[AudioAssetEntry] {
        &self.catalog.sound_effects
    }
}

#[derive(Debug, Error)]
pub enum AudioAssetsError {
    #[error(transparent)]
    Runtime(#[from] AssetRuntimeError),
}

#[cfg(test)]
mod tests {
    use std::fs;

    use camino::Utf8PathBuf;

    use super::{AudioAssets, AudioAssetsError};
    use mu_assets::{
        sha256_hex, AssetManifest, AssetManifestEntry, AssetRuntimeError, AudioAssetKind,
        MANIFEST_FILE_NAME, SUPPORTED_SCHEMA_VERSION,
    };

    fn temp_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_audio_assets_{}_{}",
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

    fn write_audio_file(root: &Utf8PathBuf, relative_path: &str, contents: &[u8]) {
        let path = root.join(relative_path);
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent).unwrap();
        }
        fs::write(path, contents).unwrap();
    }

    fn manifest_entry(relative_path: &str, contents: &[u8]) -> AssetManifestEntry {
        AssetManifestEntry {
            source_path: relative_path.to_string(),
            converted_path: relative_path.to_string(),
            kind: "audio".to_string(),
            source_hash: sha256_hex(contents),
            converted_hash: sha256_hex(contents),
            dependencies: Vec::new(),
        }
    }

    fn manifest_fixture(entries: Vec<AssetManifestEntry>) -> AssetManifest {
        AssetManifest {
            schema_version: SUPPORTED_SCHEMA_VERSION,
            source_client_version: "1.0.0".to_string(),
            content_hash: String::new(),
            generated_at: "2026-05-18T00:00:00Z".to_string(),
            entries,
        }
        .with_computed_content_hash()
    }

    #[test]
    fn load_audio_assets_collects_music_and_sfx_entries() {
        let root = temp_root();
        let music_bytes = b"music";
        let sound_bytes = b"sound";
        write_audio_file(&root, "Data/Music/Pub.mp3", music_bytes);
        write_audio_file(&root, "Data/Sound/iButtonClick.wav", sound_bytes);

        let manifest = manifest_fixture(vec![
            manifest_entry("Data/Music/Pub.mp3", music_bytes),
            manifest_entry("Data/Sound/iButtonClick.wav", sound_bytes),
        ]);
        write_manifest(&root, &manifest);

        let audio_assets = AudioAssets::load(&root).unwrap();

        assert_eq!(audio_assets.asset_root(), root.as_path());
        assert_eq!(audio_assets.manifest().entries.len(), 2);
        assert_eq!(audio_assets.music().len(), 1);
        assert_eq!(audio_assets.sound_effects().len(), 1);
        assert_eq!(audio_assets.catalog().total_assets(), 2);
        assert_eq!(audio_assets.music()[0].kind, AudioAssetKind::Music);
        assert_eq!(
            audio_assets.sound_effects()[0].kind,
            AudioAssetKind::SoundEffect
        );
    }

    #[test]
    fn load_audio_assets_reports_missing_files_through_runtime_validation() {
        let root = temp_root();
        let music_bytes = b"music";
        let manifest = manifest_fixture(vec![manifest_entry("Data/Music/Pub.mp3", music_bytes)]);
        write_manifest(&root, &manifest);

        let error = AudioAssets::load(&root).unwrap_err();

        assert!(matches!(
            error,
            AudioAssetsError::Runtime(AssetRuntimeError::MissingConvertedAsset { .. })
        ));
    }
}
