use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use thiserror::Error;

use crate::models::{
    angle_matrix_degrees, concat_transforms, local_pose_matrix, mu_local_pose_to_gltf_trs,
    normalize_vec3, report_skip_non_model_bmd, rotate_vector, swizzle_mu_to_gltf_vec3,
    transform_point, BakedModel, Matrix3x4, ModelBone, ModelMesh, ModelVertex,
};

const MAX_BONES: usize = 200;
const MAX_MESHES: usize = 50;
const TRIANGLE_STRIDE: usize = 64;

#[derive(Debug, Error)]
pub enum BmdParseError {
    #[error("failed to read BMD {path}: {source}")]
    Read {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
    #[error("not a BMD file at {path}: magic {magic:?}")]
    NotBmd { path: Utf8PathBuf, magic: [u8; 3] },
    #[error("unsupported BMD version at {path}: {version:#04x}")]
    UnsupportedVersion { path: Utf8PathBuf, version: u8 },
    #[error("BMD data too small at {path}: {found}")]
    TooSmall { path: Utf8PathBuf, found: usize },
    #[error("invalid BMD counts at {path}: meshes={meshes}, bones={bones}, actions={actions}")]
    InvalidCounts {
        path: Utf8PathBuf,
        meshes: i16,
        bones: i16,
        actions: i16,
    },
    #[error("truncated BMD {path} while reading {section}")]
    Truncated {
        path: Utf8PathBuf,
        section: &'static str,
    },
    #[error("BMD has no renderable geometry at {path}")]
    NoGeometry { path: Utf8PathBuf },
}

#[derive(Debug, Clone)]
struct ParsedAction {
    num_keys: usize,
}

#[derive(Debug, Clone)]
struct ParsedBone {
    name: String,
    parent: Option<usize>,
    local_translation: [f32; 3],
    local_rotation: [f32; 3],
}

pub fn parse_bmd_file(path: &Utf8Path) -> Result<BakedModel, BmdParseError> {
    let bytes = fs::read(path).map_err(|source| BmdParseError::Read {
        path: path.to_path_buf(),
        source,
    })?;
    parse_bmd_bytes(path, &bytes)
}

pub(crate) fn parse_bmd_bytes(path: &Utf8Path, bytes: &[u8]) -> Result<BakedModel, BmdParseError> {
    if bytes.len() < 4 {
        return Err(BmdParseError::TooSmall {
            path: path.to_path_buf(),
            found: bytes.len(),
        });
    }

    if &bytes[..3] != b"BMD" {
        return Err(BmdParseError::NotBmd {
            path: path.to_path_buf(),
            magic: [bytes[0], bytes[1], bytes[2]],
        });
    }

    let version = bytes[3];
    if version != 0x0A {
        return Err(BmdParseError::UnsupportedVersion {
            path: path.to_path_buf(),
            version,
        });
    }

    if let Some(error) = report_skip_non_model_bmd(path) {
        return Err(match error {
            crate::models::ModelConversionError::NoGeometry { path } => {
                BmdParseError::NoGeometry { path }
            }
            _ => BmdParseError::NoGeometry {
                path: path.to_path_buf(),
            },
        });
    }

    let mut reader = Reader::new(&bytes[4..]);
    let name = reader.read_c_string(32)?;
    let num_meshes = reader.read_i16(path, "mesh count")?;
    let num_bones = reader.read_i16(path, "bone count")?;
    let num_actions = reader.read_i16(path, "action count")?;

    if num_meshes < 0
        || num_meshes as usize > MAX_MESHES
        || num_bones < 0
        || num_bones as usize > MAX_BONES
        || num_actions < 0
    {
        return Err(BmdParseError::InvalidCounts {
            path: path.to_path_buf(),
            meshes: num_meshes,
            bones: num_bones,
            actions: num_actions,
        });
    }

    let mut meshes = Vec::with_capacity(num_meshes as usize);
    for mesh_index in 0..num_meshes as usize {
        meshes.push(parse_mesh(path, &mut reader, mesh_index)?);
    }

    let mut actions = Vec::with_capacity(num_actions as usize);
    for _ in 0..num_actions as usize {
        let num_keys = reader.read_i16(path, "action key count")?;
        let lock_positions = reader.read_u8(path, "action lock flag")? != 0;
        if num_keys < 0 {
            return Err(BmdParseError::InvalidCounts {
                path: path.to_path_buf(),
                meshes: num_meshes,
                bones: num_bones,
                actions: num_actions,
            });
        }

        let key_count = num_keys as usize;
        if lock_positions {
            reader.skip_bytes(path, "action positions", key_count * 12)?;
        }

        actions.push(ParsedAction {
            num_keys: key_count,
        });
    }

    let mut bones = Vec::with_capacity(num_bones as usize);
    for bone_index in 0..num_bones as usize {
        let dummy = reader.read_u8(path, "bone dummy flag")? != 0;
        if dummy {
            bones.push(ParsedBone {
                name: format!("Null_{bone_index}"),
                parent: None,
                local_translation: [0.0, 0.0, 0.0],
                local_rotation: [0.0, 0.0, 0.0],
            });
            continue;
        }

        let bone_name = reader.read_c_string(32)?;
        let parent = reader.read_i16(path, "bone parent")?;
        let mut local_translation = [0.0; 3];
        let mut local_rotation = [0.0; 3];

        for action_index in 0..actions.len() {
            let action = &actions[action_index];
            let mut positions = Vec::with_capacity(action.num_keys);
            for _ in 0..action.num_keys {
                positions.push(reader.read_vec3(path, "bone action position")?);
            }

            let mut rotations = Vec::with_capacity(action.num_keys);
            for _ in 0..action.num_keys {
                rotations.push(reader.read_vec3(path, "bone action rotation")?);
            }

            if action_index == 0 && action.num_keys > 0 {
                local_translation = positions[0];
                local_rotation = rotations[0];
            }
        }

        bones.push(ParsedBone {
            name: bone_name,
            parent: if parent < 0 {
                None
            } else {
                Some(parent as usize)
            },
            local_translation,
            local_rotation,
        });
    }

    if meshes.iter().all(|mesh| mesh.vertices.is_empty()) {
        return Err(BmdParseError::NoGeometry {
            path: path.to_path_buf(),
        });
    }

    let bone_matrices = build_bone_world_matrices(&bones);
    let model_bones = bones
        .iter()
        .map(|bone| {
            let (translation, rotation) =
                mu_local_pose_to_gltf_trs(bone.local_translation, bone.local_rotation);
            ModelBone {
                name: bone.name.clone(),
                parent: bone.parent,
                translation,
                rotation,
            }
        })
        .collect();

    let model_meshes = meshes
        .into_iter()
        .enumerate()
        .map(|(mesh_index, mesh)| {
            let mut vertices = Vec::new();
            let source_triangles = mesh.triangles;
            let source_vertices = mesh.vertices;
            let source_normals = mesh.normals;
            let source_texcoords = mesh.texcoords;

            for triangle in source_triangles {
                if triangle.polygon == 3 {
                    append_triangle_vertices(
                        path,
                        &mut vertices,
                        &bone_matrices,
                        &source_vertices,
                        &source_normals,
                        &source_texcoords,
                        &[
                            triangle.vertex_index[0],
                            triangle.vertex_index[1],
                            triangle.vertex_index[2],
                        ],
                        &[
                            triangle.normal_index[0],
                            triangle.normal_index[1],
                            triangle.normal_index[2],
                        ],
                        &[
                            triangle.texcoord_index[0],
                            triangle.texcoord_index[1],
                            triangle.texcoord_index[2],
                        ],
                    )?;
                } else if triangle.polygon == 4 {
                    append_triangle_vertices(
                        path,
                        &mut vertices,
                        &bone_matrices,
                        &source_vertices,
                        &source_normals,
                        &source_texcoords,
                        &[
                            triangle.vertex_index[0],
                            triangle.vertex_index[1],
                            triangle.vertex_index[2],
                        ],
                        &[
                            triangle.normal_index[0],
                            triangle.normal_index[1],
                            triangle.normal_index[2],
                        ],
                        &[
                            triangle.texcoord_index[0],
                            triangle.texcoord_index[1],
                            triangle.texcoord_index[2],
                        ],
                    )?;
                    append_triangle_vertices(
                        path,
                        &mut vertices,
                        &bone_matrices,
                        &source_vertices,
                        &source_normals,
                        &source_texcoords,
                        &[
                            triangle.vertex_index[0],
                            triangle.vertex_index[2],
                            triangle.vertex_index[3],
                        ],
                        &[
                            triangle.normal_index[0],
                            triangle.normal_index[2],
                            triangle.normal_index[3],
                        ],
                        &[
                            triangle.texcoord_index[0],
                            triangle.texcoord_index[2],
                            triangle.texcoord_index[3],
                        ],
                    )?;
                }
            }

            Ok(ModelMesh {
                name: mesh
                    .texture_name
                    .clone()
                    .unwrap_or_else(|| format!("mesh_{mesh_index}")),
                texture_name: mesh.texture_name,
                vertices,
            })
        })
        .collect::<Result<Vec<_>, BmdParseError>>()?;

    Ok(BakedModel {
        name,
        bones: model_bones,
        meshes: model_meshes,
    })
}

struct ParsedMesh {
    texture_name: Option<String>,
    vertices: Vec<(i16, [f32; 3])>,
    normals: Vec<(i16, [f32; 3])>,
    texcoords: Vec<[f32; 2]>,
    triangles: Vec<ParsedTriangle>,
}

struct ParsedTriangle {
    polygon: u8,
    vertex_index: [i16; 4],
    normal_index: [i16; 4],
    texcoord_index: [i16; 4],
}

fn parse_mesh(
    path: &Utf8Path,
    reader: &mut Reader<'_>,
    _mesh_index: usize,
) -> Result<ParsedMesh, BmdParseError> {
    let num_vertices = reader.read_i16(path, "mesh vertex count")?;
    let num_normals = reader.read_i16(path, "mesh normal count")?;
    let num_texcoords = reader.read_i16(path, "mesh texcoord count")?;
    let num_triangles = reader.read_i16(path, "mesh triangle count")?;
    let _texture_index = reader.read_i16(path, "mesh texture index")?;

    if num_vertices < 0 || num_normals < 0 || num_texcoords < 0 || num_triangles < 0 {
        return Err(BmdParseError::Truncated {
            path: path.to_path_buf(),
            section: "mesh counts",
        });
    }

    let mut vertices = Vec::with_capacity(num_vertices as usize);
    for _ in 0..num_vertices as usize {
        let node = reader.read_i16(path, "vertex node")?;
        let _padding = reader.read_i16(path, "vertex padding")?;
        let position = reader.read_vec3(path, "vertex position")?;
        vertices.push((node, position));
    }

    let mut normals = Vec::with_capacity(num_normals as usize);
    for _ in 0..num_normals as usize {
        let node = reader.read_i16(path, "normal node")?;
        let _padding = reader.read_i16(path, "normal padding")?;
        let normal = reader.read_vec3(path, "normal vector")?;
        let _bind_vertex = reader.read_i16(path, "normal bind vertex")?;
        let _tail_padding = reader.read_i16(path, "normal tail padding")?;
        normals.push((node, normal));
    }

    let mut texcoords = Vec::with_capacity(num_texcoords as usize);
    for _ in 0..num_texcoords as usize {
        let u = reader.read_f32(path, "texcoord u")?;
        let v = reader.read_f32(path, "texcoord v")?;
        texcoords.push([u, 1.0 - v]);
    }

    let mut triangles = Vec::with_capacity(num_triangles as usize);
    for _ in 0..num_triangles as usize {
        let polygon = reader.read_u8(path, "triangle polygon")?;
        let _padding = reader.read_u8(path, "triangle padding")?;
        let mut vertex_index = [0i16; 4];
        let mut normal_index = [0i16; 4];
        let mut texcoord_index = [0i16; 4];
        for slot in &mut vertex_index {
            *slot = reader.read_i16(path, "triangle vertex index")?;
        }
        for slot in &mut normal_index {
            *slot = reader.read_i16(path, "triangle normal index")?;
        }
        for slot in &mut texcoord_index {
            *slot = reader.read_i16(path, "triangle texcoord index")?;
        }
        reader.skip_bytes(
            path,
            "triangle payload",
            TRIANGLE_STRIDE - 1 - 1 - 8 - 8 - 8,
        )?;

        triangles.push(ParsedTriangle {
            polygon,
            vertex_index,
            normal_index,
            texcoord_index,
        });
    }

    let texture_name = reader.read_c_string(32).ok();

    Ok(ParsedMesh {
        texture_name,
        vertices,
        normals,
        texcoords,
        triangles,
    })
}

fn build_bone_world_matrices(bones: &[ParsedBone]) -> Vec<Matrix3x4> {
    let mut cache: Vec<Option<Matrix3x4>> = vec![None; bones.len()];
    (0..bones.len())
        .map(|index| build_bone_world_matrix(index, bones, &mut cache))
        .collect()
}

fn build_bone_world_matrix(
    index: usize,
    bones: &[ParsedBone],
    cache: &mut [Option<Matrix3x4>],
) -> Matrix3x4 {
    if let Some(matrix) = cache[index] {
        return matrix;
    }

    let bone = &bones[index];
    let local = local_pose_matrix(bone.local_translation, bone.local_rotation);
    let world = if let Some(parent) = bone.parent {
        if parent < bones.len() {
            let parent_world = build_bone_world_matrix(parent, bones, cache);
            concat_transforms(&parent_world, &local)
        } else {
            local
        }
    } else {
        local
    };

    cache[index] = Some(world);
    world
}

fn append_triangle_vertices(
    path: &Utf8Path,
    vertices: &mut Vec<ModelVertex>,
    bone_matrices: &[Matrix3x4],
    source_vertices: &[(i16, [f32; 3])],
    source_normals: &[(i16, [f32; 3])],
    source_texcoords: &[[f32; 2]],
    vertex_indices: &[i16; 3],
    normal_indices: &[i16; 3],
    texcoord_indices: &[i16; 3],
) -> Result<(), BmdParseError> {
    for corner in [0usize, 2, 1] {
        let vertex_index = clamp_index(vertex_indices[corner], source_vertices.len());
        let normal_index = clamp_index(normal_indices[corner], source_normals.len());
        let texcoord_index = clamp_index(texcoord_indices[corner], source_texcoords.len());

        let (node, position) =
            source_vertices
                .get(vertex_index)
                .copied()
                .ok_or_else(|| BmdParseError::Truncated {
                    path: path.to_path_buf(),
                    section: "vertex index",
                })?;
        let (_, normal) =
            source_normals
                .get(normal_index)
                .copied()
                .ok_or_else(|| BmdParseError::Truncated {
                    path: path.to_path_buf(),
                    section: "normal index",
                })?;
        let texcoord =
            *source_texcoords
                .get(texcoord_index)
                .ok_or_else(|| BmdParseError::Truncated {
                    path: path.to_path_buf(),
                    section: "texcoord index",
                })?;

        let bone_index = if node < 0 { 0 } else { node as usize };
        let matrix = bone_matrices
            .get(bone_index)
            .copied()
            .unwrap_or_else(|| angle_matrix_degrees([0.0, 0.0, 0.0]));
        let position = swizzle_mu_to_gltf_vec3(transform_point(&matrix, position));
        let normal = normalize_vec3(swizzle_mu_to_gltf_vec3(rotate_vector(&matrix, normal)));

        vertices.push(ModelVertex {
            position,
            normal,
            texcoord,
        });
    }

    Ok(())
}

fn clamp_index(index: i16, len: usize) -> usize {
    if index < 0 {
        0
    } else {
        let index = index as usize;
        if len == 0 {
            0
        } else {
            index.min(len - 1)
        }
    }
}

struct Reader<'a> {
    data: &'a [u8],
    cursor: usize,
}

