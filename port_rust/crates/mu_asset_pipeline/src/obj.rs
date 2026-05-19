use std::collections::BTreeMap;
use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use serde_json::{json, Value};
use thiserror::Error;

use crate::models::swizzle_mu_to_gltf_vec3;

const SCENE_OBJECT_ENTRY_SIZES: &[usize] = &[30, 32, 33, 45, 46, 54];

#[derive(Debug, Error)]
pub enum SceneObjectConversionError {
    #[error("failed to read scene-object source {path}: {source}")]
    Read {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
    #[error("failed to parse scene-object source {path}: {message}")]
    Parse { path: Utf8PathBuf, message: String },
    #[error("failed to write scene-object output {path}: {source}")]
    Write {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
}

#[derive(Debug, Clone)]
pub struct SceneObjectDocument {
    pub source_name: String,
    pub version: u8,
    pub map_number: u32,
    pub entry_size: usize,
    pub decode_name: String,
    pub layout_name: String,
    pub decode_score: f32,
    pub objects: Vec<SceneObject>,
}

#[derive(Debug, Clone)]
pub struct SceneObject {
    pub id: String,
    pub object_type: i16,
    pub model: String,
    pub position: [f32; 3],
    pub rotation: [f32; 3],
    pub scale: [f32; 3],
    pub properties: BTreeMap<String, Value>,
}

pub fn parse_scene_objects_file(
    path: &Utf8Path,
    output_root: &Utf8Path,
) -> Result<SceneObjectDocument, SceneObjectConversionError> {
    let raw = fs::read(path).map_err(|source| SceneObjectConversionError::Read {
        path: path.to_path_buf(),
        source,
    })?;
    parse_scene_objects_bytes(path, output_root, &raw)
}

pub(crate) fn parse_scene_objects_bytes(
    path: &Utf8Path,
    output_root: &Utf8Path,
    raw: &[u8],
) -> Result<SceneObjectDocument, SceneObjectConversionError> {
    let world_number_hint = infer_world_number(path);
    let (payload, decode_name) = decode_scene_object_payload(raw);
    let candidate = choose_layout(path, &payload, world_number_hint).ok_or_else(|| {
        SceneObjectConversionError::Parse {
            path: path.to_path_buf(),
            message: "no valid scene-object layout candidate".to_string(),
        }
    })?;

    if candidate.required > payload.len() {
        return Err(SceneObjectConversionError::Parse {
            path: path.to_path_buf(),
            message: "scene-object payload shorter than declared object count".to_string(),
        });
    }

    let mut objects = Vec::with_capacity(candidate.count);
    let mut offset = candidate.entry_offset;
    for index in 0..candidate.count {
        let object_type =
            read_i16_le(&payload, offset).map_err(|_| SceneObjectConversionError::Parse {
                path: path.to_path_buf(),
                message: "scene-object entry truncated".to_string(),
            })?;
        let px =
            read_f32_le(&payload, offset + 2).map_err(|_| SceneObjectConversionError::Parse {
                path: path.to_path_buf(),
                message: "scene-object entry truncated".to_string(),
            })?;
        let py =
            read_f32_le(&payload, offset + 6).map_err(|_| SceneObjectConversionError::Parse {
                path: path.to_path_buf(),
                message: "scene-object entry truncated".to_string(),
            })?;
        let pz =
            read_f32_le(&payload, offset + 10).map_err(|_| SceneObjectConversionError::Parse {
                path: path.to_path_buf(),
                message: "scene-object entry truncated".to_string(),
            })?;
        let ax =
            read_f32_le(&payload, offset + 14).map_err(|_| SceneObjectConversionError::Parse {
                path: path.to_path_buf(),
                message: "scene-object entry truncated".to_string(),
            })?;
        let ay =
            read_f32_le(&payload, offset + 18).map_err(|_| SceneObjectConversionError::Parse {
                path: path.to_path_buf(),
                message: "scene-object entry truncated".to_string(),
            })?;
        let az =
            read_f32_le(&payload, offset + 22).map_err(|_| SceneObjectConversionError::Parse {
                path: path.to_path_buf(),
                message: "scene-object entry truncated".to_string(),
            })?;
        let scale =
            read_f32_le(&payload, offset + 26).map_err(|_| SceneObjectConversionError::Parse {
                path: path.to_path_buf(),
                message: "scene-object entry truncated".to_string(),
            })?;

        let model_index = (object_type as i32 + 1).max(1) as u32;
        let model = resolve_scene_object_model_path(
            output_root,
            candidate.map_number,
            object_type,
            model_index,
        );

        let mut properties = BTreeMap::new();
        properties.insert("decode_name".to_string(), json!(decode_name));
        properties.insert("layout_name".to_string(), json!(candidate.layout_name));

        objects.push(SceneObject {
            id: format!("obj_{index:05}"),
            object_type,
            model,
            position: swizzle_mu_to_gltf_vec3([px, py, pz]),
            rotation: [ax, ay, az],
            scale: [scale, scale, scale],
            properties,
        });

        offset += candidate.entry_size;
    }

    Ok(SceneObjectDocument {
        source_name: path.file_name().unwrap_or("scene_objects.obj").to_string(),
        version: candidate.version,
        map_number: candidate.map_number,
        entry_size: candidate.entry_size,
        decode_name,
        layout_name: candidate.layout_name,
        decode_score: candidate.score,
        objects,
    })
}

impl SceneObjectDocument {
    pub fn to_json_bytes(&self) -> Result<Vec<u8>, SceneObjectConversionError> {
        let mut objects = Vec::with_capacity(self.objects.len());
        for object in &self.objects {
            let mut entry = json!({
                "id": object.id,
                "type": object.object_type,
                "model": object.model,
                "position": object.position,
                "rotation": object.rotation,
                "scale": object.scale,
            });
            if !object.properties.is_empty() {
                let properties = object
                    .properties
                    .iter()
                    .map(|(key, value)| (key.clone(), value.clone()))
                    .collect::<serde_json::Map<String, Value>>();
                entry["properties"] = Value::Object(properties);
            }
            objects.push(entry);
        }

        let payload = json!({
            "objects": objects,
            "metadata": {
                "source": self.source_name,
                "version": self.version,
                "map_number": self.map_number,
                "object_count": self.objects.len(),
                "entry_size": self.entry_size,
                "decode_name": self.decode_name,
                "layout_name": self.layout_name,
                "decode_score": self.decode_score,
                "rotation_encoding": "mu_angles_degrees",
                "rotation_convention": "mu_anglematrix_zyx_degrees",
                "rotation_yaw_offset_degrees": 0.0,
            },
        });

        serde_json::to_vec_pretty(&payload).map_err(|source| SceneObjectConversionError::Parse {
            path: Utf8PathBuf::from(self.source_name.clone()),
            message: source.to_string(),
        })
    }
}

struct LayoutCandidate {
    version: u8,
    map_number: u32,
    entry_offset: usize,
    entry_size: usize,
    count: usize,
    required: usize,
    layout_name: String,
    score: f32,
}

fn choose_layout(
    _path: &Utf8Path,
    payload: &[u8],
    world_hint: Option<u32>,
) -> Option<LayoutCandidate> {
    let mut candidates = Vec::new();

    if payload.len() >= 4 {
        let version = payload[0];
        let map_number = world_hint.unwrap_or(payload[1] as u32);
        let count = read_i16_le(payload, 2).ok()?;
        if count >= 0 {
            candidates.push(LayoutCandidate {
                version,
                map_number,
                entry_offset: 4,
                entry_size: 30,
                count: count as usize,
                required: 4 + count as usize * 30,
                layout_name: "version_map_count".to_string(),
                score: scene_object_score(payload, 4, count as usize, 30, world_hint, map_number),
            });
        }
    }

    if payload.len() >= 3 {
        let version = payload[0];
        let map_number = world_hint.unwrap_or(0);
        let count = read_i16_le(payload, 1).ok()?;
        if count >= 0 {
            candidates.push(LayoutCandidate {
                version,
                map_number,
                entry_offset: 3,
                entry_size: 30,
                count: count as usize,
                required: 3 + count as usize * 30,
                layout_name: "version_count".to_string(),
                score: scene_object_score(payload, 3, count as usize, 30, world_hint, map_number),
            });
        }
    }

    for entry_size in SCENE_OBJECT_ENTRY_SIZES {
        if payload.len() <= 4 {
            continue;
        }
        let body = payload.len() - 4;
        if body % entry_size != 0 {
            continue;
        }
        let version = payload[0];
        let map_number = world_hint.unwrap_or(payload[1] as u32);
        let count = body / entry_size;
        candidates.push(LayoutCandidate {
            version,
            map_number,
            entry_offset: 4,
            entry_size: *entry_size,
            count,
            required: 4 + count * entry_size,
            layout_name: "size_derived".to_string(),
            score: scene_object_score(payload, 4, count, *entry_size, world_hint, map_number),
        });
    }

    candidates
        .into_iter()
        .filter(|candidate| candidate.count > 0 || candidate.entry_size == 30)
        .max_by(|left, right| left.score.total_cmp(&right.score))
}

fn scene_object_score(
    payload: &[u8],
    offset: usize,
    count: usize,
    entry_size: usize,
    world_hint: Option<u32>,
    map_number: u32,
) -> f32 {
    if count == 0 || entry_size < 30 || offset + count * entry_size > payload.len() {
        return 0.0;
    }

    let sample_count = count.min(64);
    let mut sane_entries = 0;

    for index in 0..sample_count {
        let entry_offset = offset + index * entry_size;
        let object_type = read_i16_le(payload, entry_offset).unwrap_or(i16::MAX);
        let px = read_f32_le(payload, entry_offset + 2).unwrap_or(f32::NAN);
        let py = read_f32_le(payload, entry_offset + 6).unwrap_or(f32::NAN);
        let pz = read_f32_le(payload, entry_offset + 10).unwrap_or(f32::NAN);
        let ax = read_f32_le(payload, entry_offset + 14).unwrap_or(f32::NAN);
        let ay = read_f32_le(payload, entry_offset + 18).unwrap_or(f32::NAN);
        let az = read_f32_le(payload, entry_offset + 22).unwrap_or(f32::NAN);
        let scale = read_f32_le(payload, entry_offset + 26).unwrap_or(f32::NAN);

        if ![px, py, pz, ax, ay, az, scale]
            .iter()
            .all(|value| value.is_finite())
        {
            continue;
        }
        if object_type < -1 || object_type > 4096 {
            continue;
        }
        if px.abs() > 250_000.0 || py.abs() > 250_000.0 || pz.abs() > 250_000.0 {
            continue;
        }
        if ax.abs() > 36_000.0 || ay.abs() > 36_000.0 || az.abs() > 36_000.0 {
            continue;
        }
        if !(0.001..=128.0).contains(&scale) {
            continue;
        }

        sane_entries += 1;
    }

    let mut score = sane_entries as f32 / sample_count as f32;
    if offset + count * entry_size == payload.len() {
        score += 0.05;
    }
    if world_hint.is_some() && world_hint == Some(map_number) {
        score += 0.20;
    }
    score
}

fn decode_scene_object_payload(raw: &[u8]) -> (Vec<u8>, String) {
    (raw.to_vec(), "raw".to_string())
}

fn resolve_scene_object_model_path(
    output_root: &Utf8Path,
    world_number: u32,
    object_type: i16,
    model_index: u32,
) -> String {
    let directory = format!("object_{}", world_number.max(1));
    let file_name = format!("object_{model_index:02}.glb");
    let model_path = Utf8PathBuf::from(directory).join(file_name);
    let disk_path = output_root.join(&model_path);
    if disk_path.exists() {
        return format!("data/{}", model_path);
    }

    let _ = object_type;
    format!("data/{}", model_path)
}

fn read_i16_le(bytes: &[u8], offset: usize) -> Result<i16, ()> {
    if offset + 2 > bytes.len() {
        return Err(());
    }
    Ok(i16::from_le_bytes([bytes[offset], bytes[offset + 1]]))
}

fn read_f32_le(bytes: &[u8], offset: usize) -> Result<f32, ()> {
    if offset + 4 > bytes.len() {
        return Err(());
    }
    Ok(f32::from_le_bytes([
        bytes[offset],
        bytes[offset + 1],
        bytes[offset + 2],
        bytes[offset + 3],
    ]))
}

fn infer_world_number(path: &Utf8Path) -> Option<u32> {
    for component in path.components() {
        let segment = component.as_str().to_ascii_lowercase();
        if let Some(value) = parse_indexed_segment(&segment, "world") {
            return Some(value);
        }
    }
    None
}

fn parse_indexed_segment(segment: &str, prefix: &str) -> Option<u32> {
    if !segment.starts_with(prefix) {
        return None;
    }

    let mut rest = &segment[prefix.len()..];
    if rest.starts_with('_') {
        rest = &rest[1..];
    }
    if rest.is_empty() || !rest.chars().all(|ch| ch.is_ascii_digit()) {
        return None;
    }
    rest.parse::<u32>().ok()
}

#[cfg(test)]
mod tests {
    use super::{parse_indexed_segment, resolve_scene_object_model_path, scene_object_score};
    use camino::Utf8Path;

    #[test]
    fn scores_simple_scene_object_payloads() {
        let mut payload = Vec::new();
        payload.extend_from_slice(&[1, 1, 3, 0]);
        for index in 0..3 {
            let object_type = index as i16;
            payload.extend_from_slice(&object_type.to_le_bytes());
            for component in [1.0f32, 2.0, 3.0, 4.0, 5.0, 6.0, 1.0] {
                payload.extend_from_slice(&component.to_le_bytes());
            }
        }

        let score = scene_object_score(&payload, 4, 3, 30, Some(1), 1);
        assert!(score > 0.8);
    }

    #[test]
    fn resolves_scene_object_model_path() {
        let path = resolve_scene_object_model_path(Utf8Path::new("/tmp/out"), 1, 0, 1);
        assert_eq!(path, "data/object_1/object_01.glb");
    }

    #[test]
    fn parses_indexed_segments() {
        assert_eq!(parse_indexed_segment("world_12", "world"), Some(12));
        assert_eq!(parse_indexed_segment("object01", "object"), Some(1));
    }
}
