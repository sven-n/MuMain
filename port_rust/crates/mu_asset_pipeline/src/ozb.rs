use std::collections::BTreeSet;
use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use mu_assets::{sha256_hex, AssetManifestEntry};
use serde::{Deserialize, Serialize};
use thiserror::Error;

use crate::terrain::{
    height_multiplier_for_path, normalized_extension, terrain_output_relative_path, terrain_scale,
    terrain_size, write_json_file,
};

const TERRAIN_HEIGHT_KIND: &str = "terrain-height";
const TERRAIN_TILE_COUNT: usize = 256 * 256;
const LEGACY_OZB_TERRAIN_HEADER: usize = 1080;
const LEGACY_OZB_TERRAIN_PIXELS: usize = TERRAIN_TILE_COUNT;

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct TerrainHeightJson {
    pub width: u32,
    pub height: u32,
    pub heights: Vec<Vec<f64>>,
    pub metadata: TerrainHeightMetadata,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct TerrainHeightMetadata {
    pub source: String,
    pub source_bits_per_pixel: u8,
    pub row_order: String,
    pub height_multiplier: f64,
    pub height_offset: f64,
    pub legacy_terrain_scale: f64,
    pub source_unique_values: usize,
    pub source_min: u32,
    pub source_max: u32,
    pub normalized_sample_min: f64,
    pub normalized_sample_max: f64,
}

#[derive(Debug, Error)]
pub enum TerrainHeightError {
    #[error("unsupported terrain height extension at {path}: {extension}")]
    UnsupportedExtension {
        path: Utf8PathBuf,
        extension: String,
    },
    #[error("terrain height source path escapes the source root: {path}")]
    SourceOutsideRoot { path: Utf8PathBuf },
    #[error("failed to read terrain height {path}: {source}")]
    ReadSource {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
    #[error("failed to parse terrain height {path}: {message}")]
    ParseSource { path: Utf8PathBuf, message: String },
    #[error("failed to write terrain height {path}: {source}")]
    WriteOutput {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
}

pub fn convert_terrain_height_file(
    source_root: &Utf8Path,
    source_path: &Utf8Path,
    output_root: &Utf8Path,
) -> Result<AssetManifestEntry, TerrainHeightError> {
    let relative_path = source_path.strip_prefix(source_root).map_err(|_| {
        TerrainHeightError::SourceOutsideRoot {
            path: source_path.to_path_buf(),
        }
    })?;

    let extension = normalized_extension(source_path);
    if extension != "ozb" {
        return Err(TerrainHeightError::UnsupportedExtension {
            path: source_path.to_path_buf(),
            extension,
        });
    }

    let source_bytes = fs::read(source_path).map_err(|source| TerrainHeightError::ReadSource {
        path: source_path.to_path_buf(),
        source,
    })?;
    let terrain_height = decode_terrain_height_bytes(source_path, &source_bytes)?;
    let output_relative_path = terrain_output_relative_path(relative_path, "terrain_height.json");
    let output_path = output_root.join(&output_relative_path);
    let converted_bytes = write_json_file(&output_path, &terrain_height).map_err(|source| {
        TerrainHeightError::WriteOutput {
            path: output_path.clone(),
            source,
        }
    })?;

    Ok(AssetManifestEntry {
        source_path: relative_path.to_string(),
        converted_path: output_relative_path.to_string(),
        kind: TERRAIN_HEIGHT_KIND.to_string(),
        source_hash: sha256_hex(&source_bytes),
        converted_hash: sha256_hex(&converted_bytes),
        dependencies: Vec::new(),
    })
}

pub(crate) fn decode_terrain_height_bytes(
    path: &Utf8Path,
    raw: &[u8],
) -> Result<TerrainHeightJson, TerrainHeightError> {
    let (samples, effective_bpp) = match extract_terrain_height_samples(raw) {
        Ok(result) => result,
        Err(error) => {
            let legacy_min = LEGACY_OZB_TERRAIN_HEADER + (LEGACY_OZB_TERRAIN_PIXELS - 256);
            if raw.len() >= legacy_min {
                (raw_terrain_height_samples(raw), 8)
            } else {
                return Err(TerrainHeightError::ParseSource {
                    path: path.to_path_buf(),
                    message: error,
                });
            }
        }
    };

    if samples.len() != TERRAIN_TILE_COUNT {
        return Err(TerrainHeightError::ParseSource {
            path: path.to_path_buf(),
            message: format!(
                "invalid terrain height sample count: {} (expected {})",
                samples.len(),
                TERRAIN_TILE_COUNT
            ),
        });
    }

    let mut height_multiplier = f64::from(height_multiplier_for_path(path));
    let mut height_offset = 0.0;
    if effective_bpp == 24 {
        height_multiplier = 1.0;
        height_offset = -500.0;
    }

    let unique_values = samples.iter().copied().collect::<BTreeSet<_>>().len();
    let source_min = samples.iter().copied().min().unwrap_or(0);
    let source_max = samples.iter().copied().max().unwrap_or(0);

    let mut normalized_sample_min = f64::INFINITY;
    let mut normalized_sample_max = f64::NEG_INFINITY;
    let heights = samples
        .chunks(terrain_size())
        .map(|row| {
            row.iter()
                .map(|sample| {
                    let value = f64::from(*sample);
                    normalized_sample_min = normalized_sample_min.min(value);
                    normalized_sample_max = normalized_sample_max.max(value);
                    value
                })
                .collect::<Vec<f64>>()
        })
        .collect::<Vec<_>>();

    if !normalized_sample_min.is_finite() {
        normalized_sample_min = 0.0;
        normalized_sample_max = 0.0;
    }

    Ok(TerrainHeightJson {
        width: terrain_size() as u32,
        height: terrain_size() as u32,
        heights,
        metadata: TerrainHeightMetadata {
            source: path.file_name().unwrap_or("terrain_height.ozb").to_string(),
            source_bits_per_pixel: effective_bpp,
            row_order: "legacy_client".to_string(),
            height_multiplier,
            height_offset,
            legacy_terrain_scale: f64::from(terrain_scale()),
            source_unique_values: unique_values,
            source_min,
            source_max,
            normalized_sample_min,
            normalized_sample_max,
        },
    })
}

fn extract_terrain_height_samples(payload: &[u8]) -> Result<(Vec<u32>, u8), String> {
    if payload.len() < 54 {
        return Err(format!(
            "Terrain payload too small for BMP header ({} < 54).",
            payload.len()
        ));
    }

    if !payload.starts_with(b"BM") {
        return Err("TerrainHeight payload does not start with BMP signature 'BM'.".to_string());
    }

    let width = i32::from_le_bytes([payload[18], payload[19], payload[20], payload[21]]);
    let height = i32::from_le_bytes([payload[22], payload[23], payload[24], payload[25]]);
    let bits_per_pixel = u16::from_le_bytes([payload[28], payload[29]]);
    let compression = u32::from_le_bytes([payload[30], payload[31], payload[32], payload[33]]);

    let width = width.unsigned_abs();
    let height = height.unsigned_abs();
    if width < TERRAIN_SIZE_U32 || height < TERRAIN_SIZE_U32 {
        return Err(format!(
            "TerrainHeight bitmap too small ({}x{}); expected at least {}x{}.",
            width,
            height,
            terrain_size(),
            terrain_size()
        ));
    }

    if compression != 0 {
        return Err(format!(
            "unsupported TerrainHeight BMP compression: {} (expected BI_RGB/0)",
            compression
        ));
    }

    if bits_per_pixel <= 8 {
        let legacy_required = LEGACY_OZB_TERRAIN_HEADER + LEGACY_OZB_TERRAIN_PIXELS;
        if payload.len() < legacy_required {
            return Err(format!(
                "TerrainHeight payload shorter than expected for legacy 8-bit layout ({} < {}).",
                payload.len(),
                legacy_required
            ));
        }

        let pixels = payload
            [LEGACY_OZB_TERRAIN_HEADER..LEGACY_OZB_TERRAIN_HEADER + LEGACY_OZB_TERRAIN_PIXELS]
            .iter()
            .copied()
            .map(u32::from)
            .collect();
        return Ok((pixels, 8));
    }

    let bytes_per_pixel = usize::from(bits_per_pixel).max(1) / 8;
    if bytes_per_pixel < 3 {
        return Err(format!(
            "unsupported TerrainHeight bit depth: {} (expected 8 or >=24)",
            bits_per_pixel
        ));
    }

    let fixed_offset = 54;
    let required = fixed_offset + LEGACY_OZB_TERRAIN_PIXELS * bytes_per_pixel;
    if required > payload.len() {
        let legacy_required = LEGACY_OZB_TERRAIN_HEADER + LEGACY_OZB_TERRAIN_PIXELS;
        if payload.len() >= legacy_required {
            let pixels = payload
                [LEGACY_OZB_TERRAIN_HEADER..LEGACY_OZB_TERRAIN_HEADER + LEGACY_OZB_TERRAIN_PIXELS]
                .iter()
                .copied()
                .map(u32::from)
                .collect();
            return Ok((pixels, 8));
        }

        return Err(format!(
            "TerrainHeight payload shorter than expected for fixed 24-bit layout ({} < {}).",
            payload.len(),
            required
        ));
    }

    let mut samples = Vec::with_capacity(LEGACY_OZB_TERRAIN_PIXELS);
    for index in 0..LEGACY_OZB_TERRAIN_PIXELS {
        let px_offset = fixed_offset + index * bytes_per_pixel;
        if px_offset + 2 >= payload.len() {
            return Err(format!(
                "TerrainHeight pixel out of bounds at index={} (offset {} >= {}).",
                index,
                px_offset,
                payload.len()
            ));
        }

        let b_val = payload[px_offset];
        let g_val = payload[px_offset + 1];
        let r_val = payload[px_offset + 2];
        samples.push(u32::from(r_val) + u32::from(g_val) * 256 + u32::from(b_val) * 65_536);
    }

    Ok((samples, 24))
}

fn raw_terrain_height_samples(payload: &[u8]) -> Vec<u32> {
    let available = payload
        .len()
        .saturating_sub(LEGACY_OZB_TERRAIN_HEADER)
        .min(LEGACY_OZB_TERRAIN_PIXELS);
    let start = LEGACY_OZB_TERRAIN_HEADER.min(payload.len());
    let end = start + available;

    let mut samples = payload[start..end]
        .iter()
        .copied()
        .map(u32::from)
        .collect::<Vec<_>>();
    samples.resize(LEGACY_OZB_TERRAIN_PIXELS, 0);
    samples
}

const TERRAIN_SIZE_U32: u32 = 256;

#[cfg(test)]
mod tests {
    use std::fs;
    use std::path::Path;

    use camino::Utf8PathBuf;

    use super::{convert_terrain_height_file, decode_terrain_height_bytes, TerrainHeightJson};
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
            "mu_asset_pipeline_terrain_height_{label}_{}_{}",
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

    #[test]
    fn converts_terrain_height_sidecar() {
        let source_root = repo_root().join("src/bin/Data");
        let source = source_path("TerrainHeight.OZB");
        let output_root = temp_root("terrain_height");

        let entry = convert_terrain_height_file(&source_root, &source, &output_root).unwrap();
        let output_path = output_root.join(entry.converted_path.clone());
        let output_bytes = fs::read(&output_path).unwrap();
        let json: TerrainHeightJson = serde_json::from_slice(&output_bytes).unwrap();

        assert_eq!(entry.kind, "terrain-height");
        assert_eq!(
            entry.converted_path,
            terrain_output_relative_path(
                source.strip_prefix(&source_root).unwrap(),
                "terrain_height.json",
            )
            .to_string()
        );
        assert_eq!(entry.source_hash, sha256_hex(&fs::read(&source).unwrap()));
        assert_eq!(entry.converted_hash, sha256_hex(&output_bytes));
        assert_eq!(json.width, 256);
        assert_eq!(json.height, 256);
        assert_eq!(json.metadata.source, "TerrainHeight.OZB");
        assert_eq!(json.metadata.source_bits_per_pixel, 8);
        assert_eq!(json.metadata.row_order, "legacy_client");
        assert_eq!(json.metadata.height_multiplier, 1.5);
        assert_eq!(json.metadata.height_offset, 0.0);
        assert_eq!(json.metadata.legacy_terrain_scale, 100.0);
        assert_eq!(json.metadata.source_unique_values, 255);
        assert_eq!(json.metadata.source_min, 0);
        assert_eq!(json.metadata.source_max, 255);
        assert_eq!(json.metadata.normalized_sample_min, 0.0);
        assert_eq!(json.metadata.normalized_sample_max, 255.0);
        assert_eq!(json.heights.len(), 256);
        assert_eq!(json.heights[0].len(), 256);
        assert_eq!(
            &json.heights[0][..16],
            &[
                255.0, 0.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0,
                15.0, 15.0,
            ]
        );
    }

    #[test]
    fn decode_terrain_height_bytes_handles_legacy_padding() {
        let source = source_path("TerrainHeight.OZB");
        let source_bytes = fs::read(&source).unwrap();
        let decoded = decode_terrain_height_bytes(&source, &source_bytes).unwrap();

        assert_eq!(decoded.metadata.source_bits_per_pixel, 8);
        assert_eq!(decoded.metadata.source_min, 0);
        assert_eq!(decoded.metadata.source_max, 255);
    }
}
