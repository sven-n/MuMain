//! Canonical inventory for legacy top-level source paths.

use std::fmt::Write;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum SourceClassification {
    Ported,
    Replaced,
    FixtureOnly,
    ReferenceOnly,
    Rejected,
}

impl SourceClassification {
    const fn index(self) -> usize {
        match self {
            Self::Ported => 0,
            Self::Replaced => 1,
            Self::FixtureOnly => 2,
            Self::ReferenceOnly => 3,
            Self::Rejected => 4,
        }
    }

    pub const fn as_str(self) -> &'static str {
        match self {
            Self::Ported => "ported",
            Self::Replaced => "replaced",
            Self::FixtureOnly => "fixture-only",
            Self::ReferenceOnly => "reference-only",
            Self::Rejected => "rejected",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SourceEntry {
    pub name: &'static str,
    pub classification: SourceClassification,
    pub target: &'static str,
    pub note: &'static str,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SourceRoot {
    pub root: &'static str,
    pub entries: &'static [SourceEntry],
}

macro_rules! source_root {
    ($root:expr, [$( $name:expr => $classification:ident, $target:expr, $note:expr ),* $(,)?]) => {
        SourceRoot {
            root: $root,
            entries: &[
                $(
                    SourceEntry {
                        name: $name,
                        classification: SourceClassification::$classification,
                        target: $target,
                        note: $note,
                    },
                )*
            ],
        }
    };
}

pub const SOURCE_ROOTS: &[SourceRoot] = &[
    source_root!("ClientLibrary", [
        ".gitignore" => ReferenceOnly, "source comparison inputs", "keeps generated binding noise out of diffs",
        "Common.xslt" => ReferenceOnly, "source comparison inputs", "shared XSLT used by the legacy binding pipeline",
        "ConnectionManager.ChatServer.Custom.cs" => ReferenceOnly, "mu_protocol", "chat-server packet reference",
        "ConnectionManager.ChatServerFunctions.cs" => ReferenceOnly, "mu_protocol", "chat-server packet reference",
        "ConnectionManager.ClientToServer.Custom.cs" => ReferenceOnly, "mu_protocol", "client-to-server packet reference",
        "ConnectionManager.ClientToServerFunctions.cs" => ReferenceOnly, "mu_protocol", "client-to-server packet reference",
        "ConnectionManager.ConnectServerFunctions.cs" => ReferenceOnly, "mu_protocol", "connect-server packet reference",
        "ConnectionManager.cs" => ReferenceOnly, "mu_network", "connection manager bridge reference",
        "ConnectionWrapper.cs" => ReferenceOnly, "mu_network", "socket wrapper reference",
        "GenerateBindingsHeader.xslt" => ReferenceOnly, "source comparison inputs", "legacy binding generator reference",
        "GenerateEnums.xslt" => ReferenceOnly, "source comparison inputs", "legacy binding generator reference",
        "GenerateExtensionsDotNet.xslt" => ReferenceOnly, "source comparison inputs", "legacy binding generator reference",
        "GenerateFunctions.xslt" => ReferenceOnly, "source comparison inputs", "legacy binding generator reference",
        "GenerateFunctionsHeader.xslt" => ReferenceOnly, "source comparison inputs", "legacy binding generator reference",
        "MUnique.Client.Library.csproj" => ReferenceOnly, "source comparison inputs", "project metadata for the legacy comparison build",
        "Properties" => ReferenceOnly, "source comparison inputs", "assembly metadata reference",
    ]),
    source_root!("src/source", [
        "Audio" => ReferenceOnly, "mu_audio", "legacy audio runtime reference",
        "Camera" => ReferenceOnly, "mu_input / mu_render", "camera control reference",
        "Character" => ReferenceOnly, "mu_gameplay", "character systems reference",
        "Core" => ReferenceOnly, "mu_core / mu_app / mu_input", "shared runtime services reference",
        "Data" => ReferenceOnly, "mu_assets", "configuration and data loader reference",
        "Dotnet" => ReferenceOnly, "mu_protocol / mu_network", ".NET bridge reference",
        "Engine" => ReferenceOnly, "mu_render / mu_gameplay", "engine loop reference",
        "ExternalObject" => ReferenceOnly, "mu_gameplay / mu_render", "external entity reference",
        "GameLogic" => ReferenceOnly, "mu_gameplay", "gameplay rules reference",
        "GameShop" => ReferenceOnly, "mu_gameplay / mu_ui", "game shop reference",
        "Guild" => ReferenceOnly, "mu_gameplay / mu_ui", "guild and social reference",
        "MUHelper" => ReferenceOnly, "mu_gameplay / mu_ui", "MU Helper reference",
        "Network" => ReferenceOnly, "mu_network", "legacy transport reference",
        "Platform" => ReferenceOnly, "mu_core / mu_app", "platform abstraction reference",
        "Render" => ReferenceOnly, "mu_render", "rendering reference",
        "Scenes" => ReferenceOnly, "mu_app", "scene routing reference",
        "StdAfx.cpp" => ReferenceOnly, "mu_core", "legacy precompiled-header translation unit",
        "UI" => ReferenceOnly, "mu_ui", "UI layout reference",
        "World" => ReferenceOnly, "mu_gameplay / mu_render", "world and terrain reference",
        "stdafx.h" => ReferenceOnly, "mu_core", "legacy precompiled-header header",
    ]),
    source_root!("src/MuEditor", [
        "Config" => ReferenceOnly, "mu_editor_admin", "editor config reference",
        "Core" => ReferenceOnly, "mu_editor_admin", "editor runtime reference",
        "UI" => ReferenceOnly, "mu_ui / mu_editor_admin", "editor UI reference",
        "README.md" => ReferenceOnly, "editor docs", "legacy editor usage reference",
    ]),
    source_root!("src/bin", [
        "Data" => FixtureOnly, "mu_asset_pipeline / mu_assets", "legacy asset root for converter and inventory fixtures",
        "Translations" => FixtureOnly, "mu_ui::i18n", "legacy locale bundles for translation fixtures",
        "config.ini.template" => ReferenceOnly, "mu_app::config", "legacy config template reference",
        "glew32.dll" => Rejected, "none", "legacy Windows OpenGL binary is not shipped by the Rust client",
        "ogg.dll" => Rejected, "none", "legacy audio binary is not shipped by the Rust client",
        "vorbisfile.dll" => Rejected, "none", "legacy audio binary is not shipped by the Rust client",
    ]),
    source_root!("src/ThirdParty", [
        "SDL" => ReferenceOnly, "mu_render", "legacy window/input backend reference",
        "SDL_mixer" => ReferenceOnly, "mu_audio", "legacy audio backend reference",
        "imgui" => ReferenceOnly, "mu_ui", "legacy debug UI reference",
        "json.hpp" => ReferenceOnly, "mu_protocol / mu_assets", "third-party JSON reference",
    ]),
    source_root!("src/dependencies", [
        ".gitattributes" => ReferenceOnly, "packaging rules", "legacy dependency repo metadata",
        "include" => ReferenceOnly, "mu_render / mu_asset_pipeline", "native include header reference",
        "lib" => Rejected, "none", "legacy binary libraries are not shipped by the Rust client",
        "netcore" => ReferenceOnly, "mu_protocol / mu_network", "native .NET support reference",
    ]),
    source_root!("tests", [
        "CMakeLists.txt" => ReferenceOnly, "Rust test fixtures", "legacy C++ test harness reference",
        "main.cpp" => ReferenceOnly, "Rust test fixtures", "legacy test entrypoint reference",
        "text" => ReferenceOnly, "text wrapping regression fixture", "legacy text suite reference",
        "third_party" => ReferenceOnly, "Rust test fixtures", "vendored doctest reference",
    ]),
];

pub fn source_inventory_roots() -> &'static [SourceRoot] {
    SOURCE_ROOTS
}

pub fn source_inventory_report() -> String {
    let mut counts = [0usize; 5];
    let mut total = 0usize;

    for root in SOURCE_ROOTS {
        for entry in root.entries {
            counts[entry.classification.index()] += 1;
            total += 1;
        }
    }

    let mut report = String::new();
    writeln!(&mut report, "source inventory").unwrap();
    writeln!(&mut report, "roots={} entries={total}", SOURCE_ROOTS.len()).unwrap();
    writeln!(
        &mut report,
        "ported={} replaced={} fixture-only={} reference-only={} rejected={}",
        counts[SourceClassification::Ported.index()],
        counts[SourceClassification::Replaced.index()],
        counts[SourceClassification::FixtureOnly.index()],
        counts[SourceClassification::ReferenceOnly.index()],
        counts[SourceClassification::Rejected.index()],
    )
    .unwrap();

    for root in SOURCE_ROOTS {
        writeln!(&mut report, "[{}] count={}", root.root, root.entries.len()).unwrap();
        for entry in root.entries {
            writeln!(
                &mut report,
                "{} | {} | {} | {}",
                entry.name,
                entry.classification.as_str(),
                entry.target,
                entry.note
            )
            .unwrap();
        }
    }

    report
}

#[cfg(test)]
mod tests {
    use super::{source_inventory_report, source_inventory_roots};
    use std::collections::BTreeSet;

    #[test]
    fn source_inventory_root_set_is_stable() {
        let actual = source_inventory_roots()
            .iter()
            .map(|root| root.root)
            .collect::<BTreeSet<_>>();
        let expected = [
            "ClientLibrary",
            "src/source",
            "src/MuEditor",
            "src/bin",
            "src/ThirdParty",
            "src/dependencies",
            "tests",
        ]
        .into_iter()
        .collect::<BTreeSet<_>>();

        assert_eq!(actual, expected);
    }

    #[test]
    fn source_inventory_report_summarizes_the_current_audit() {
        let report = source_inventory_report();

        assert!(report.contains("source inventory"));
        assert!(report.contains("roots=7"));
        assert!(report.contains("entries=58"));
        assert!(report.contains("fixture-only=2"));
        assert!(report.contains("reference-only=52"));
        assert!(report.contains("rejected=4"));
    }
}
