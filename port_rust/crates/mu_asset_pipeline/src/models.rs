use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use mu_assets::{sha256_hex, AssetManifestEntry};
use serde_json::{json, Value};
use thiserror::Error;

const MODEL_KIND: &str = "model";
const GLB_EXTENSION: &str = "glb";
const SCENE_OBJECT_FILE_NAME: &str = "scene_objects.json";
const GLTF_GENERATOR: &str = "mu-rust mu_asset_pipeline";
const WORLD_DIR_PREFIX: &str = "world";
const OBJECT_DIR_PREFIX: &str = "object";
const OBJECT_MODEL_PREFIX: &str = "object";
const NON_MODEL_BMD_STEMS: &[&str] = &[
    "item",
    "minimap",
    "itemsetting",
    "petdata",
    "gate",
    "movereq",
    "dialog",
    "filter",
    "quest",
    "npcname",
    "servercode",
    "itemaddoption",
    "socketitem",
    "masterskill",
    "masterpassive",
    "character",
    "skill",
    "chaosbox",
    "mixlist",
];

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ModelConversionReport {
    pub source_root: Utf8PathBuf,
    pub output_root: Utf8PathBuf,
    pub converted: Vec<AssetManifestEntry>,
    pub rejected: Vec<ModelRejection>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ModelRejection {
    pub path: Utf8PathBuf,
    pub reason: ModelRejectionReason,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ModelRejectionReason {
    UnsupportedExtension,
    SourceOutsideRoot,
    ReadFailed,
    ParseFailed,
    NoGeometry,
    UnsupportedVersion,
    WriteFailed,
}

impl ModelRejectionReason {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::UnsupportedExtension => "unsupported-extension",
            Self::SourceOutsideRoot => "source-outside-root",
            Self::ReadFailed => "read-failed",
            Self::ParseFailed => "parse-failed",
            Self::NoGeometry => "no-geometry",
            Self::UnsupportedVersion => "unsupported-version",
            Self::WriteFailed => "write-failed",
        }
    }
}

