use std::collections::BTreeMap;
use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use thiserror::Error;

use crate::models::{
    mu_local_pose_to_gltf_trs, normalize_vec3, swizzle_mu_to_gltf_vec3, BakedModel, ModelBone,
    ModelMesh, ModelVertex,
};

#[derive(Debug, Error)]
pub enum SmdParseError {
    #[error("failed to read SMD {path}: {source}")]
    Read {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
    #[error("invalid SMD token at {path}: {reason}")]
    InvalidToken { path: Utf8PathBuf, reason: String },
    #[error("invalid SMD structure at {path}: {reason}")]
    InvalidStructure { path: Utf8PathBuf, reason: String },
    #[error("SMD has no renderable geometry at {path}")]
    NoGeometry { path: Utf8PathBuf },
}

#[derive(Debug, Clone)]
struct ParsedNode {
    name: String,
    parent: i32,
}

#[derive(Debug, Clone)]
struct ParsedTriangle {
    texture_name: String,
    vertices: [ParsedTriangleVertex; 3],
}

#[derive(Debug, Clone, Copy)]
struct ParsedTriangleVertex {
    node: i32,
    position: [f32; 3],
    normal: [f32; 3],
    texcoord: [f32; 2],
}

pub fn parse_smd_file(path: &Utf8Path) -> Result<BakedModel, SmdParseError> {
    let content = fs::read_to_string(path).map_err(|source| SmdParseError::Read {
        path: path.to_path_buf(),
        source,
    })?;
    parse_smd_text(path, &content)
}

pub(crate) fn parse_smd_text(path: &Utf8Path, content: &str) -> Result<BakedModel, SmdParseError> {
    let tokens = tokenize(path, content)?;
    let mut stream = TokenStream::new(&tokens);

    stream.expect_name(path, "version")?;
    let _version = stream.expect_number(path)?;

    let nodes = parse_nodes(path, &mut stream)?;
    let node_transforms = parse_skeleton(path, &mut stream, nodes.len())?;
    let triangles = parse_triangles(path, &mut stream)?;

    if triangles.is_empty() {
        return Err(SmdParseError::NoGeometry {
            path: path.to_path_buf(),
        });
    }

    let bones = nodes
        .iter()
        .enumerate()
        .map(|(index, node)| {
            let transform = node_transforms
                .get(index)
                .copied()
                .unwrap_or(([0.0, 0.0, 0.0], [0.0, 0.0, 0.0]));
            let (translation, rotation) = mu_local_pose_to_gltf_trs(transform.0, transform.1);
            ModelBone {
                name: node.name.clone(),
                parent: if node.parent < 0 {
                    None
                } else {
                    Some(node.parent as usize)
                },
                translation,
                rotation,
            }
        })
        .collect();

    let mut meshes_by_texture: BTreeMap<String, Vec<ModelVertex>> = BTreeMap::new();
    for triangle in triangles {
        let mesh = meshes_by_texture
            .entry(triangle.texture_name.clone())
            .or_default();
        for vertex in [
            triangle.vertices[0],
            triangle.vertices[2],
            triangle.vertices[1],
        ] {
            mesh.push(ModelVertex {
                position: swizzle_mu_to_gltf_vec3(vertex.position),
                normal: normalize_vec3(swizzle_mu_to_gltf_vec3(vertex.normal)),
                texcoord: vertex.texcoord,
            });
        }
    }

    let meshes = meshes_by_texture
        .into_iter()
        .map(|(texture_name, vertices)| ModelMesh {
            name: texture_name.clone(),
            texture_name: Some(texture_name),
            vertices,
        })
        .collect();

    Ok(BakedModel {
        name: path
            .file_stem()
            .map(|stem| stem.to_string())
            .unwrap_or_else(|| "smd".to_string()),
        bones,
        meshes,
    })
}

fn parse_nodes(
    path: &Utf8Path,
    stream: &mut TokenStream<'_>,
) -> Result<Vec<ParsedNode>, SmdParseError> {
    stream.expect_name(path, "nodes")?;
    let mut nodes = Vec::new();
    loop {
        match stream.peek() {
            Some(Token::Name(name)) if name.eq_ignore_ascii_case("end") => {
                stream.next();
                break;
            }
            Some(Token::Number(_)) => {
                let _index = stream.expect_number(path)? as i32;
                let name = stream.expect_name_any(path)?;
                let parent = stream.expect_number(path)? as i32;
                nodes.push(ParsedNode { name, parent });
            }
            Some(token) => {
                return Err(SmdParseError::InvalidToken {
                    path: path.to_path_buf(),
                    reason: format!("unexpected token in nodes section: {token:?}"),
                })
            }
            None => {
                return Err(SmdParseError::InvalidStructure {
                    path: path.to_path_buf(),
                    reason: "missing nodes end marker".to_string(),
                })
            }
        }
    }
    Ok(nodes)
}

fn parse_skeleton(
    path: &Utf8Path,
    stream: &mut TokenStream<'_>,
    node_count: usize,
) -> Result<Vec<([f32; 3], [f32; 3])>, SmdParseError> {
    stream.expect_name(path, "skeleton")?;
    let mut transforms = vec![([0.0, 0.0, 0.0], [0.0, 0.0, 0.0]); node_count];
    let mut have_time_zero = false;

    loop {
        match stream.peek() {
            Some(Token::Name(name)) if name.eq_ignore_ascii_case("end") => {
                stream.next();
                break;
            }
            Some(Token::Name(name)) if name.eq_ignore_ascii_case("time") => {
                stream.next();
                let time_index = stream.expect_number(path)? as i32;
                for _ in 0..node_count {
                    let node_index = stream.expect_number(path)? as usize;
                    let position = [
                        stream.expect_number(path)?,
                        stream.expect_number(path)?,
                        stream.expect_number(path)?,
                    ];
                    let rotation = [
                        stream.expect_number(path)?,
                        stream.expect_number(path)?,
                        stream.expect_number(path)?,
                    ];
                    if time_index == 0 && node_index < node_count {
                        transforms[node_index] = (position, rotation);
                        have_time_zero = true;
                    }
                }
            }
            Some(token) => {
                return Err(SmdParseError::InvalidToken {
                    path: path.to_path_buf(),
                    reason: format!("unexpected token in skeleton section: {token:?}"),
                })
            }
            None => {
                return Err(SmdParseError::InvalidStructure {
                    path: path.to_path_buf(),
                    reason: "missing skeleton end marker".to_string(),
                })
            }
        }
    }

    if !have_time_zero {
        return Ok(transforms);
    }

    Ok(transforms)
}

fn parse_triangles(
    path: &Utf8Path,
    stream: &mut TokenStream<'_>,
) -> Result<Vec<ParsedTriangle>, SmdParseError> {
    stream.expect_name(path, "triangles")?;
    let mut triangles = Vec::new();
    loop {
        match stream.peek() {
            Some(Token::Name(name)) if name.eq_ignore_ascii_case("end") => {
                stream.next();
                break;
            }
            Some(Token::Name(texture_name)) => {
                let texture_name = texture_name.clone();
                stream.next();
                let mut vertices = [ParsedTriangleVertex {
                    node: 0,
                    position: [0.0, 0.0, 0.0],
                    normal: [0.0, 0.0, 0.0],
                    texcoord: [0.0, 0.0],
                }; 3];
                for vertex in &mut vertices {
                    vertex.node = stream.expect_number(path)? as i32;
                    vertex.position = [
                        stream.expect_number(path)?,
                        stream.expect_number(path)?,
                        stream.expect_number(path)?,
                    ];
                    vertex.normal = [
                        stream.expect_number(path)?,
                        stream.expect_number(path)?,
                        stream.expect_number(path)?,
                    ];
                    vertex.texcoord[0] = stream.expect_number(path)?;
                    vertex.texcoord[1] = 1.0 - stream.expect_number(path)?;
                }
                triangles.push(ParsedTriangle {
                    texture_name,
                    vertices,
                });
            }
            Some(token) => {
                return Err(SmdParseError::InvalidToken {
                    path: path.to_path_buf(),
                    reason: format!("unexpected token in triangles section: {token:?}"),
                })
            }
            None => {
                return Err(SmdParseError::InvalidStructure {
                    path: path.to_path_buf(),
                    reason: "missing triangles end marker".to_string(),
                })
            }
        }
    }
    Ok(triangles)
}

#[derive(Debug, Clone)]
enum Token {
    Name(String),
    Number(f32),
}

fn tokenize(path: &Utf8Path, content: &str) -> Result<Vec<Token>, SmdParseError> {
    let mut tokens = Vec::new();
    let mut chars = content.chars().peekable();

    while let Some(ch) = chars.next() {
        if ch.is_whitespace() {
            continue;
        }

        if ch == '/' && chars.peek() == Some(&'/') {
            chars.next();
            while let Some(next) = chars.next() {
                if next == '\n' {
                    break;
                }
            }
            continue;
        }

        if ch == '"' {
            let mut value = String::new();
            while let Some(next) = chars.next() {
                if next == '"' {
                    break;
                }
                value.push(next);
            }
            tokens.push(Token::Name(value));
            continue;
        }

        if ch.is_ascii_digit() || ch == '-' || ch == '.' {
            let mut value = String::new();
            value.push(ch);
            while let Some(next) = chars.peek() {
                if next.is_ascii_digit() || *next == '.' || *next == '-' {
                    value.push(*next);
                    chars.next();
                } else {
                    break;
                }
            }
            let number = value
                .parse::<f32>()
                .map_err(|source| SmdParseError::InvalidToken {
                    path: path.to_path_buf(),
                    reason: source.to_string(),
                })?;
            tokens.push(Token::Number(number));
            continue;
        }

        if ch.is_ascii_alphabetic() {
            let mut value = String::new();
            value.push(ch);
            while let Some(next) = chars.peek() {
                if next.is_ascii_alphanumeric() || *next == '_' || *next == '.' {
                    value.push(*next);
                    chars.next();
                } else {
                    break;
                }
            }
            tokens.push(Token::Name(value));
            continue;
        }
    }

    Ok(tokens)
}

struct TokenStream<'a> {
    tokens: &'a [Token],
    index: usize,
}

