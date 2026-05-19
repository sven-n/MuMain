use camino::Utf8Path;

use crate::{AssetManifest, AssetManifestEntry};

const AUDIO_EXTENSIONS: &[&str] = &["mp3", "ogg", "wav"];
const MUSIC_ROOT: &[&str] = &["data", "music"];
const SOUND_ROOT: &[&str] = &["data", "sound"];

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum AudioAssetKind {
    Music,
    SoundEffect,
}

impl AudioAssetKind {
    pub fn label(self) -> &'static str {
        match self {
            Self::Music => "music",
            Self::SoundEffect => "sfx",
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct AudioAssetEntry {
    pub kind: AudioAssetKind,
    pub entry: AssetManifestEntry,
}

#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct AudioAssetCatalog {
    pub music: Vec<AudioAssetEntry>,
    pub sound_effects: Vec<AudioAssetEntry>,
}

impl AudioAssetCatalog {
    pub fn total_assets(&self) -> usize {
        self.music.len() + self.sound_effects.len()
    }

    pub fn is_empty(&self) -> bool {
        self.total_assets() == 0
    }
}

pub fn classify_audio_path(path: &Utf8Path) -> Option<AudioAssetKind> {
    if !is_supported_extension(path) {
        return None;
    }

    if has_root(path, MUSIC_ROOT) {
        return Some(AudioAssetKind::Music);
    }

    if has_root(path, SOUND_ROOT) {
        return Some(AudioAssetKind::SoundEffect);
    }

    None
}

pub fn collect_audio_assets(manifest: &AssetManifest) -> AudioAssetCatalog {
    let mut catalog = AudioAssetCatalog::default();

    for entry in &manifest.entries {
        let Some(kind) = classify_audio_path(Utf8Path::new(&entry.source_path)) else {
            continue;
        };

        let audio_entry = AudioAssetEntry {
            kind,
            entry: entry.clone(),
        };

        match kind {
            AudioAssetKind::Music => catalog.music.push(audio_entry),
            AudioAssetKind::SoundEffect => catalog.sound_effects.push(audio_entry),
        }
    }

    catalog
        .music
        .sort_by(|left, right| left.entry.source_path.cmp(&right.entry.source_path));
    catalog
        .sound_effects
        .sort_by(|left, right| left.entry.source_path.cmp(&right.entry.source_path));

    catalog
}

pub fn format_audio_catalog(catalog: &AudioAssetCatalog) -> String {
    let mut out = String::new();
    out.push_str(&format!("total_assets: {}\n", catalog.total_assets()));
    out.push_str("\n[music]\n");
    for entry in &catalog.music {
        out.push_str(&format!(
            "{}: {} -> {}\n",
            entry.kind.label(),
            entry.entry.source_path,
            entry.entry.converted_path
        ));
    }
    out.push_str("\n[sfx]\n");
    for entry in &catalog.sound_effects {
        out.push_str(&format!(
            "{}: {} -> {}\n",
            entry.kind.label(),
            entry.entry.source_path,
            entry.entry.converted_path
        ));
    }
    out
}

fn has_root(path: &Utf8Path, expected: &[&str]) -> bool {
    let mut components = path.components();

    for expected_component in expected {
        let Some(component) = components.next() else {
            return false;
        };

        if !component.as_str().eq_ignore_ascii_case(expected_component) {
            return false;
        }
    }

    true
}

fn is_supported_extension(path: &Utf8Path) -> bool {
    let extension = path
        .extension()
        .map(str::to_ascii_lowercase)
        .unwrap_or_else(|| "<none>".to_owned());

    AUDIO_EXTENSIONS.contains(&extension.as_str())
}

#[cfg(test)]
mod tests {
    use camino::Utf8PathBuf;

    use super::{classify_audio_path, collect_audio_assets, AudioAssetCatalog, AudioAssetKind};
    use crate::{sha256_hex, AssetManifest, AssetManifestEntry, SUPPORTED_SCHEMA_VERSION};

    fn manifest_entry(source_path: &str) -> AssetManifestEntry {
        AssetManifestEntry {
            source_path: source_path.to_string(),
            converted_path: source_path.to_string(),
            kind: "audio".to_string(),
            source_hash: sha256_hex(source_path.as_bytes()),
            converted_hash: sha256_hex(source_path.as_bytes()),
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
    fn classifies_music_and_sound_effect_paths() {
        assert_eq!(
            classify_audio_path(&Utf8PathBuf::from("Data/Music/Pub.mp3")),
            Some(AudioAssetKind::Music)
        );
        assert_eq!(
            classify_audio_path(&Utf8PathBuf::from("data/sound/w35/ww_idle1.wav")),
            Some(AudioAssetKind::SoundEffect)
        );
        assert_eq!(
            classify_audio_path(&Utf8PathBuf::from("Data/Music/readme.txt")),
            None
        );
        assert_eq!(
            classify_audio_path(&Utf8PathBuf::from("Interface/click.wav")),
            None
        );
    }

    #[test]
    fn collect_audio_assets_only_keeps_music_and_sfx_entries() {
        let manifest = manifest_fixture(vec![
            manifest_entry("Data/Music/Pub.mp3"),
            manifest_entry("Data/Sound/iButtonClick.wav"),
            manifest_entry("Interface/icon.bmp"),
        ]);

        let catalog = collect_audio_assets(&manifest);

        assert_eq!(catalog.total_assets(), 2);
        assert_eq!(catalog.music.len(), 1);
        assert_eq!(catalog.sound_effects.len(), 1);
        assert_eq!(catalog.music[0].kind, AudioAssetKind::Music);
        assert_eq!(catalog.music[0].entry.source_path, "Data/Music/Pub.mp3");
        assert_eq!(catalog.sound_effects[0].kind, AudioAssetKind::SoundEffect);
        assert_eq!(
            catalog.sound_effects[0].entry.source_path,
            "Data/Sound/iButtonClick.wav"
        );
    }

    #[test]
    fn audio_catalog_can_be_empty() {
        let manifest = manifest_fixture(vec![manifest_entry("Interface/icon.bmp")]);

        let catalog: AudioAssetCatalog = collect_audio_assets(&manifest);

        assert!(catalog.is_empty());
        assert!(catalog.music.is_empty());
        assert!(catalog.sound_effects.is_empty());
    }
}
