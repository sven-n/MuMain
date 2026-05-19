use std::collections::{BTreeMap, BTreeSet};
use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use serde_json::Value;
use thiserror::Error;

pub type TranslationMap = BTreeMap<String, String>;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum TranslationDomain {
    Game,
    Editor,
    Metadata,
}

impl TranslationDomain {
    pub const fn file_name(self) -> &'static str {
        match self {
            Self::Game => "game.json",
            Self::Editor => "editor.json",
            Self::Metadata => "metadata.json",
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TranslationBundle {
    pub locale: String,
    pub language_name: String,
    pub game: TranslationMap,
    pub editor: TranslationMap,
    pub metadata: TranslationMap,
}

impl TranslationBundle {
    pub fn lookup(&self, domain: TranslationDomain, key: &str) -> Option<&str> {
        self.map(domain).get(key).map(String::as_str)
    }

    pub fn has_translation(&self, domain: TranslationDomain, key: &str) -> bool {
        self.map(domain).contains_key(key)
    }

    fn map(&self, domain: TranslationDomain) -> &TranslationMap {
        match domain {
            TranslationDomain::Game => &self.game,
            TranslationDomain::Editor => &self.editor,
            TranslationDomain::Metadata => &self.metadata,
        }
    }
}

#[derive(Debug, Error)]
pub enum TranslationError {
    #[error("translation root missing: {path}")]
    MissingTranslationRoot { path: Utf8PathBuf },
    #[error("translation file missing: {path}")]
    MissingTranslationFile { path: Utf8PathBuf },
    #[error("failed to read translation file {path}: {source}")]
    ReadTranslationFile {
        path: Utf8PathBuf,
        source: io::Error,
    },
    #[error("failed to parse translation file {path}: {source}")]
    ParseTranslationFile {
        path: Utf8PathBuf,
        source: serde_json::Error,
    },
    #[error("translation file has no string entries: {path}")]
    EmptyTranslationFile { path: Utf8PathBuf },
    #[error("failed to read translation directory {path}: {source}")]
    ReadTranslationDirectory {
        path: Utf8PathBuf,
        source: io::Error,
    },
}

pub fn available_locales(translations_root: &Utf8Path) -> Result<Vec<String>, TranslationError> {
    if !translations_root.exists() || !translations_root.is_dir() {
        return Ok(Vec::new());
    }

    let mut locales = BTreeSet::new();
    let entries = fs::read_dir(translations_root).map_err(|source| {
        TranslationError::ReadTranslationDirectory {
            path: translations_root.to_path_buf(),
            source,
        }
    })?;

    for entry in entries {
        let entry = entry.map_err(|source| TranslationError::ReadTranslationDirectory {
            path: translations_root.to_path_buf(),
            source,
        })?;

        let locale = entry.file_name().to_string_lossy().into_owned();
        let locale_dir = translations_root.join(&locale);
        if locale_dir.is_dir()
            && locale_dir
                .join(TranslationDomain::Game.file_name())
                .is_file()
        {
            locales.insert(locale);
        }
    }

    Ok(locales.into_iter().collect())
}

pub fn load_translation_bundle(
    translations_root: &Utf8Path,
    locale: &str,
) -> Result<TranslationBundle, TranslationError> {
    if !translations_root.exists() || !translations_root.is_dir() {
        return Err(TranslationError::MissingTranslationRoot {
            path: translations_root.to_path_buf(),
        });
    }

    let locale_root = translations_root.join(locale);
    let game = load_translation_map(&locale_root.join(TranslationDomain::Game.file_name()))?;
    let editor =
        load_optional_translation_map(&locale_root.join(TranslationDomain::Editor.file_name()));
    let metadata =
        load_optional_translation_map(&locale_root.join(TranslationDomain::Metadata.file_name()));
    let language_name = editor
        .get("language_name")
        .cloned()
        .unwrap_or_else(|| fallback_language_name(locale));

    Ok(TranslationBundle {
        locale: locale.to_owned(),
        language_name,
        game,
        editor,
        metadata,
    })
}

pub fn load_translation_map(path: &Utf8Path) -> Result<TranslationMap, TranslationError> {
    if !path.is_file() {
        return Err(TranslationError::MissingTranslationFile {
            path: path.to_path_buf(),
        });
    }

    let bytes = fs::read(path).map_err(|source| TranslationError::ReadTranslationFile {
        path: path.to_path_buf(),
        source,
    })?;
    let text = decode_translation_text(&bytes);
    let raw_map: BTreeMap<String, Value> =
        serde_json::from_str(&text).map_err(|source| TranslationError::ParseTranslationFile {
            path: path.to_path_buf(),
            source,
        })?;

    let mut map = TranslationMap::new();
    for (key, value) in raw_map {
        if let Value::String(value) = value {
            map.insert(key, value);
        }
    }

    if map.is_empty() {
        return Err(TranslationError::EmptyTranslationFile {
            path: path.to_path_buf(),
        });
    }

    Ok(map)
}

fn load_optional_translation_map(path: &Utf8Path) -> TranslationMap {
    load_translation_map(path).unwrap_or_default()
}

fn fallback_language_name(locale: &str) -> String {
    let mut chars = locale.chars();
    match chars.next() {
        Some(first) => {
            let mut display_name = first.to_uppercase().collect::<String>();
            display_name.push_str(chars.as_str());
            display_name
        }
        None => String::new(),
    }
}

fn decode_translation_text(bytes: &[u8]) -> String {
    if bytes.starts_with(&[0xEF, 0xBB, 0xBF]) {
        return String::from_utf8_lossy(&bytes[3..]).into_owned();
    }

    if bytes.starts_with(&[0xFF, 0xFE]) {
        return decode_utf16_le(&bytes[2..]);
    }

    if bytes.starts_with(&[0xFE, 0xFF]) {
        return decode_utf16_be(&bytes[2..]);
    }

    String::from_utf8_lossy(bytes).into_owned()
}

fn decode_utf16_le(bytes: &[u8]) -> String {
    let units = bytes
        .chunks_exact(2)
        .map(|chunk| u16::from_le_bytes([chunk[0], chunk[1]]))
        .collect::<Vec<_>>();
    String::from_utf16_lossy(&units)
}

fn decode_utf16_be(bytes: &[u8]) -> String {
    let units = bytes
        .chunks_exact(2)
        .map(|chunk| u16::from_be_bytes([chunk[0], chunk[1]]))
        .collect::<Vec<_>>();
    String::from_utf16_lossy(&units)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;

    fn temp_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_translations_{}_{}",
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

    fn write_file(path: &Utf8Path, contents: &str) {
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent).unwrap();
        }
        fs::write(path, contents).unwrap();
    }