impl<'a> TokenStream<'a> {
    fn new(tokens: &'a [Token]) -> Self {
        Self { tokens, index: 0 }
    }

    fn peek(&self) -> Option<&'a Token> {
        self.tokens.get(self.index)
    }

    fn next(&mut self) -> Option<&'a Token> {
        let token = self.tokens.get(self.index);
        if token.is_some() {
            self.index += 1;
        }
        token
    }

    fn expect_name(&mut self, path: &Utf8Path, expected: &str) -> Result<(), SmdParseError> {
        let token = self.next().ok_or_else(|| SmdParseError::InvalidStructure {
            path: path.to_path_buf(),
            reason: format!("expected {expected}"),
        })?;
        match token {
            Token::Name(value) if value.eq_ignore_ascii_case(expected) => Ok(()),
            other => Err(SmdParseError::InvalidToken {
                path: path.to_path_buf(),
                reason: format!("expected {expected}, found {other:?}"),
            }),
        }
    }

    fn expect_name_any(&mut self, path: &Utf8Path) -> Result<String, SmdParseError> {
        let token = self.next().ok_or_else(|| SmdParseError::InvalidStructure {
            path: path.to_path_buf(),
            reason: "expected name".to_string(),
        })?;
        match token {
            Token::Name(value) => Ok(value.clone()),
            other => Err(SmdParseError::InvalidToken {
                path: path.to_path_buf(),
                reason: format!("expected name, found {other:?}"),
            }),
        }
    }

    fn expect_number(&mut self, path: &Utf8Path) -> Result<f32, SmdParseError> {
        let token = self.next().ok_or_else(|| SmdParseError::InvalidStructure {
            path: path.to_path_buf(),
            reason: "expected number".to_string(),
        })?;
        match token {
            Token::Number(value) => Ok(*value),
            other => Err(SmdParseError::InvalidToken {
                path: path.to_path_buf(),
                reason: format!("expected number, found {other:?}"),
            }),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::parse_smd_text;
    use camino::Utf8Path;

    #[test]
    fn parses_minimal_smd() {
        let content = r#"
version 1
nodes
0 "root" -1
end
skeleton
time 0
0 0 0 0 0 0 0
end
triangles
texture
0 0 0 0 0 0 1 0 0
0 1 0 0 0 0 1 1 0
0 0 1 0 0 0 1 0 1
end
"#;

        let model = parse_smd_text(Utf8Path::new("test.smd"), content).unwrap();

        assert_eq!(model.meshes.len(), 1);
        assert_eq!(model.meshes[0].vertices.len(), 3);
        assert_eq!(model.bones.len(), 1);
    }
}
