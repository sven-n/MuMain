use camino::{Utf8Path, Utf8PathBuf};
use mu_audio::Settings as AudioSettings;
use mu_input::Bindings;
use serde::{Deserialize, Serialize};
use std::fs;
use std::io;
use thiserror::Error;

const CONFIG_DIR: &str = "config";
const CONFIG_FILE_NAME: &str = "client.toml";
const DEFAULT_WINDOW_WIDTH: u32 = 1024;
const DEFAULT_WINDOW_HEIGHT: u32 = 768;
const DEFAULT_FULLSCREEN: bool = false;
const DEFAULT_VSYNC: bool = true;
const DEFAULT_FPS_LIMIT: u32 = 0;
const DEFAULT_REDUCE_EFFECTS: bool = false;
const DEFAULT_LAST_SERVER: &str = "127.127.127.127:44406";
const DEFAULT_LANGUAGE: &str = "en";
const MIN_WINDOW_DIMENSION: u32 = 1;

#[derive(Debug, Clone, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(default)]
pub struct Config {
    pub video: VideoSettings,
    pub audio: AudioSettings,
    pub controls: ControlsSettings,
    pub performance: PerformanceSettings,
    pub network: NetworkSettings,
    pub locale: LocaleSettings,
}

impl Config {
    pub fn load(path: impl AsRef<Utf8Path>) -> Result<Self, ConfigError> {
        let path = path.as_ref();
        let contents = match fs::read_to_string(path) {
            Ok(contents) => contents,
            Err(error) if error.kind() == io::ErrorKind::NotFound => return Ok(Self::default()),
            Err(source) => {
                return Err(ConfigError::Io {
                    path: path.to_path_buf(),
                    source,
                });
            }
        };

        let config = toml::from_str::<Self>(&contents).map_err(|source| ConfigError::Parse {
            path: path.to_path_buf(),
            source,
        })?;

        Ok(config.normalized())
    }

    pub fn load_from_root(root: impl AsRef<Utf8Path>) -> Result<Self, ConfigError> {
        Self::load(Self::path_in(root))
    }

    pub fn save(&self, path: impl AsRef<Utf8Path>) -> Result<(), ConfigError> {
        let path = path.as_ref();
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent).map_err(|source| ConfigError::Io {
                path: parent.to_path_buf(),
                source,
            })?;
        }

        let contents = toml::to_string_pretty(&self.clone().normalized()).map_err(|source| {
            ConfigError::Serialize {
                path: path.to_path_buf(),
                source,
            }
        })?;
        fs::write(path, contents).map_err(|source| ConfigError::Io {
            path: path.to_path_buf(),
            source,
        })?;

        Ok(())
    }

    pub fn save_to_root(&self, root: impl AsRef<Utf8Path>) -> Result<(), ConfigError> {
        self.save(Self::path_in(root))
    }

    pub fn path_in(root: impl AsRef<Utf8Path>) -> Utf8PathBuf {
        root.as_ref().join(CONFIG_DIR).join(CONFIG_FILE_NAME)
    }

    pub fn normalized(mut self) -> Self {
        self.video = self.video.normalized();
        self.audio = self.audio.normalized();
        self.controls = self.controls.normalized();
        self.performance = self.performance.normalized();
        self.network = self.network.normalized();
        self.locale = self.locale.normalized();
        self
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(default)]
pub struct VideoSettings {
    pub width: u32,
    pub height: u32,
    pub fullscreen: bool,
    pub vsync: bool,
    pub fps_limit: u32,
    pub reduce_effects: bool,
}

impl VideoSettings {
    pub fn normalized(mut self) -> Self {
        self.width = self.width.max(MIN_WINDOW_DIMENSION);
        self.height = self.height.max(MIN_WINDOW_DIMENSION);
        self
    }
}

