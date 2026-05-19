use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use mu_assets::AssetManifestEntry;
use serde::Serialize;

use crate::att::TerrainAttributeError;
use crate::map::TerrainMapError;
use crate::ozb::TerrainHeightError;

const TERRAIN_STEM_PREFIX: &str = "terrain";
const ENCRYPTED_TERRAIN_STEM_PREFIX: &str = "encterrain";
const ENCRYPTED_TERRAIN_UNDERSCORE_STEM_PREFIX: &str = "enc_terrain";
const WORLD_DIR_PREFIX: &str = "world";
const TERRAIN_SIZE: usize = 256;
const TERRAIN_SCALE: f32 = 100.0;
const DEFAULT_WORLD_HEIGHT_MULTIPLIER: f32 = 1.5;
const LOGIN_WORLD_HEIGHT_MULTIPLIER: f32 = 3.0;
const MAP_XOR_KEY: [u8; 16] = [
    0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27, 0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2,
];
const BUX_KEY: [u8; 3] = [0xFC, 0xCF, 0xAB];
const MAP_KEY_SEED: u8 = 0x5E;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TerrainConversionReport {
    pub source_root: Utf8PathBuf,
    pub output_root: Utf8PathBuf,
    pub converted: Vec<AssetManifestEntry>,
    pub rejected: Vec<TerrainRejection>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TerrainRejection {
    pub path: Utf8PathBuf,
    pub reason: TerrainRejectionReason,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TerrainRejectionReason {
    UnsupportedExtension,
    SourceOutsideRoot,
    ReadFailed,
    ParseFailed,
    WriteFailed,
}

impl TerrainRejectionReason {
    pub const fn as_str(self) -> &'static str {
        match self {
            Self::UnsupportedExtension => "unsupported-extension",
            Self::SourceOutsideRoot => "source-outside-root",
            Self::ReadFailed => "read-failed",
            Self::ParseFailed => "parse-failed",
            Self::WriteFailed => "write-failed",
        }
    }
}

impl From<&TerrainMapError> for TerrainRejectionReason {
    fn from(error: &TerrainMapError) -> Self {
        match error {
            TerrainMapError::UnsupportedExtension { .. } => Self::UnsupportedExtension,
            TerrainMapError::SourceOutsideRoot { .. } => Self::SourceOutsideRoot,
            TerrainMapError::ReadSource { .. } => Self::ReadFailed,
            TerrainMapError::ParseSource { .. } => Self::ParseFailed,
            TerrainMapError::WriteOutput { .. } => Self::WriteFailed,
        }
    }
}

impl From<&TerrainAttributeError> for TerrainRejectionReason {
    fn from(error: &TerrainAttributeError) -> Self {
        match error {
            TerrainAttributeError::UnsupportedExtension { .. } => Self::UnsupportedExtension,
            TerrainAttributeError::SourceOutsideRoot { .. } => Self::SourceOutsideRoot,
            TerrainAttributeError::ReadSource { .. } => Self::ReadFailed,
            TerrainAttributeError::ParseSource { .. } => Self::ParseFailed,
            TerrainAttributeError::WriteOutput { .. } => Self::WriteFailed,
        }
    }
}

impl From<&TerrainHeightError> for TerrainRejectionReason {
    fn from(error: &TerrainHeightError) -> Self {
        match error {
            TerrainHeightError::UnsupportedExtension { .. } => Self::UnsupportedExtension,
            TerrainHeightError::SourceOutsideRoot { .. } => Self::SourceOutsideRoot,
            TerrainHeightError::ReadSource { .. } => Self::ReadFailed,
            TerrainHeightError::ParseSource { .. } => Self::ParseFailed,
            TerrainHeightError::WriteOutput { .. } => Self::WriteFailed,
        }
    }
}

pub fn convert_terrain_assets(
    source_root: &Utf8Path,
    output_root: &Utf8Path,
) -> io::Result<TerrainConversionReport> {
    let mut report = TerrainConversionReport {
        source_root: source_root.to_path_buf(),
        output_root: output_root.to_path_buf(),
        converted: Vec::new(),
        rejected: Vec::new(),
    };

    visit_terrain_tree(source_root, source_root, output_root, &mut report)?;
    report
        .converted
        .sort_by(|left, right| left.source_path.cmp(&right.source_path));
    report
        .rejected
        .sort_by(|left, right| left.path.cmp(&right.path));
    Ok(report)
}

pub(crate) fn visit_terrain_tree(
    source_root: &Utf8Path,
    current_root: &Utf8Path,
    output_root: &Utf8Path,
    report: &mut TerrainConversionReport,
) -> io::Result<()> {
    let mut entries = Vec::new();
    for entry in fs::read_dir(current_root)? {
        let entry = entry?;
        entries.push(entry.path());
    }
    entries.sort();

    for path in entries {
        let path = Utf8PathBuf::from_path_buf(path).map_err(|path| {
            io::Error::new(
                io::ErrorKind::InvalidData,
                format!("non-utf8 terrain path: {}", path.to_string_lossy()),
            )
        })?;

        if path.is_dir() {
            visit_terrain_tree(source_root, &path, output_root, report)?;
            continue;
        }

        if !path.is_file() {
            continue;
        }

        match convert_terrain_path(source_root, &path, output_root) {
            Ok(Some(entry)) => report.converted.push(entry),
            Ok(None) => {}
            Err(error) => report.rejected.push(TerrainRejection {
                path: path.to_path_buf(),
                reason: error.reason(),
            }),
        }
    }

    Ok(())
}

pub(crate) fn convert_terrain_path(
    source_root: &Utf8Path,
    source_path: &Utf8Path,
    output_root: &Utf8Path,
) -> Result<Option<AssetManifestEntry>, TerrainFileError> {
    if is_terrain_height_asset(source_path) {
        return crate::ozb::convert_terrain_height_file(source_root, source_path, output_root)
            .map(Some)
            .map_err(TerrainFileError::from);
    }

    if is_terrain_map_asset(source_path) {
        return crate::map::convert_terrain_map_file(source_root, source_path, output_root)
            .map(Some)
            .map_err(TerrainFileError::from);
    }

    if is_terrain_attribute_asset(source_path) {
        return crate::att::convert_terrain_attribute_file(source_root, source_path, output_root)
            .map(Some)
            .map_err(TerrainFileError::from);
    }

    Ok(None)
}

#[derive(Debug)]
pub(crate) enum TerrainFileError {
    Map(TerrainMapError),
    Attribute(TerrainAttributeError),
    Height(TerrainHeightError),
}

impl TerrainFileError {
    fn reason(&self) -> TerrainRejectionReason {
        match self {
            Self::Map(error) => TerrainRejectionReason::from(error),
            Self::Attribute(error) => TerrainRejectionReason::from(error),
            Self::Height(error) => TerrainRejectionReason::from(error),
        }
    }
}

impl From<TerrainMapError> for TerrainFileError {
    fn from(error: TerrainMapError) -> Self {
        Self::Map(error)
    }
}

impl From<TerrainAttributeError> for TerrainFileError {
    fn from(error: TerrainAttributeError) -> Self {
        Self::Attribute(error)
    }
}

impl From<TerrainHeightError> for TerrainFileError {
    fn from(error: TerrainHeightError) -> Self {
        Self::Height(error)
    }
}

pub(crate) fn write_json_file<T: Serialize>(
    path: &Utf8Path,
    value: &T,
) -> Result<Vec<u8>, io::Error> {
    let payload = serde_json::to_vec_pretty(value)
        .map_err(|error| io::Error::new(io::ErrorKind::InvalidData, error))?;

    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)?;
    }
    fs::write(path, &payload)?;
    Ok(payload)
}

