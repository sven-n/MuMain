use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use mu_assets::{sha256_hex, AssetManifestEntry};
use serde::{Deserialize, Serialize};
use thiserror::Error;

use crate::terrain::{
    apply_bux_convert, has_modulus_magic, is_encrypted_terrain_stem, map_file_decrypt,
    normalized_extension, normalized_stem, rows_from_u16, terrain_output_relative_path,
    terrain_size, write_json_file,
};

const TERRAIN_ATTRIBUTE_KIND: &str = "terrain-attribute";
const TERRAIN_TILE_COUNT: usize = 256 * 256;

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct TerrainAttributeJson {
    pub header: TerrainAttributeHeader,
    pub is_extended: bool,
    pub terrain_size: u32,
    pub terrain_data: Vec<Vec<u16>>,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct TerrainAttributeHeader {
    pub version: u8,
    pub map_number: u8,
    pub width: u8,
    pub height: u8,
}

#[derive(Debug, Error)]
pub enum TerrainAttributeError {
    #[error("unsupported terrain attribute extension at {path}: {extension}")]
    UnsupportedExtension {
        path: Utf8PathBuf,
        extension: String,
    },
    #[error("terrain attribute source path escapes the source root: {path}")]
    SourceOutsideRoot { path: Utf8PathBuf },
    #[error("failed to read terrain attribute {path}: {source}")]
    ReadSource {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
    #[error("failed to parse terrain attribute {path}: {message}")]
    ParseSource { path: Utf8PathBuf, message: String },
    #[error("failed to write terrain attribute {path}: {source}")]
    WriteOutput {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
}

pub fn convert_terrain_attribute_file(
    source_root: &Utf8Path,
    source_path: &Utf8Path,
    output_root: &Utf8Path,
) -> Result<AssetManifestEntry, TerrainAttributeError> {
    let relative_path = source_path.strip_prefix(source_root).map_err(|_| {
        TerrainAttributeError::SourceOutsideRoot {
            path: source_path.to_path_buf(),
        }
    })?;

    let extension = normalized_extension(source_path);
    if extension != "att" {
        return Err(TerrainAttributeError::UnsupportedExtension {
            path: source_path.to_path_buf(),
            extension,
        });
    }

    let source_bytes =
        fs::read(source_path).map_err(|source| TerrainAttributeError::ReadSource {
            path: source_path.to_path_buf(),
            source,
        })?;
    let terrain_attribute = decode_terrain_attribute_bytes(source_path, &source_bytes)?;
    let output_file_name = format!("{}.json", source_path.file_stem().unwrap_or("terrain"));
    let output_relative_path = terrain_output_relative_path(relative_path, &output_file_name);
    let output_path = output_root.join(&output_relative_path);
    let converted_bytes = write_json_file(&output_path, &terrain_attribute).map_err(|source| {
        TerrainAttributeError::WriteOutput {
            path: output_path.clone(),
            source,
        }
    })?;

    Ok(AssetManifestEntry {
        source_path: relative_path.to_string(),
        converted_path: output_relative_path.to_string(),
        kind: TERRAIN_ATTRIBUTE_KIND.to_string(),
        source_hash: sha256_hex(&source_bytes),
        converted_hash: sha256_hex(&converted_bytes),
        dependencies: Vec::new(),
    })
}

pub(crate) fn decode_terrain_attribute_bytes(
    path: &Utf8Path,
    raw: &[u8],
) -> Result<TerrainAttributeJson, TerrainAttributeError> {
    let mut decoded = if has_modulus_magic(raw) {
        return Err(TerrainAttributeError::ParseSource {
            path: path.to_path_buf(),
            message:
                "Season16+ terrain attribute decryption is not implemented in the Rust port yet"
                    .to_string(),
        });
    } else if is_encrypted_terrain_stem(&normalized_stem(path)) {
        apply_bux_convert(&map_file_decrypt(raw))
    } else {
        raw.to_vec()
    };

    if matches!(decoded.len(), 65539 | 131075) {
        decoded.push(0);
    }
    if !matches!(decoded.len(), 65540 | 131076) {
        return Err(TerrainAttributeError::ParseSource {
            path: path.to_path_buf(),
            message: format!(
                "unexpected terrain attribute payload size: {}",
                decoded.len()
            ),
        });
    }

    let header = TerrainAttributeHeader {
        version: decoded[0],
        map_number: decoded[1],
        width: decoded[2],
        height: decoded[3],
    };
    let body = &decoded[4..];
    let is_extended = decoded.len() == 131076;

    let values: Vec<u16> = if is_extended {
        body.chunks_exact(2)
            .map(|chunk| u16::from_le_bytes([chunk[0], chunk[1]]))
            .collect()
    } else {
        body.iter().copied().map(u16::from).collect()
    };

    if values.len() != TERRAIN_TILE_COUNT {
        return Err(TerrainAttributeError::ParseSource {
            path: path.to_path_buf(),
            message: format!(
                "invalid terrain attribute sample count: {} (expected {})",
                values.len(),
                TERRAIN_TILE_COUNT
            ),
        });
    }

    Ok(TerrainAttributeJson {
        header,
        is_extended,
        terrain_size: terrain_size() as u32,
        terrain_data: rows_from_u16(&values),
    })
}

#[cfg(test)]
mod tests {
    use std::fs;
    use std::path::Path;

    use camino::Utf8PathBuf;

    use super::{
        convert_terrain_attribute_file, decode_terrain_attribute_bytes, TerrainAttributeJson,
    };
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

    fn golden_path(file_name: &str) -> Utf8PathBuf {
        repo_root().join(format!("port_rust/assets/data/world_1/{file_name}.json"))
    }

    fn temp_root(label: &str) -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_asset_pipeline_terrain_att_{label}_{}_{}",
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

    fn assert_attribute_conversion(
        file_name: &str,
        expected_map_number: u8,
        expected_is_extended: bool,
    ) {
        let source_root = repo_root().join("src/bin/Data");
        let source = source_path(file_name);
        let output_root = temp_root(file_name);

        let entry = convert_terrain_attribute_file(&source_root, &source, &output_root).unwrap();
        let output_path = output_root.join(entry.converted_path.clone());
        let output_bytes = fs::read(&output_path).unwrap();
        let golden_bytes = fs::read(golden_path(file_name.trim_end_matches(".att"))).unwrap();
        let json: TerrainAttributeJson = serde_json::from_slice(&output_bytes).unwrap();

        assert_eq!(entry.kind, "terrain-attribute");
        assert_eq!(
            entry.converted_path,
            terrain_output_relative_path(
                source.strip_prefix(&source_root).unwrap(),
                &format!("{}.json", source.file_stem().unwrap()),
            )
            .to_string()
        );
        assert_eq!(output_bytes, golden_bytes);
        assert_eq!(entry.source_hash, sha256_hex(&fs::read(&source).unwrap()));
        assert_eq!(entry.converted_hash, sha256_hex(&output_bytes));
        assert_eq!(json.header.version, 0);
        assert_eq!(json.header.map_number, expected_map_number);
        assert_eq!(json.is_extended, expected_is_extended);
        assert_eq!(json.terrain_size, 256);
        assert_eq!(json.terrain_data.len(), 256);
    }

    #[test]
    fn converts_terrain_attribute_sidecar() {
        assert_attribute_conversion("Terrain.att", 255, false);
    }

    #[test]
    fn converts_encrypted_terrain_attribute_sidecar() {
        assert_attribute_conversion("EncTerrain1.att", 1, true);
    }

    #[test]
    fn decode_terrain_attribute_bytes_handles_legacy_padding() {
        let source = source_path("Terrain.att");
        let source_bytes = fs::read(&source).unwrap();
        let decoded = decode_terrain_attribute_bytes(&source, &source_bytes).unwrap();

        assert_eq!(decoded.header.map_number, 255);
        assert!(!decoded.is_extended);
    }
}
