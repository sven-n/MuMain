use std::fs;
use std::path::PathBuf;

use camino::Utf8PathBuf;

#[test]
fn runtime_crate_manifests_do_not_reference_mu_asset_pipeline() {
    let crates_root = workspace_root().join("crates");

    for entry in fs::read_dir(&crates_root).unwrap() {
        let entry = entry.unwrap();
        let crate_dir = entry.path();
        if !crate_dir.is_dir() {
            continue;
        }

        let manifest = crate_dir.join("Cargo.toml");
        if !manifest.is_file() {
            continue;
        }

        let crate_name = crate_dir
            .file_name()
            .and_then(|name| name.to_str())
            .unwrap_or_default();

        if crate_name == "mu_asset_pipeline" {
            continue;
        }

        let contents = fs::read_to_string(&manifest).unwrap();
        assert!(
            !contents.contains("mu_asset_pipeline"),
            "{} still references mu_asset_pipeline",
            manifest.display()
        );
    }
}

fn workspace_root() -> Utf8PathBuf {
    let crate_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    let root = crate_dir
        .parent()
        .and_then(|parent| parent.parent())
        .unwrap();
    Utf8PathBuf::from_path_buf(root.to_path_buf()).unwrap()
}