impl<'a> Reader<'a> {
    fn new(data: &'a [u8]) -> Self {
        Self { data, cursor: 0 }
    }

    fn read_u8(&mut self, path: &Utf8Path, section: &'static str) -> Result<u8, BmdParseError> {
        if self.cursor >= self.data.len() {
            return Err(BmdParseError::Truncated {
                path: path.to_path_buf(),
                section,
            });
        }
        let value = self.data[self.cursor];
        self.cursor += 1;
        Ok(value)
    }

    fn read_i16(&mut self, path: &Utf8Path, section: &'static str) -> Result<i16, BmdParseError> {
        let bytes = self.read_bytes(path, section, 2)?;
        Ok(i16::from_le_bytes([bytes[0], bytes[1]]))
    }

    fn read_f32(&mut self, path: &Utf8Path, section: &'static str) -> Result<f32, BmdParseError> {
        let bytes = self.read_bytes(path, section, 4)?;
        Ok(f32::from_le_bytes([bytes[0], bytes[1], bytes[2], bytes[3]]))
    }

    fn read_vec3(
        &mut self,
        path: &Utf8Path,
        section: &'static str,
    ) -> Result<[f32; 3], BmdParseError> {
        Ok([
            self.read_f32(path, section)?,
            self.read_f32(path, section)?,
            self.read_f32(path, section)?,
        ])
    }

