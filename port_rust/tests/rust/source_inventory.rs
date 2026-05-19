use std::collections::BTreeSet;
use std::fs;
use std::path::{Path, PathBuf};

use mu_test_support::source_inventory::{source_inventory_report, source_inventory_roots};

const LEGACY_ROOTS: &[&str] = &[
    "ClientLibrary",
    "src/source",
    "src/MuEditor",
    "src/bin",
    "src/ThirdParty",
    "src/dependencies",
    "tests",
];

fn repo_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("../../../")
}

fn read_top_level_entries(root: &Path) -> BTreeSet<String> {
    fs::read_dir(root)
        .unwrap_or_else(|err| panic!("missing legacy root {}: {}", root.display(), err))
        .map(|entry| {
            entry
                .unwrap_or_else(|err| panic!("failed to read {}: {}", root.display(), err))
                .file_name()
                .to_string_lossy()
                .into_owned()
        })
        .collect()
}

#[test]
fn source_inventory_covers_every_legacy_root_once() {
    let actual_roots = source_inventory_roots()
        .iter()
        .map(|root| root.root)
        .collect::<BTreeSet<_>>();
    let expected_roots = LEGACY_ROOTS.iter().copied().collect::<BTreeSet<_>>();

    assert_eq!(actual_roots, expected_roots);
}

#[test]
fn source_inventory_matches_the_current_top_level_legacy_paths() {
    let repo_root = repo_root();

    for root in source_inventory_roots() {
        let actual = read_top_level_entries(&repo_root.join(root.root));
        let expected = root
            .entries
            .iter()
            .map(|entry| entry.name.to_owned())
            .collect::<BTreeSet<_>>();

        assert_eq!(actual, expected, "inventory mismatch for {}", root.root);
    }

    let report = source_inventory_report();
    assert!(report.contains("source inventory"));
    assert!(report.contains("fixture-only=2"));
    assert!(report.contains("reference-only=52"));
    assert!(report.contains("rejected=4"));
}