pub(crate) fn terrain_output_relative_path(
    relative_path: &Utf8Path,
    output_file_name: &str,
) -> Utf8PathBuf {
    if let Some(world_dir) = canonical_world_dir_from_path(relative_path) {
        return world_dir.join(output_file_name);
    }

    let parent = relative_path.parent().unwrap_or_else(|| Utf8Path::new(""));
    if parent.as_str().is_empty() {
        Utf8PathBuf::from(output_file_name)
    } else {
        parent.join(output_file_name)
    }
}

pub(crate) fn canonical_world_dir_from_path(relative_path: &Utf8Path) -> Option<Utf8PathBuf> {
    for component in relative_path.components() {
        let segment = component.as_str();
        if let Some(world_number) = parse_world_number(segment) {
            return Some(Utf8PathBuf::from(format!("world_{world_number}")));
        }
    }

    None
}

pub(crate) fn parse_world_number(segment: &str) -> Option<u32> {
    let lower = segment.to_ascii_lowercase();
    let suffix = lower
        .strip_prefix("world_")
        .or_else(|| lower.strip_prefix(WORLD_DIR_PREFIX))?;
    suffix.parse().ok()
}

pub(crate) fn height_multiplier_for_path(path: &Utf8Path) -> f32 {
    let world55 = path
        .components()
        .any(|component| component.as_str().eq_ignore_ascii_case("world55"));
    if world55 {
        LOGIN_WORLD_HEIGHT_MULTIPLIER
    } else {
        DEFAULT_WORLD_HEIGHT_MULTIPLIER
    }
}

