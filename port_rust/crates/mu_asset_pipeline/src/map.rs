use std::collections::BTreeSet;
use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use mu_assets::{sha256_hex, AssetManifestEntry};
use serde::{Deserialize, Serialize};
use thiserror::Error;

use crate::terrain::{
    has_modulus_magic, is_encrypted_terrain_stem, map_file_decrypt, normalized_extension,
    normalized_stem, rows_from_u8, terrain_output_relative_path, terrain_size, write_json_file,
};

const TERRAIN_MAP_KIND: &str = "terrain-map";
const TERRAIN_TILE_COUNT: usize = 256 * 256;
const LAYER_COUNT: usize = 3;

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct TerrainMapJson {
    pub header: TerrainMapHeader,
    pub terrain_size: u32,
    pub layer_stats: TerrainLayerStatsSet,
    pub layer1: Vec<Vec<u8>>,
    pub layer2: Vec<Vec<u8>>,
    pub alpha: Vec<Vec<u8>>,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct TerrainMapHeader {
    pub version: u8,
    pub map_number: u8,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct TerrainLayerStatsSet {
    pub layer1: TerrainLayerStats,
    pub layer2: TerrainLayerStats,
    pub alpha: TerrainLayerStats,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct TerrainLayerStats {
    pub min: u8,
    pub max: u8,
    pub mean: f64,
    pub unique_values: usize,
}

#[derive(Debug, Error)]
pub enum TerrainMapError {
    #[error("unsupported terrain map extension at {path}: {extension}")]
    UnsupportedExtension {
        path: Utf8PathBuf,
        extension: String,
    },
    #[error("terrain map source path escapes the source root: {path}")]
    SourceOutsideRoot { path: Utf8PathBuf },
    #[error("failed to read terrain map {path}: {source}")]
    ReadSource {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
    #[error("failed to parse terrain map {path}: {message}")]
    ParseSource { path: Utf8PathBuf, message: String },
    #[error("failed to write terrain map {path}: {source}")]
    WriteOutput {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
}

pub fn convert_terrain_map_file(
    source_root: &Utf8Path,
    source_path: &Utf8Path,
    output_root: &Utf8Path,
) -> Result<AssetManifestEntry, TerrainMapError> {
    let relative_path =
        source_path
            .strip_prefix(source_root)
            .map_err(|_| TerrainMapError::SourceOutsideRoot {
                path: source_path.to_path_buf(),
            })?;

    let extension = normalized_extension(source_path);
    if extension != "map" {
        return Err(TerrainMapError::UnsupportedExtension {
            path: source_path.to_path_buf(),
            extension,
        });
    }

    let source_bytes = fs::read(source_path).map_err(|source| TerrainMapError::ReadSource {
        path: source_path.to_path_buf(),
        source,
    })?;
    let terrain_map = decode_terrain_map_bytes(source_path, &source_bytes)?;
    let output_file_name = format!("{}.json", source_path.file_name().unwrap_or("terrain.map"));
    let output_relative_path = terrain_output_relative_path(relative_path, &output_file_name);
    let output_path = output_root.join(&output_relative_path);
    let converted_bytes = write_json_file(&output_path, &terrain_map).map_err(|source| {
        TerrainMapError::WriteOutput {
            path: output_path.clone(),
            source,
        }
    })?;

    Ok(AssetManifestEntry {
        source_path: relative_path.to_string(),
        converted_path: output_relative_path.to_string(),
        kind: TERRAIN_MAP_KIND.to_string(),
        source_hash: sha256_hex(&source_bytes),
        converted_hash: sha256_hex(&converted_bytes),
        dependencies: Vec::new(),
    })
}

pub(crate) fn decode_terrain_map_bytes(
    path: &Utf8Path,
    raw: &[u8],
) -> Result<TerrainMapJson, TerrainMapError> {
    if has_modulus_magic(raw) {
        return Err(TerrainMapError::ParseSource {
            path: path.to_path_buf(),
            message: "Season16+ terrain map decryption is not implemented in the Rust port yet"
                .to_string(),
        });
    }

    let mut decoded = if is_encrypted_terrain_stem(&normalized_stem(path)) {
        map_file_decrypt(raw)
    } else {
        raw.to_vec()
    };

    let expected = 2 + TERRAIN_TILE_COUNT * LAYER_COUNT;
    if decoded.len() == expected - 1 {
        decoded.push(0);
    }
    if decoded.len() < expected {
        return Err(TerrainMapError::ParseSource {
            path: path.to_path_buf(),
            message: format!(
                "decoded map payload too small ({len} < {expected})",
                len = decoded.len()
            ),
        });
    }

    let header = TerrainMapHeader {
        version: decoded[0],
        map_number: decoded[1],
    };
    let layer1 = decoded[2..2 + TERRAIN_TILE_COUNT].to_vec();
    let layer2 = decoded[2 + TERRAIN_TILE_COUNT..2 + TERRAIN_TILE_COUNT * 2].to_vec();
    let alpha = decoded[2 + TERRAIN_TILE_COUNT * 2..2 + TERRAIN_TILE_COUNT * 3].to_vec();

    Ok(TerrainMapJson {
        header,
        terrain_size: terrain_size() as u32,
        layer_stats: TerrainLayerStatsSet {
            layer1: layer_stats(&layer1),
            layer2: layer_stats(&layer2),
            alpha: layer_stats(&alpha),
        },
        layer1: rows_from_u8(&layer1),
        layer2: rows_from_u8(&layer2),
        alpha: rows_from_u8(&alpha),
    })
}

fn layer_stats(values: &[u8]) -> TerrainLayerStats {
    let mut unique = BTreeSet::new();
    let mut sum: u64 = 0;
    let mut min = u8::MAX;
    let mut max = u8::MIN;

    for value in values {
        unique.insert(*value);
        sum += u64::from(*value);
        min = min.min(*value);
        max = max.max(*value);
    }

    TerrainLayerStats {
        min,
        max,
        mean: if values.is_empty() {
            0.0
        } else {
            let mean = sum as f64 / values.len() as f64;
            (mean * 100_000.0).round() / 100_000.0
        },
        unique_values: unique.len(),
    }
}

#[cfg(test)]
mod tests {
    use std::fs;
    use std::path::Path;

    use camino::Utf8PathBuf;

    use super::{convert_terrain_map_file, decode_terrain_map_bytes, TerrainMapJson};
    use crate::terrain::terrain_output_relative_path;
    use mu_assets::sha256_hex;

    fn repo_root() -> Utf8PathBuf {
        Utf8PathBuf::from_path_buf(
            Path::new(env!("CARGO_MANIFEST_DIR"))
                .join("../../../")
                .canonicalize()
                .unwrap(),
        )
        .unwrap()
    }

    fn source_path(name: &str) -> Utf8PathBuf {
        repo_root().join(format!("src/bin/Data/World1/{name}"))
    }

    fn temp_root(label: &str) -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_asset_pipeline_terrain_map_{label}_{}_{}",
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

    fn assert_map_conversion(file_name: &str, expected_map_number: u8) {
        let source_root = repo_root().join("src/bin/Data");
        let source = source_path(file_name);
        let output_root = temp_root(file_name);

        let entry = convert_terrain_map_file(&source_root, &source, &output_root).unwrap();
        let output_path = output_root.join(entry.converted_path.clone());
        let output_bytes = fs::read(&output_path).unwrap();
        let golden_bytes = fs::read(golden_path(file_name)).unwrap();
        let json: TerrainMapJson = serde_json::from_slice(&output_bytes).unwrap();

        assert_eq!(entry.kind, "terrain-map");
        assert_eq!(
            entry.converted_path,
            terrain_output_relative_path(
                source.strip_prefix(&source_root).unwrap(),
                &format!("{file_name}.json"),
            )
            .to_string()
        );
        assert_eq!(output_bytes, golden_bytes);
        assert_eq!(entry.source_hash, sha256_hex(&fs::read(&source).unwrap()));
        assert_eq!(entry.converted_hash, sha256_hex(&output_bytes));
        assert_eq!(json.header.version, 0);
        assert_eq!(json.header.map_number, expected_map_number);
        assert_eq!(json.terrain_size, 256);
        assert_eq!(json.layer1.len(), 256);
        assert_eq!(json.layer2.len(), 256);
        assert_eq!(json.alpha.len(), 256);
    }

    fn golden_path(file_name: &str) -> Utf8PathBuf {
        repo_root().join(format!("port_rust/assets/data/world_1/{file_name}.json"))
    }

    #[test]
    fn converts_terrain_map_sidecar() {
        assert_map_conversion("Terrain.map", 5);
    }

    #[test]
    fn converts_encrypted_terrain_map_sidecar() {
        assert_map_conversion("EncTerrain1.map", 1);
    }

    #[test]
    fn decode_terrain_map_bytes_handles_legacy_padding() {
        let source = source_path("Terrain.map");
        let source_bytes = fs::read(&source).unwrap();
        let decoded = decode_terrain_map_bytes(&source, &source_bytes).unwrap();

        assert_eq!(decoded.header.map_number, 5);
    }
}
