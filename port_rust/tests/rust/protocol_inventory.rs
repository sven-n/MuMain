use std::path::{Path, PathBuf};

const FIXTURE_DIR: &str = "tests/fixtures";
const EXPECTED_GROUP_COUNT: usize = 14;

struct ProtocolFixture {
    group: &'static str,
    file: &'static str,
}

const PROTOCOL_FIXTURES: &[ProtocolFixture] = &[
    ProtocolFixture {
        group: "connection_manager",
        file: "connection_manager.txt",
    },
    ProtocolFixture {
        group: "connect_server",
        file: "connect_server.txt",
    },
    ProtocolFixture {
        group: "chat_server",
        file: "chat_server.txt",
    },
    ProtocolFixture {
        group: "session_login_chat",
        file: "session_login_chat.txt",
    },
    ProtocolFixture {
        group: "player_shop_items_vault",
        file: "player_shop_items_vault.txt",
    },
    ProtocolFixture {
        group: "npc_warp_world",
        file: "npc_warp_world.txt",
    },
    ProtocolFixture {
        group: "character_progression",
        file: "character_progression.txt",
    },
    ProtocolFixture {
        group: "combat_skills_pets",
        file: "combat_skills_pets.txt",
    },
    ProtocolFixture {
        group: "trade_mail_friend_social",
        file: "trade_mail_friend_social.txt",
    },
    ProtocolFixture {
        group: "guild_alliance_castle",
        file: "guild_alliance_castle.txt",
    },
    ProtocolFixture {
        group: "game_shop_cash_shop",
        file: "game_shop_cash_shop.txt",
    },
    ProtocolFixture {
        group: "mu_helper",
        file: "mu_helper.txt",
    },
    ProtocolFixture {
        group: "quests_events_minigames",
        file: "quests_events_minigames.txt",
    },
    ProtocolFixture {
        group: "region_faction_duel",
        file: "region_faction_duel.txt",
    },
];

fn fixture_path(file: &str) -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .join(FIXTURE_DIR)
        .join(file)
}

#[cfg(test)]
mod tests {
    use super::{fixture_path, EXPECTED_GROUP_COUNT, PROTOCOL_FIXTURES};
    use std::collections::BTreeSet;
    use std::fs;

    #[test]
    fn protocol_inventory_fixtures_cover_all_groups() {
        assert_eq!(PROTOCOL_FIXTURES.len(), EXPECTED_GROUP_COUNT);

        let mut groups = BTreeSet::new();
        let mut files = BTreeSet::new();

        for fixture in PROTOCOL_FIXTURES {
            assert!(
                groups.insert(fixture.group),
                "duplicate protocol group: {}",
                fixture.group
            );
            assert!(
                files.insert(fixture.file),
                "duplicate fixture file: {}",
                fixture.file
            );

            let contents = fs::read_to_string(fixture_path(fixture.file))
                .unwrap_or_else(|err| panic!("missing fixture {}: {}", fixture.file, err));
            assert!(
                contents.contains(&format!("group={}", fixture.group)),
                "fixture {} did not declare its group",
                fixture.file
            );
            assert!(
                contents.contains("status=placeholder"),
                "fixture {} was not marked as a placeholder",
                fixture.file
            );
        }

        assert_eq!(groups.len(), EXPECTED_GROUP_COUNT);
        assert_eq!(files.len(), EXPECTED_GROUP_COUNT);
    }
}
