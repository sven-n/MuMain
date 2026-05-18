# [high] Rust Bevy port implementation

The migration plan in `.features/20260518-0000-rust-bevy-port/plan.md` is complete, but implementation is intentionally pending because the user requested no code in this step. Next step after authorization is `F1.S1.T1 Workspace skeleton`.

# [high] Rust Bevy port phases remaining

`F1.S1.T1` is complete. The migration is still pending from `F1.S1.T2` onward, starting with dependency policy and the app state shell.

# [high] Rust Bevy app state shell

`F1.S1.T2` is complete. The migration is still pending from `F1.S1.T3` onward, starting with `port_rust/crates/mu_client/src/main.rs` and `port_rust/crates/mu_app/src/{lib.rs,state.rs,cli.rs}` for the app state shell.

# [high] Rust Linux CI gate

`F1.S1.T3` is complete. The migration is still pending from `F1.S2.T1` onward, starting with `.github/workflows/rust-client.yml` for Linux Rust CI.

# [high] Rust Windows x64 CI gate

`F1.S2.T1` is complete. The migration is still pending from `F1.S2.T2` onward, starting with `.github/workflows/rust-client-windows.yml` for Windows x64 Rust CI.

# [high] Rust Windows x64 CI evidence

`.github/workflows/rust-client-windows.yml` is written, but `F1.S2.T2` remains blocked until GitHub Actions produces the Windows x64 release artifact, SHA256 checksum, and build log.

# [high] Protocol inventory fixtures

`F2.S1.T1` is complete locally. Continue migration with `F2.S1.T2`, creating protocol inventory fixtures under `port_rust/crates/mu_protocol/tests/fixtures/**` and `port_rust/tests/rust/protocol_inventory.rs`.

# [high] Legacy comparison harness

`F2.S1.T2` is done locally. Next local step is `F2.S1.T3`, adding the comparison harness under `port_rust/crates/mu_test_support` so fixture and evidence tracking can continue.

# [high] Source inventory audit blocked

`F2.S1.T5` is the next planned implementation step after the UI fixture and evidence-doc work. It remains blocked until implementation authorization, and will add the source/dependency classification audit under `port_rust/crates/mu_test_support` and `port_rust/tests/rust/source_inventory.rs`.
