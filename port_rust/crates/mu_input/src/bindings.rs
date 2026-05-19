use serde::{Deserialize, Serialize};
use std::collections::BTreeMap;

const DEFAULT_BINDING_PAIRS: &[(&str, &str)] = &[
    ("move_forward", "W"),
    ("move_backward", "S"),
    ("move_left", "A"),
    ("move_right", "D"),
    ("jump", "Space"),
    ("attack", "Mouse1"),
    ("interact", "E"),
    ("chat", "Enter"),
    ("inventory", "I"),
    ("options", "Esc"),
    ("toggle_hud", "F12"),
    ("toggle_fullscreen", "Alt+Enter"),
];

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(transparent)]
pub struct Bindings(BTreeMap<String, String>);

impl Bindings {
    pub fn new(bindings: BTreeMap<String, String>) -> Self {
        Self(bindings)
    }

    pub fn defaults() -> Self {
        Self::from_pairs(DEFAULT_BINDING_PAIRS)
    }

    pub fn from_pairs(pairs: &[(&str, &str)]) -> Self {
        let mut bindings = BTreeMap::new();

        for (action, binding) in pairs {
            bindings.insert((*action).to_string(), (*binding).to_string());
        }

        Self(bindings)
    }

    pub fn get(&self, action: &str) -> Option<&str> {
        self.0.get(action).map(String::as_str)
    }

    pub fn insert(
        &mut self,
        action: impl Into<String>,
        binding: impl Into<String>,
    ) -> Option<String> {
        self.0.insert(action.into(), binding.into())
    }

    pub fn iter(&self) -> impl Iterator<Item = (&str, &str)> {
        self.0
            .iter()
            .map(|(action, binding)| (action.as_str(), binding.as_str()))
    }

    pub fn into_inner(self) -> BTreeMap<String, String> {
        self.0
    }
}

impl Default for Bindings {
    fn default() -> Self {
        Self::defaults()
    }
}

#[cfg(test)]
mod tests {
    use super::Bindings;

    #[test]
    fn defaults_include_core_actions() {
        let bindings = Bindings::default();

        assert_eq!(bindings.get("move_forward"), Some("W"));
        assert_eq!(bindings.get("attack"), Some("Mouse1"));
        assert_eq!(bindings.get("toggle_fullscreen"), Some("Alt+Enter"));
    }

    #[test]
    fn bindings_roundtrip_through_toml() {
        let bindings = Bindings::default();
        let toml = toml::to_string_pretty(&bindings).unwrap();
        let decoded: Bindings = toml::from_str(&toml).unwrap();

        assert_eq!(decoded, bindings);
    }
}