#[derive(Debug, Error)]
pub enum ModelConversionError {
    #[error("unsupported model extension at {path}: {extension}")]
    UnsupportedExtension {
        path: Utf8PathBuf,
        extension: String,
    },
    #[error("model source path escapes the source root: {path}")]
    SourceOutsideRoot { path: Utf8PathBuf },
    #[error("failed to read model source {path}: {source}")]
    ReadSource {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
    #[error("failed to parse model {path}: {message}")]
    ParseSource { path: Utf8PathBuf, message: String },
    #[error("model has no geometry at {path}")]
    NoGeometry { path: Utf8PathBuf },
    #[error("unsupported BMD version at {path}: {version:#04x}")]
    UnsupportedVersion { path: Utf8PathBuf, version: u8 },
    #[error("failed to write model {path}: {source}")]
    WriteOutput {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
}

impl From<&ModelConversionError> for ModelRejectionReason {
    fn from(error: &ModelConversionError) -> Self {
        match error {
            ModelConversionError::UnsupportedExtension { .. } => Self::UnsupportedExtension,
            ModelConversionError::SourceOutsideRoot { .. } => Self::SourceOutsideRoot,
            ModelConversionError::ReadSource { .. } => Self::ReadFailed,
            ModelConversionError::ParseSource { .. } => Self::ParseFailed,
            ModelConversionError::NoGeometry { .. } => Self::NoGeometry,
            ModelConversionError::UnsupportedVersion { .. } => Self::UnsupportedVersion,
            ModelConversionError::WriteOutput { .. } => Self::WriteFailed,
        }
    }
}

impl From<crate::bmd::BmdParseError> for ModelConversionError {
    fn from(error: crate::bmd::BmdParseError) -> Self {
        match error {
            crate::bmd::BmdParseError::Read { path, source } => {
                ModelConversionError::ReadSource { path, source }
            }
            crate::bmd::BmdParseError::NotBmd { path, magic } => {
                ModelConversionError::ParseSource {
                    path,
                    message: format!("not a BMD file: magic {magic:?}"),
                }
            }
            crate::bmd::BmdParseError::UnsupportedVersion { path, version } => {
                ModelConversionError::UnsupportedVersion { path, version }
            }
            crate::bmd::BmdParseError::TooSmall { path, found } => {
                ModelConversionError::ParseSource {
                    path,
                    message: format!("BMD data too small: {found} bytes"),
                }
            }
            crate::bmd::BmdParseError::InvalidCounts {
                path,
                meshes,
                bones,
                actions,
            } => ModelConversionError::ParseSource {
                path,
                message: format!(
                    "invalid counts: meshes={meshes}, bones={bones}, actions={actions}"
                ),
            },
            crate::bmd::BmdParseError::Truncated { path, section } => {
                ModelConversionError::ParseSource {
                    path,
                    message: format!("truncated while reading {section}"),
                }
            }
            crate::bmd::BmdParseError::NoGeometry { path } => {
                ModelConversionError::NoGeometry { path }
            }
        }
    }
}

impl From<crate::smd::SmdParseError> for ModelConversionError {
    fn from(error: crate::smd::SmdParseError) -> Self {
        match error {
            crate::smd::SmdParseError::Read { path, source } => {
                ModelConversionError::ReadSource { path, source }
            }
            crate::smd::SmdParseError::InvalidToken { path, reason }
            | crate::smd::SmdParseError::InvalidStructure { path, reason } => {
                ModelConversionError::ParseSource {
                    path,
                    message: reason,
                }
            }
            crate::smd::SmdParseError::NoGeometry { path } => {
                ModelConversionError::NoGeometry { path }
            }
        }
    }
}

impl From<crate::obj::SceneObjectConversionError> for ModelConversionError {
    fn from(error: crate::obj::SceneObjectConversionError) -> Self {
        match error {
            crate::obj::SceneObjectConversionError::Read { path, source } => {
                ModelConversionError::ReadSource { path, source }
            }
            crate::obj::SceneObjectConversionError::Parse { path, message } => {
                ModelConversionError::ParseSource { path, message }
            }
            crate::obj::SceneObjectConversionError::Write { path, source } => {
                ModelConversionError::WriteOutput { path, source }
            }
        }
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct BakedModel {
    pub name: String,
    pub bones: Vec<ModelBone>,
    pub meshes: Vec<ModelMesh>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct ModelBone {
    pub name: String,
    pub parent: Option<usize>,
    pub translation: [f32; 3],
    pub rotation: [f32; 4],
}

#[derive(Debug, Clone, PartialEq)]
pub struct ModelMesh {
    pub name: String,
    pub texture_name: Option<String>,
    pub vertices: Vec<ModelVertex>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct ModelVertex {
    pub position: [f32; 3],
    pub normal: [f32; 3],
    pub texcoord: [f32; 2],
}

pub(crate) type Matrix3 = [[f32; 3]; 3];
pub(crate) type Matrix3x4 = [[f32; 4]; 3];

pub fn convert_models(
    source_root: &Utf8Path,
    output_root: &Utf8Path,
) -> Result<ModelConversionReport, ModelConversionError> {
    let mut report = ModelConversionReport {
        source_root: source_root.to_path_buf(),
        output_root: output_root.to_path_buf(),
        converted: Vec::new(),
        rejected: Vec::new(),
    };

    visit_model_tree(source_root, source_root, output_root, &mut report)?;
    report
        .converted
        .sort_by(|left, right| left.source_path.cmp(&right.source_path));
    report
        .rejected
        .sort_by(|left, right| left.path.cmp(&right.path));
    Ok(report)
}

pub fn convert_model_file(
    source_root: &Utf8Path,
    source_path: &Utf8Path,
    output_root: &Utf8Path,
) -> Result<AssetManifestEntry, ModelConversionError> {
    let relative_path = source_path.strip_prefix(source_root).map_err(|_| {
        ModelConversionError::SourceOutsideRoot {
            path: source_path.to_path_buf(),
        }
    })?;

    let extension = normalized_extension(source_path);
    match extension.as_str() {
        "bmd" => {
            let source_bytes =
                fs::read(source_path).map_err(|source| ModelConversionError::ReadSource {
                    path: source_path.to_path_buf(),
                    source,
                })?;
            let model = crate::bmd::parse_bmd_bytes(source_path, &source_bytes)?;
            let output_relative_path = canonicalize_model_output_path(relative_path);
            let output_path = output_root.join(&output_relative_path);
            let converted_bytes = encode_glb(&model);
            write_output_bytes(&output_path, &converted_bytes)?;
            Ok(asset_manifest_entry(
                relative_path,
                &output_relative_path,
                &source_bytes,
                &converted_bytes,
            ))
        }
        "smd" => {
            let source_bytes =
                fs::read(source_path).map_err(|source| ModelConversionError::ReadSource {
                    path: source_path.to_path_buf(),
                    source,
                })?;
            let source_text = std::str::from_utf8(&source_bytes).map_err(|source| {
                ModelConversionError::ParseSource {
                    path: source_path.to_path_buf(),
                    message: source.to_string(),
                }
            })?;
            let model = crate::smd::parse_smd_text(source_path, source_text)?;
            let output_relative_path = canonicalize_model_output_path(relative_path);
            let output_path = output_root.join(&output_relative_path);
            let converted_bytes = encode_glb(&model);
            write_output_bytes(&output_path, &converted_bytes)?;
            Ok(asset_manifest_entry(
                relative_path,
                &output_relative_path,
                &source_bytes,
                &converted_bytes,
            ))
        }
        "obj" => {
            let source_bytes =
                fs::read(source_path).map_err(|source| ModelConversionError::ReadSource {
                    path: source_path.to_path_buf(),
                    source,
                })?;
            let scene_objects =
                crate::obj::parse_scene_objects_bytes(source_path, output_root, &source_bytes)?;
            let output_relative_path = scene_objects_output_path(relative_path);
            let output_path = output_root.join(&output_relative_path);
            let converted_bytes = scene_objects.to_json_bytes()?;
            write_output_bytes(&output_path, &converted_bytes)?;
            Ok(asset_manifest_entry(
                relative_path,
                &output_relative_path,
                &source_bytes,
                &converted_bytes,
            ))
        }
        _ => Err(ModelConversionError::UnsupportedExtension {
            path: source_path.to_path_buf(),
            extension,
        }),
    }
}

pub(crate) fn encode_glb(model: &BakedModel) -> Vec<u8> {
    let mut binary = Vec::new();
    let mut buffer_views = Vec::<Value>::new();
    let mut accessors = Vec::<Value>::new();
    let mut meshes = Vec::<Value>::new();
    let mut nodes = Vec::<Value>::new();
    let mut scene_root_nodes = Vec::<usize>::new();

    let mut bone_children = vec![Vec::<usize>::new(); model.bones.len()];
    for (index, bone) in model.bones.iter().enumerate() {
        if let Some(parent) = bone.parent {
            if parent < model.bones.len() {
                bone_children[parent].push(index);
            }
        }
    }

    for (index, bone) in model.bones.iter().enumerate() {
        let mut node = json!({
            "name": bone.name,
            "translation": bone.translation,
            "rotation": bone.rotation,
        });
        if !bone_children[index].is_empty() {
            node["children"] = json!(bone_children[index]);
        }
        if bone.parent.is_none() {
            scene_root_nodes.push(index);
        }
        nodes.push(node);
    }

    for mesh in &model.meshes {
        if mesh.vertices.is_empty() {
            continue;
        }

        let gltf_mesh = append_mesh(mesh, &mut binary, &mut buffer_views, &mut accessors);
        let mesh_index = meshes.len();
        meshes.push(gltf_mesh);

        let node_index = nodes.len();
        let mut node = json!({
            "mesh": mesh_index,
            "name": mesh.name,
        });
        if !mesh.name.is_empty() {
            node["name"] = json!(mesh.name.clone());
        }
        nodes.push(node);
        scene_root_nodes.push(node_index);
    }

    let root = json!({
        "asset": {
            "version": "2.0",
            "generator": GLTF_GENERATOR,
        },
        "scene": 0,
        "scenes": [
            {
                "nodes": scene_root_nodes,
            }
        ],
        "nodes": nodes,
        "meshes": meshes,
        "buffers": [
            {
                "byteLength": binary.len(),
            }
        ],
        "bufferViews": buffer_views,
        "accessors": accessors,
    });

    build_glb(root, binary)
}

fn append_mesh(
    mesh: &ModelMesh,
    binary: &mut Vec<u8>,
    buffer_views: &mut Vec<Value>,
    accessors: &mut Vec<Value>,
) -> Value {
    let positions: Vec<[f32; 3]> = mesh
        .vertices
        .iter()
        .map(|vertex| swizzle_mu_to_gltf_vec3(vertex.position))
        .collect();
    let normals: Vec<[f32; 3]> = mesh
        .vertices
        .iter()
        .map(|vertex| normalize_vec3(swizzle_mu_to_gltf_vec3(vertex.normal)))
        .collect();
    let texcoords: Vec<[f32; 2]> = mesh.vertices.iter().map(|vertex| vertex.texcoord).collect();
    let indices: Vec<u32> = (0..mesh.vertices.len() as u32).collect();

    let (positions_offset, positions_length, positions_min, positions_max) =
        append_vec3_buffer(binary, &positions);
    buffer_views.push(json!({
        "buffer": 0,
        "byteOffset": positions_offset,
        "byteLength": positions_length,
        "target": 34962,
    }));
    let position_accessor = accessors.len();
    accessors.push(json!({
        "bufferView": buffer_views.len() - 1,
        "componentType": 5126,
        "count": positions.len(),
        "type": "VEC3",
        "min": positions_min,
        "max": positions_max,
    }));

    let (normals_offset, normals_length, _, _) = append_vec3_buffer(binary, &normals);
    buffer_views.push(json!({
        "buffer": 0,
        "byteOffset": normals_offset,
        "byteLength": normals_length,
        "target": 34962,
    }));
    let normal_accessor = accessors.len();
    accessors.push(json!({
        "bufferView": buffer_views.len() - 1,
        "componentType": 5126,
        "count": normals.len(),
        "type": "VEC3",
    }));

    let (texcoords_offset, texcoords_length) = append_vec2_buffer(binary, &texcoords);
    buffer_views.push(json!({
        "buffer": 0,
        "byteOffset": texcoords_offset,
        "byteLength": texcoords_length,
        "target": 34962,
    }));
    let texcoord_accessor = accessors.len();
    accessors.push(json!({
        "bufferView": buffer_views.len() - 1,
        "componentType": 5126,
        "count": texcoords.len(),
        "type": "VEC2",
    }));

    let (indices_offset, indices_length) = append_u32_buffer(binary, &indices);
    buffer_views.push(json!({
        "buffer": 0,
        "byteOffset": indices_offset,
        "byteLength": indices_length,
        "target": 34963,
    }));
    let index_accessor = accessors.len();
    accessors.push(json!({
        "bufferView": buffer_views.len() - 1,
        "componentType": 5125,
        "count": indices.len(),
        "type": "SCALAR",
    }));

    let mut primitive = json!({
        "attributes": {
            "POSITION": position_accessor,
            "NORMAL": normal_accessor,
            "TEXCOORD_0": texcoord_accessor,
        },
        "indices": index_accessor,
        "mode": 4,
    });
    if let Some(texture_name) = &mesh.texture_name {
        primitive["extras"] = json!({
            "texture_name": texture_name,
        });
    }

    json!({
        "name": mesh.name,
        "primitives": [primitive],
    })
}

fn build_glb(json_root: Value, binary: Vec<u8>) -> Vec<u8> {
    let json_bytes = serde_json::to_vec(&json_root).expect("glTF JSON serialization is infallible");
    let json_padded_len = align_to_4(json_bytes.len());
    let bin_padded_len = align_to_4(binary.len());
    let total_length = 12 + 8 + json_padded_len + 8 + bin_padded_len;

    let mut glb = Vec::with_capacity(total_length);
    glb.extend_from_slice(&0x46546C67u32.to_le_bytes());
    glb.extend_from_slice(&2u32.to_le_bytes());
    glb.extend_from_slice(&(total_length as u32).to_le_bytes());

    glb.extend_from_slice(&(json_padded_len as u32).to_le_bytes());
    glb.extend_from_slice(&0x4E4F534Au32.to_le_bytes());
    glb.extend_from_slice(&json_bytes);
    while glb.len() % 4 != 0 {
        glb.push(b' ');
    }

    glb.extend_from_slice(&(bin_padded_len as u32).to_le_bytes());
    glb.extend_from_slice(&0x004E4942u32.to_le_bytes());
    glb.extend_from_slice(&binary);
    while glb.len() % 4 != 0 {
        glb.push(0);
    }

    glb
}

fn append_vec3_buffer(
    binary: &mut Vec<u8>,
    values: &[[f32; 3]],
) -> (usize, usize, [f32; 3], [f32; 3]) {
    let offset = binary.len();
    let mut min = [f32::INFINITY; 3];
    let mut max = [f32::NEG_INFINITY; 3];

    for value in values {
        for index in 0..3 {
            min[index] = min[index].min(value[index]);
            max[index] = max[index].max(value[index]);
            binary.extend_from_slice(&value[index].to_le_bytes());
        }
    }

    let length = binary.len() - offset;
    (offset, length, min, max)
}

fn append_vec2_buffer(binary: &mut Vec<u8>, values: &[[f32; 2]]) -> (usize, usize) {
    let offset = binary.len();
    for value in values {
        for component in value {
            binary.extend_from_slice(&component.to_le_bytes());
        }
    }
    let length = binary.len() - offset;
    (offset, length)
}

fn append_u32_buffer(binary: &mut Vec<u8>, values: &[u32]) -> (usize, usize) {
    let offset = binary.len();
    for value in values {
        binary.extend_from_slice(&value.to_le_bytes());
    }
    let length = binary.len() - offset;
    (offset, length)
}

fn align_to_4(length: usize) -> usize {
    (length + 3) & !3
}

pub(crate) fn swizzle_mu_to_gltf_vec3(value: [f32; 3]) -> [f32; 3] {
    [value[0], value[2], value[1]]
}

pub(crate) fn swizzle_mu_to_gltf_matrix3(matrix: Matrix3) -> Matrix3 {
    let perm = [0usize, 2, 1];
    [
        [
            matrix[perm[0]][perm[0]],
            matrix[perm[0]][perm[1]],
            matrix[perm[0]][perm[2]],
        ],
        [
            matrix[perm[1]][perm[0]],
            matrix[perm[1]][perm[1]],
            matrix[perm[1]][perm[2]],
        ],
        [
            matrix[perm[2]][perm[0]],
            matrix[perm[2]][perm[1]],
            matrix[perm[2]][perm[2]],
        ],
    ]
}

pub(crate) fn mu_local_pose_to_gltf_trs(
    translation_mu: [f32; 3],
    rotation_mu_radians: [f32; 3],
) -> ([f32; 3], [f32; 4]) {
    let rotation_mu = rotation_matrix3_from_mu_radians(rotation_mu_radians);
    let rotation_gltf = swizzle_mu_to_gltf_matrix3(rotation_mu);
    let quaternion = matrix3_to_quaternion(rotation_gltf);
    (swizzle_mu_to_gltf_vec3(translation_mu), quaternion)
}

pub(crate) fn rotation_matrix3_from_mu_radians(rotation_radians: [f32; 3]) -> Matrix3 {
    let rotation_degrees = [
        rad_to_deg(rotation_radians[0]),
        rad_to_deg(rotation_radians[1]),
        rad_to_deg(rotation_radians[2]),
    ];
    rotation_matrix3_from_degrees(rotation_degrees)
}

pub(crate) fn rotation_matrix3_from_degrees(rotation_degrees: [f32; 3]) -> Matrix3 {
    let angle = rotation_degrees[2] * (std::f32::consts::PI * 2.0 / 360.0);
    let sy = angle.sin();
    let cy = angle.cos();
    let angle = rotation_degrees[1] * (std::f32::consts::PI * 2.0 / 360.0);
    let sp = angle.sin();
    let cp = angle.cos();
    let angle = rotation_degrees[0] * (std::f32::consts::PI * 2.0 / 360.0);
    let sr = angle.sin();
    let cr = angle.cos();

    [
        [cp * cy, sr * sp * cy + cr * -sy, cr * sp * cy + -sr * -sy],
        [cp * sy, sr * sp * sy + cr * cy, cr * sp * sy + -sr * cy],
        [-sp, sr * cp, cr * cp],
    ]
}

pub(crate) fn angle_matrix_degrees(rotation_degrees: [f32; 3]) -> Matrix3x4 {
    let rotation = rotation_matrix3_from_degrees(rotation_degrees);
    [
        [rotation[0][0], rotation[0][1], rotation[0][2], 0.0],
        [rotation[1][0], rotation[1][1], rotation[1][2], 0.0],
        [rotation[2][0], rotation[2][1], rotation[2][2], 0.0],
    ]
}

pub(crate) fn concat_transforms(left: &Matrix3x4, right: &Matrix3x4) -> Matrix3x4 {
    [
        [
            left[0][0] * right[0][0] + left[0][1] * right[1][0] + left[0][2] * right[2][0],
            left[0][0] * right[0][1] + left[0][1] * right[1][1] + left[0][2] * right[2][1],
            left[0][0] * right[0][2] + left[0][1] * right[1][2] + left[0][2] * right[2][2],
            left[0][0] * right[0][3]
                + left[0][1] * right[1][3]
                + left[0][2] * right[2][3]
                + left[0][3],
        ],
        [
            left[1][0] * right[0][0] + left[1][1] * right[1][0] + left[1][2] * right[2][0],
            left[1][0] * right[0][1] + left[1][1] * right[1][1] + left[1][2] * right[2][1],
            left[1][0] * right[0][2] + left[1][1] * right[1][2] + left[1][2] * right[2][2],
            left[1][0] * right[0][3]
                + left[1][1] * right[1][3]
                + left[1][2] * right[2][3]
                + left[1][3],
        ],
        [
            left[2][0] * right[0][0] + left[2][1] * right[1][0] + left[2][2] * right[2][0],
            left[2][0] * right[0][1] + left[2][1] * right[1][1] + left[2][2] * right[2][1],
            left[2][0] * right[0][2] + left[2][1] * right[1][2] + left[2][2] * right[2][2],
            left[2][0] * right[0][3]
                + left[2][1] * right[1][3]
                + left[2][2] * right[2][3]
                + left[2][3],
        ],
    ]
}

pub(crate) fn transform_point(matrix: &Matrix3x4, point: [f32; 3]) -> [f32; 3] {
    [
        point[0] * matrix[0][0] + point[1] * matrix[0][1] + point[2] * matrix[0][2] + matrix[0][3],
        point[0] * matrix[1][0] + point[1] * matrix[1][1] + point[2] * matrix[1][2] + matrix[1][3],
        point[0] * matrix[2][0] + point[1] * matrix[2][1] + point[2] * matrix[2][2] + matrix[2][3],
    ]
}

pub(crate) fn rotate_vector(matrix: &Matrix3x4, vector: [f32; 3]) -> [f32; 3] {
    [
        vector[0] * matrix[0][0] + vector[1] * matrix[0][1] + vector[2] * matrix[0][2],
        vector[0] * matrix[1][0] + vector[1] * matrix[1][1] + vector[2] * matrix[1][2],
        vector[0] * matrix[2][0] + vector[1] * matrix[2][1] + vector[2] * matrix[2][2],
    ]
}

pub(crate) fn normalize_vec3(vector: [f32; 3]) -> [f32; 3] {
    let length = (vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]).sqrt();
    if length == 0.0 {
        return [0.0, 0.0, 0.0];
    }
    [vector[0] / length, vector[1] / length, vector[2] / length]
}

pub(crate) fn rad_to_deg(value: f32) -> f32 {
    value * (180.0 / std::f32::consts::PI)
}

pub(crate) fn matrix3_to_quaternion(matrix: Matrix3) -> [f32; 4] {
    let trace = matrix[0][0] + matrix[1][1] + matrix[2][2];
    let mut quaternion = if trace > 0.0 {
        let s = (trace + 1.0).sqrt() * 2.0;
        [
            (matrix[2][1] - matrix[1][2]) / s,
            (matrix[0][2] - matrix[2][0]) / s,
            (matrix[1][0] - matrix[0][1]) / s,
            0.25 * s,
        ]
    } else if matrix[0][0] > matrix[1][1] && matrix[0][0] > matrix[2][2] {
        let s = (1.0 + matrix[0][0] - matrix[1][1] - matrix[2][2]).sqrt() * 2.0;
        [
            0.25 * s,
            (matrix[0][1] + matrix[1][0]) / s,
            (matrix[0][2] + matrix[2][0]) / s,
            (matrix[2][1] - matrix[1][2]) / s,
        ]
    } else if matrix[1][1] > matrix[2][2] {
        let s = (1.0 + matrix[1][1] - matrix[0][0] - matrix[2][2]).sqrt() * 2.0;
        [
            (matrix[0][1] + matrix[1][0]) / s,
            0.25 * s,
            (matrix[1][2] + matrix[2][1]) / s,
            (matrix[0][2] - matrix[2][0]) / s,
        ]
    } else {
        let s = (1.0 + matrix[2][2] - matrix[0][0] - matrix[1][1]).sqrt() * 2.0;
        [
            (matrix[0][2] + matrix[2][0]) / s,
            (matrix[1][2] + matrix[2][1]) / s,
            0.25 * s,
            (matrix[1][0] - matrix[0][1]) / s,
        ]
    };

    let length = (quaternion[0] * quaternion[0]
        + quaternion[1] * quaternion[1]
        + quaternion[2] * quaternion[2]
        + quaternion[3] * quaternion[3])
        .sqrt();
    if length > 0.0 {
        for component in &mut quaternion {
            *component /= length;
        }
    }
    quaternion
}

pub(crate) fn local_pose_matrix(
    translation_mu: [f32; 3],
    rotation_mu_radians: [f32; 3],
) -> Matrix3x4 {
    let rotation = rotation_matrix3_from_mu_radians(rotation_mu_radians);
    [
        [
            rotation[0][0],
            rotation[0][1],
            rotation[0][2],
            translation_mu[0],
        ],
        [
            rotation[1][0],
            rotation[1][1],
            rotation[1][2],
            translation_mu[1],
        ],
        [
            rotation[2][0],
            rotation[2][1],
            rotation[2][2],
            translation_mu[2],
        ],
    ]
}

pub(crate) fn model_uses_non_model_stem(path: &Utf8Path) -> bool {
    NON_MODEL_BMD_STEMS.contains(&normalized_stem(path).as_str())
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

pub(crate) fn canonicalize_model_output_path(relative: &Utf8Path) -> Utf8PathBuf {
    canonicalize_output_path(relative, true)
}

pub(crate) fn scene_objects_output_path(relative: &Utf8Path) -> Utf8PathBuf {
    let canonical_parent = canonicalize_output_path(
        relative.parent().unwrap_or_else(|| Utf8Path::new("")),
        false,
    );
    if canonical_parent.as_str().is_empty() {
        return Utf8PathBuf::from(SCENE_OBJECT_FILE_NAME);
    }
    canonical_parent.join(SCENE_OBJECT_FILE_NAME)
}

fn canonicalize_output_path(relative: &Utf8Path, normalize_filename: bool) -> Utf8PathBuf {
    let mut normalized = Utf8PathBuf::new();
    for (index, component) in relative.components().enumerate() {
        let segment = component.as_str();
        if segment.is_empty() {
            continue;
        }

        if index == 0 && segment.eq_ignore_ascii_case("data") {
            continue;
        }

        if let Some(world) = normalize_indexed_segment(segment, WORLD_DIR_PREFIX) {
            normalized.push(world);
            continue;
        }

        if let Some(object_dir) = normalize_indexed_segment(segment, OBJECT_DIR_PREFIX) {
            normalized.push(object_dir);
            continue;
        }

        if normalize_filename && relative.components().count() == index + 1 {
            normalized.push(normalize_output_filename(segment));
        } else {
            normalized.push(segment);
        }
    }

    normalized
}

fn normalize_output_filename(filename: &str) -> String {
    let path = Utf8Path::new(filename);
    let stem = path.file_stem().unwrap_or(filename);

    if let Some(normalized_stem) = normalize_indexed_segment(stem, OBJECT_MODEL_PREFIX) {
        return format!("{normalized_stem}.{GLB_EXTENSION}");
    }

    format!("{}.{}", stem.to_ascii_lowercase(), GLB_EXTENSION)
}

fn normalize_indexed_segment(segment: &str, prefix: &str) -> Option<String> {
    let lower = segment.to_ascii_lowercase();
    if !lower.starts_with(prefix) {
        return None;
    }

    let mut rest = &segment[prefix.len()..];
    if rest.starts_with('_') {
        rest = &rest[1..];
    }

    if rest.is_empty() || !rest.chars().all(|ch| ch.is_ascii_digit()) {
        return None;
    }

    let value = rest.parse::<u32>().ok()?;
    Some(format!("{prefix}_{value}"))
}

fn asset_manifest_entry(
    source_relative: &Utf8Path,
    converted_relative: &Utf8Path,
    source_bytes: &[u8],
    converted_bytes: &[u8],
) -> AssetManifestEntry {
    AssetManifestEntry {
        source_path: source_relative.to_string(),
        converted_path: converted_relative.to_string(),
        kind: MODEL_KIND.to_string(),
        source_hash: sha256_hex(source_bytes),
        converted_hash: sha256_hex(converted_bytes),
        dependencies: Vec::new(),
    }
}

fn write_output_bytes(path: &Utf8Path, bytes: &[u8]) -> Result<(), ModelConversionError> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent).map_err(|source| ModelConversionError::WriteOutput {
            path: path.to_path_buf(),
            source,
        })?;
    }
    fs::write(path, bytes).map_err(|source| ModelConversionError::WriteOutput {
        path: path.to_path_buf(),
        source,
    })
}

fn visit_model_tree(
    source_root: &Utf8Path,
    current_root: &Utf8Path,
    output_root: &Utf8Path,
    report: &mut ModelConversionReport,
) -> Result<(), ModelConversionError> {
    let mut entries = Vec::new();
    for entry in fs::read_dir(current_root).map_err(|source| ModelConversionError::ReadSource {
        path: current_root.to_path_buf(),
        source,
    })? {
        let entry = entry.map_err(|source| ModelConversionError::ReadSource {
            path: current_root.to_path_buf(),
            source,
        })?;
        let path = Utf8PathBuf::from_path_buf(entry.path()).map_err(|_| {
            ModelConversionError::ReadSource {
                path: current_root.to_path_buf(),
                source: io::Error::new(io::ErrorKind::InvalidData, "non-utf8 path"),
            }
        })?;
        entries.push(path);
    }

    entries.sort();

    for path in entries {
        if path.is_dir() {
            visit_model_tree(source_root, &path, output_root, report)?;
            continue;
        }

        let extension = normalized_extension(&path);
        if !matches!(extension.as_str(), "bmd" | "smd" | "obj") {
            continue;
        }

        match convert_model_file(source_root, &path, output_root) {
            Ok(entry) => report.converted.push(entry),
            Err(error) => report.rejected.push(ModelRejection {
                path: path.to_path_buf(),
                reason: ModelRejectionReason::from(&error),
            }),
        }
    }

    Ok(())
}

fn should_skip_bmd_source(path: &Utf8Path) -> bool {
    model_uses_non_model_stem(path)
}

pub(crate) fn report_skip_non_model_bmd(path: &Utf8Path) -> Option<ModelConversionError> {
    if should_skip_bmd_source(path) {
        Some(ModelConversionError::NoGeometry {
            path: path.to_path_buf(),
        })
    } else {
        None
    }
}

#[cfg(test)]
mod tests {
    use super::{canonicalize_model_output_path, scene_objects_output_path};
    use camino::Utf8Path;

    #[test]
    fn canonicalizes_object_model_paths() {
        let relative = Utf8Path::new("Object1/Object01.bmd");
        let canonical = canonicalize_model_output_path(relative);

        assert_eq!(canonical, Utf8Path::new("object_1/object_1.glb"));
    }

    #[test]
    fn scene_objects_output_path_uses_world_directory() {
        let relative = Utf8Path::new("World1/EncTerrain1.obj");
        let output = scene_objects_output_path(relative);

        assert_eq!(output, Utf8Path::new("world_1/scene_objects.json"));
    }
}
