use serde::{Deserialize, Serialize};

const DEFAULT_MASTER_VOLUME: u8 = 5;
const DEFAULT_MUSIC_VOLUME: u8 = 5;
const DEFAULT_EFFECTS_VOLUME: u8 = 5;
const DEFAULT_MUTE: bool = false;
const MIN_VOLUME: u8 = 0;
const MAX_VOLUME: u8 = 10;

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(default)]
pub struct Settings {
    pub master: u8,
    pub music: u8,
    pub effects: u8,
    pub mute: bool,
}

impl Settings {
    pub fn normalized(mut self) -> Self {
        self.master = self.master.clamp(MIN_VOLUME, MAX_VOLUME);
        self.music = self.music.clamp(MIN_VOLUME, MAX_VOLUME);
        self.effects = self.effects.clamp(MIN_VOLUME, MAX_VOLUME);
        self
    }

    pub fn is_muted(&self) -> bool {
        self.mute || self.master == MIN_VOLUME
    }
}

impl Default for Settings {
    fn default() -> Self {
        Self {
            master: DEFAULT_MASTER_VOLUME,
            music: DEFAULT_MUSIC_VOLUME,
            effects: DEFAULT_EFFECTS_VOLUME,
            mute: DEFAULT_MUTE,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::Settings;

    #[test]
    fn defaults_match_expected_volume_floor() {
        let settings = Settings::default();

        assert_eq!(settings.master, 5);
        assert_eq!(settings.music, 5);
        assert_eq!(settings.effects, 5);
        assert!(!settings.mute);
    }

    #[test]
    fn settings_roundtrip_through_toml() {
        let settings = Settings {
            master: 8,
            music: 7,
            effects: 4,
            mute: true,
        };
        let toml = toml::to_string_pretty(&settings).unwrap();
        let decoded: Settings = toml::from_str(&toml).unwrap();

        assert_eq!(decoded, settings);
    }

    #[test]
    fn normalized_clamps_volume_levels() {
        let settings = Settings {
            master: 15,
            music: 11,
            effects: 42,
            mute: false,
        }
        .normalized();

        assert_eq!(settings.master, 10);
        assert_eq!(settings.music, 10);
        assert_eq!(settings.effects, 10);
        assert!(!settings.is_muted());
    }
}