    #[test]
    fn available_locales_only_reports_game_directories() {
        let root = temp_root();
        write_file(
            &root.join("en/game.json"),
            r#"{"_comment":"English","hello":"Hello"}"#,
        );
        write_file(
            &root.join("en/editor.json"),
            r#"{"language_name":"English"}"#,
        );
        write_file(
            &root.join("pt/editor.json"),
            r#"{"language_name":"Portuguese"}"#,
        );
        write_file(
            &root.join("ru/game.json"),
            r#"{"_comment":"Russian","hello":"Привет"}"#,
        );

        let locales = available_locales(&root).unwrap();

        assert_eq!(locales, vec!["en".to_string(), "ru".to_string()]);
    }

    #[test]
    fn load_translation_bundle_reads_maps_and_display_name() {
        let root = temp_root();
        write_file(
            &root.join("en/game.json"),
            r#"{"_comment":"English","hello":"Hello","file_missing":"Missing"}"#,
        );
        write_file(
            &root.join("en/editor.json"),
            r#"{"language_name":"English","btn_ok":"OK"}"#,
        );
        write_file(&root.join("en/metadata.json"), r#"{"field_Name":"Name"}"#);

        let bundle = load_translation_bundle(&root, "en").unwrap();

        assert_eq!(bundle.locale, "en");
        assert_eq!(bundle.language_name, "English");
        assert_eq!(
            bundle.lookup(TranslationDomain::Game, "hello"),
            Some("Hello")
        );
        assert_eq!(
            bundle.lookup(TranslationDomain::Editor, "btn_ok"),
            Some("OK")
        );
        assert_eq!(
            bundle.lookup(TranslationDomain::Metadata, "field_Name"),
            Some("Name")
        );
        assert!(bundle.has_translation(TranslationDomain::Game, "_comment"));
    }

    #[test]
    fn load_translation_bundle_falls_back_to_locale_name_when_editor_is_missing() {
        let root = temp_root();
        write_file(&root.join("pt/game.json"), r#"{"hello":"Olá"}"#);

        let bundle = load_translation_bundle(&root, "pt").unwrap();

        assert_eq!(bundle.language_name, "Pt");
    }

    #[test]
    fn load_translation_map_rejects_empty_objects() {
        let root = temp_root();
        let path = root.join("en/game.json");
        write_file(&path, r#"{"_comment":1,"_note":false}"#);

        let error = load_translation_map(&path).unwrap_err();

        assert!(matches!(
            error,
            TranslationError::EmptyTranslationFile { .. }
        ));
    }
}