pub(crate) fn is_terrain_height_asset(path: &Utf8Path) -> bool {
    normalized_extension(path) == "ozb"
        && matches!(
            normalized_stem(path).as_str(),
            "terrainheight" | "terrain_height"
        )
}

pub(crate) fn is_terrain_map_asset(path: &Utf8Path) -> bool {
    normalized_extension(path) == "map" && is_terrain_stem(&normalized_stem(path))
}

pub(crate) fn is_terrain_attribute_asset(path: &Utf8Path) -> bool {
    normalized_extension(path) == "att" && is_terrain_stem(&normalized_stem(path))
}

pub(crate) fn is_terrain_stem(stem: &str) -> bool {
    let lower = stem.to_ascii_lowercase();
    lower.starts_with(TERRAIN_STEM_PREFIX)
        || lower.starts_with(ENCRYPTED_TERRAIN_STEM_PREFIX)
        || lower.starts_with(ENCRYPTED_TERRAIN_UNDERSCORE_STEM_PREFIX)
}

pub(crate) fn is_encrypted_terrain_stem(stem: &str) -> bool {
    let lower = stem.to_ascii_lowercase();
    lower.starts_with(ENCRYPTED_TERRAIN_STEM_PREFIX)
        || lower.starts_with(ENCRYPTED_TERRAIN_UNDERSCORE_STEM_PREFIX)
}

pub(crate) fn normalized_extension(path: &Utf8Path) -> String {
    path.extension()
        .map(str::to_ascii_lowercase)
        .unwrap_or_else(|| "<none>".to_owned())
}

pub(crate) fn normalized_stem(path: &Utf8Path) -> String {
    path.file_stem()
        .map(str::to_ascii_lowercase)
        .unwrap_or_else(|| "<none>".to_owned())
}

pub(crate) fn map_file_decrypt(data: &[u8]) -> Vec<u8> {
    let mut out = vec![0u8; data.len()];
    let mut map_key = MAP_KEY_SEED;

    for (index, value) in data.iter().copied().enumerate() {
        out[index] = (value ^ MAP_XOR_KEY[index % MAP_XOR_KEY.len()]).wrapping_sub(map_key);
        map_key = value.wrapping_add(0x3D);
    }

    out
}

pub(crate) fn apply_bux_convert(data: &[u8]) -> Vec<u8> {
    let mut out = data.to_vec();
    for (index, value) in out.iter_mut().enumerate() {
        *value ^= BUX_KEY[index % BUX_KEY.len()];
    }
    out
}

pub(crate) fn has_modulus_magic(data: &[u8]) -> bool {
    data.len() >= 4 && data[3] == 1 && (data.starts_with(b"ATT") || data.starts_with(b"MAP"))
}

pub(crate) fn rows_from_u8(values: &[u8]) -> Vec<Vec<u8>> {
    values
        .chunks(TERRAIN_SIZE)
        .map(|row| row.to_vec())
        .collect()
}

pub(crate) fn rows_from_u16(values: &[u16]) -> Vec<Vec<u16>> {
    values
        .chunks(TERRAIN_SIZE)
        .map(|row| row.to_vec())
        .collect()
}

pub(crate) fn terrain_size() -> usize {
    TERRAIN_SIZE
}

pub(crate) fn terrain_scale() -> f32 {
    TERRAIN_SCALE
}
