use std::collections::BTreeSet;

use mu_ui::fixture_routes;

const SNAPSHOT_MANIFEST: &str = include_str!("ui_fixtures/manifest.txt");

const EXPECTED_SLUGS: &[&str] = &[
    "boot",
    "loading",
    "login",
    "server-select",
    "options",
    "messages",
    "character-select",
    "character-create",
    "character-delete",
    "world",
    "hud",
    "chat",
    "minimap",
    "hotkeys",
    "inventory",
    "trade",
    "vault",
    "shop",
    "npc",
    "gate",
    "events",
    "combat",
    "duel",
    "siege",
    "party",
    "friend",
    "guild",
    "quests",
    "mu-helper",
    "game-shop",
    "editor-admin",
    "admin-core",
    "admin-item-editor",
    "admin-skill-editor",
    "error",
];

#[test]
fn fixture_manifest_matches_snapshot() {
    let actual = fixture_routes()
        .iter()
        .map(|route| format!("{}|{}|{}", route.slug, route.label, route.fixture_path))
        .collect::<Vec<_>>()
        .join("\n");

    assert_eq!(SNAPSHOT_MANIFEST, format!("{actual}\n"));
}

#[test]
fn fixture_routes_cover_expected_surfaces_once() {
    let routes = fixture_routes();
    let actual_slugs = routes.iter().map(|route| route.slug).collect::<Vec<_>>();
    let unique_slugs = actual_slugs.iter().copied().collect::<BTreeSet<_>>();
    let expected_slugs = EXPECTED_SLUGS.iter().copied().collect::<BTreeSet<_>>();

    assert_eq!(actual_slugs.len(), unique_slugs.len(), "duplicate UI fixture slugs");
    assert_eq!(unique_slugs, expected_slugs);
}