    fn read_c_string(&mut self, len: usize) -> Result<String, BmdParseError> {
        let bytes = self.read_bytes_no_error(len)?;
        let end = bytes
            .iter()
            .position(|byte| *byte == 0)
            .unwrap_or(bytes.len());
        Ok(String::from_utf8_lossy(&bytes[..end]).trim().to_string())
    }

    fn skip_bytes(
        &mut self,
        path: &Utf8Path,
        section: &'static str,
        len: usize,
    ) -> Result<(), BmdParseError> {
        let _ = self.read_bytes(path, section, len)?;
        Ok(())
    }

    fn read_bytes(
        &mut self,
        path: &Utf8Path,
        section: &'static str,
        len: usize,
    ) -> Result<&'a [u8], BmdParseError> {
        if self.cursor + len > self.data.len() {
            return Err(BmdParseError::Truncated {
                path: path.to_path_buf(),
                section,
            });
        }
        let bytes = &self.data[self.cursor..self.cursor + len];
        self.cursor += len;
        Ok(bytes)
    }

    fn read_bytes_no_error(&mut self, len: usize) -> Result<&'a [u8], BmdParseError> {
        if self.cursor + len > self.data.len() {
            return Err(BmdParseError::Truncated {
                path: Utf8PathBuf::from("bmd"),
                section: "c-string",
            });
        }
        let bytes = &self.data[self.cursor..self.cursor + len];
        self.cursor += len;
        Ok(bytes)
    }
}
