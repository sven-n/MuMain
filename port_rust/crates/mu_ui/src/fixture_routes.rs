#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct UiFixtureRoute {
    pub slug: &'static str,
    pub label: &'static str,
    pub fixture_path: &'static str,
}

impl UiFixtureRoute {
    pub const fn new(slug: &'static str, label: &'static str, fixture_path: &'static str) -> Self {
        Self {
            slug,
            label,
            fixture_path,
        }
    }
}

pub const FIXTURE_ROUTES: &[UiFixtureRoute] = &[
    UiFixtureRoute::new("boot", "Boot", "ui_fixtures/boot.snap"),
    UiFixtureRoute::new("loading", "Loading", "ui_fixtures/loading.snap"),
    UiFixtureRoute::new("login", "Login", "ui_fixtures/login.snap"),
    UiFixtureRoute::new(
        "server-select",
        "Server Select",
        "ui_fixtures/server-select.snap",
    ),
    UiFixtureRoute::new("options", "Options", "ui_fixtures/options.snap"),
    UiFixtureRoute::new("messages", "Messages", "ui_fixtures/messages.snap"),
    UiFixtureRoute::new(
        "character-select",
        "Character Select",
        "ui_fixtures/character-select.snap",
    ),
    UiFixtureRoute::new(
        "character-create",
        "Character Create",
        "ui_fixtures/character-create.snap",
    ),
    UiFixtureRoute::new(
        "character-delete",
        "Character Delete",
        "ui_fixtures/character-delete.snap",
    ),
    UiFixtureRoute::new("world", "World", "ui_fixtures/world.snap"),
    UiFixtureRoute::new("hud", "HUD", "ui_fixtures/hud.snap"),
    UiFixtureRoute::new("chat", "Chat", "ui_fixtures/chat.snap"),
    UiFixtureRoute::new("minimap", "Minimap", "ui_fixtures/minimap.snap"),
    UiFixtureRoute::new("hotkeys", "Hotkeys", "ui_fixtures/hotkeys.snap"),
    UiFixtureRoute::new("inventory", "Inventory", "ui_fixtures/inventory.snap"),
    UiFixtureRoute::new("trade", "Trade", "ui_fixtures/trade.snap"),
    UiFixtureRoute::new("vault", "Vault", "ui_fixtures/vault.snap"),
    UiFixtureRoute::new("shop", "Shop", "ui_fixtures/shop.snap"),
    UiFixtureRoute::new("npc", "NPC", "ui_fixtures/npc.snap"),
    UiFixtureRoute::new("gate", "Gate", "ui_fixtures/gate.snap"),
    UiFixtureRoute::new("events", "Events", "ui_fixtures/events.snap"),
    UiFixtureRoute::new("combat", "Combat", "ui_fixtures/combat.snap"),
    UiFixtureRoute::new("duel", "Duel", "ui_fixtures/duel.snap"),
    UiFixtureRoute::new("siege", "Siege", "ui_fixtures/siege.snap"),
    UiFixtureRoute::new("party", "Party", "ui_fixtures/party.snap"),
    UiFixtureRoute::new("friend", "Friend", "ui_fixtures/friend.snap"),
    UiFixtureRoute::new("guild", "Guild", "ui_fixtures/guild.snap"),
    UiFixtureRoute::new("quests", "Quests", "ui_fixtures/quests.snap"),
    UiFixtureRoute::new("mu-helper", "MU Helper", "ui_fixtures/mu-helper.snap"),
    UiFixtureRoute::new("game-shop", "GameShop", "ui_fixtures/game-shop.snap"),
    UiFixtureRoute::new(
        "editor-admin",
        "Editor Admin",
        "ui_fixtures/editor-admin.snap",
    ),
    UiFixtureRoute::new("admin-core", "Admin Core", "ui_fixtures/admin-core.snap"),
    UiFixtureRoute::new(
        "admin-item-editor",
        "Admin Item Editor",
        "ui_fixtures/admin-item-editor.snap",
    ),
    UiFixtureRoute::new(
        "admin-skill-editor",
        "Admin Skill Editor",
        "ui_fixtures/admin-skill-editor.snap",
    ),
    UiFixtureRoute::new("error", "Error", "ui_fixtures/error.snap"),
];

pub fn fixture_routes() -> &'static [UiFixtureRoute] {
    FIXTURE_ROUTES
}