impl Default for VideoSettings {
    fn default() -> Self {
        Self {
            width: DEFAULT_WINDOW_WIDTH,
            height: DEFAULT_WINDOW_HEIGHT,
            fullscreen: DEFAULT_FULLSCREEN,
            vsync: DEFAULT_VSYNC,
            fps_limit: DEFAULT_FPS_LIMIT,
            reduce_effects: DEFAULT_REDUCE_EFFECTS,
        }
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(default)]
pub struct ControlsSettings {
    pub bindings: Bindings,
}

impl ControlsSettings {
    pub fn normalized(self) -> Self {
        self
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(default)]
pub struct PerformanceSettings {
    pub overlays: bool,
}

impl PerformanceSettings {
    pub fn normalized(self) -> Self {
        self
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(default)]
pub struct NetworkSettings {
    pub last_server: String,
    pub remember_username: bool,
}

impl NetworkSettings {
    pub fn normalized(mut self) -> Self {
        let trimmed = self.last_server.trim();
        self.last_server = if trimmed.is_empty() {
            DEFAULT_LAST_SERVER.to_string()
        } else {
            trimmed.to_string()
        };
        self
    }
}

impl Default for NetworkSettings {
    fn default() -> Self {
        Self {
            last_server: DEFAULT_LAST_SERVER.to_string(),
            remember_username: false,
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(default)]
pub struct LocaleSettings {
    pub language: String,
}

impl LocaleSettings {
    pub fn normalized(mut self) -> Self {
        self.language = canonical_locale(&self.language);
        self
    }
}

impl Default for LocaleSettings {
    fn default() -> Self {
        Self {
            language: DEFAULT_LANGUAGE.to_string(),
        }
    }
}

#[derive(Debug, Error)]
pub enum ConfigError {
    #[error("failed to read config `{path}`: {source}")]
    Io {
        path: Utf8PathBuf,
        source: io::Error,
    },
    #[error("failed to parse config `{path}`: {source}")]
    Parse {
        path: Utf8PathBuf,
        source: toml::de::Error,
    },
    #[error("failed to serialize config `{path}`: {source}")]
    Serialize {
        path: Utf8PathBuf,
        source: toml::ser::Error,
    },
}

fn canonical_locale(value: &str) -> String {
    match value.trim().to_ascii_lowercase().as_str() {
        "eng" | "en" => "en".to_string(),
        "de" => "de".to_string(),
        "es" => "es".to_string(),
        "id" => "id".to_string(),
        "pl" => "pl".to_string(),
        "pt" | "pt-br" => "pt".to_string(),
        "ru" => "ru".to_string(),
        "tl" => "tl".to_string(),
        "uk" => "uk".to_string(),
        "zh-tw" | "zh_tw" => "zh-TW".to_string(),
        other => other.to_string(),
    }
}

#[cfg(test)]
mod tests {
    use super::{Config, LocaleSettings};
    use camino::Utf8PathBuf;
    use std::fs;

    fn temp_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_app_config_{}_{}",
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

    #[test]
    fn load_missing_config_returns_defaults() {
        let root = temp_root();
        let path = Config::path_in(&root);

        let config = Config::load(&path).unwrap();

        assert_eq!(config, Config::default());
    }

    #[test]
    fn save_and_load_roundtrip_preserves_settings_without_secrets() {
        let root = temp_root();
        let mut config = Config::default();
        config.video.width = 1920;
        config.video.height = 1080;
        config.video.fullscreen = true;
        config.video.vsync = false;
        config.video.fps_limit = 144;
        config.video.reduce_effects = true;
        config.audio.master = 8;
        config.audio.music = 7;
        config.audio.effects = 6;
        config.audio.mute = true;
        config.controls.bindings.insert("inventory", "Tab");
        config.performance.overlays = true;
        config.network.last_server = "127.0.0.1:44405".to_string();
        config.network.remember_username = true;
        config.locale = LocaleSettings {
            language: "Eng".to_string(),
        };

        let path = Config::path_in(&root);
        config.save(&path).unwrap();

        assert!(path.exists());
        let contents = fs::read_to_string(&path).unwrap();
        assert!(!contents.contains("password"));
        assert!(!contents.contains("token"));
        assert!(!contents.contains("session"));

        let loaded = Config::load(&path).unwrap();
        assert_eq!(loaded.video.width, 1920);
        assert_eq!(loaded.video.height, 1080);
        assert!(loaded.video.fullscreen);
        assert!(!loaded.video.vsync);
        assert_eq!(loaded.video.fps_limit, 144);
        assert!(loaded.video.reduce_effects);
        assert_eq!(loaded.audio.master, 8);
        assert_eq!(loaded.audio.music, 7);
        assert_eq!(loaded.audio.effects, 6);
        assert!(loaded.audio.mute);
        assert_eq!(loaded.controls.bindings.get("inventory"), Some("Tab"));
        assert!(loaded.performance.overlays);
        assert_eq!(loaded.network.last_server, "127.0.0.1:44405");
        assert!(loaded.network.remember_username);
        assert_eq!(loaded.locale.language, "en");
    }

    #[test]
    fn canonical_locale_normalizes_expected_aliases() {
        let locale = LocaleSettings {
            language: "zh_tw".to_string(),
        }
        .normalized();

        assert_eq!(locale.language, "zh-TW");
    }
}
