use camino::{Utf8Path, Utf8PathBuf};
use mu_assets::{
    available_locales, load_translation_bundle, TranslationBundle, TranslationDomain,
    TranslationError,
};

#[derive(Debug, Clone)]
pub struct Translator {
    translations_root: Utf8PathBuf,
    bundle: TranslationBundle,
}

impl Translator {
    pub fn load(
        translations_root: impl AsRef<Utf8Path>,
        locale: &str,
    ) -> Result<Self, TranslationError> {
        let translations_root = translations_root.as_ref().to_path_buf();
        let bundle = load_translation_bundle(&translations_root, locale)?;

        Ok(Self {
            translations_root,
            bundle,
        })
    }

    pub fn locale(&self) -> &str {
        &self.bundle.locale
    }

    pub fn language_name(&self) -> &str {
        &self.bundle.language_name
    }

    pub fn available_locales(&self) -> Result<Vec<String>, TranslationError> {
        available_locales(&self.translations_root)
    }

    pub fn switch_language(&mut self, locale: &str) -> Result<(), TranslationError> {
        self.bundle = load_translation_bundle(&self.translations_root, locale)?;
        Ok(())
    }

    pub fn has_translation(&self, domain: TranslationDomain, key: &str) -> bool {
        self.bundle.has_translation(domain, key)
    }

    pub fn translate(&self, domain: TranslationDomain, key: &str) -> String {
        self.translate_with_fallback(domain, key, key)
    }

    pub fn translate_with_fallback(
        &self,
        domain: TranslationDomain,
        key: &str,
        fallback: &str,
    ) -> String {
        self.bundle
            .lookup(domain, key)
            .map(str::to_owned)
            .unwrap_or_else(|| fallback.to_owned())
    }

    pub fn format(&self, domain: TranslationDomain, key: &str, args: &[&str]) -> String {
        format_placeholders(&self.translate(domain, key), args)
    }
}

fn format_placeholders(template: &str, args: &[&str]) -> String {
    let mut output = template.to_owned();

    for (index, arg) in args.iter().enumerate() {
        let placeholder = format!("{{{index}}}");
        let mut search_from = 0;

        while let Some(offset) = output[search_from..].find(&placeholder) {
            let start = search_from + offset;
            let end = start + placeholder.len();
            output.replace_range(start..end, arg);
            search_from = start + arg.len();
        }
    }

    output
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;

    fn temp_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_ui_i18n_{}_{}",
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
    fn translator_falls_back_to_key_and_custom_text() {
        let root = temp_root();
        write_file(
            &root.join("en/game.json"),
            r#"{"hello":"Hello","error_file_not_found":"File not found: {0}"}"#,
        );
        write_file(
            &root.join("en/editor.json"),
            r#"{"language_name":"English"}"#,
        );

        let translator = Translator::load(&root, "en").unwrap();

        assert_eq!(translator.locale(), "en");
        assert_eq!(translator.language_name(), "English");
        assert_eq!(
            translator.translate(TranslationDomain::Game, "missing"),
            "missing"
        );
        assert_eq!(
            translator.translate_with_fallback(TranslationDomain::Game, "missing", "fallback"),
            "fallback"
        );
        assert_eq!(
            translator.format(
                TranslationDomain::Game,
                "error_file_not_found",
                &["data.txt"]
            ),
            "File not found: data.txt"
        );
    }

    #[test]
    fn translator_switches_locale_without_losing_fallback_behavior() {
        let root = temp_root();
        write_file(&root.join("en/game.json"), r#"{"hello":"Hello"}"#);
        write_file(
            &root.join("en/editor.json"),
            r#"{"language_name":"English"}"#,
        );
        write_file(&root.join("pt/game.json"), r#"{"hello":"Olá"}"#);
        write_file(
            &root.join("pt/editor.json"),
            r#"{"language_name":"Português"}"#,
        );

        let mut translator = Translator::load(&root, "en").unwrap();
        assert_eq!(
            translator.translate(TranslationDomain::Game, "hello"),
            "Hello"
        );

        translator.switch_language("pt").unwrap();

        assert_eq!(translator.locale(), "pt");
        assert_eq!(translator.language_name(), "Português");
        assert_eq!(
            translator.translate(TranslationDomain::Game, "hello"),
            "Olá"
        );
        assert_eq!(
            translator.translate(TranslationDomain::Editor, "missing"),
            "missing"
        );
    }
}
