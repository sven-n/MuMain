use std::fs;
use std::io::Cursor;

use camino::{Utf8Path, Utf8PathBuf};
use image::{DynamicImage, ImageFormat};
use mu_assets::{sha256_hex, AssetManifestEntry};
use thiserror::Error;

const TEXTURE_KIND: &str = "texture";
const PNG_EXTENSION: &str = "png";
const SUPPORTED_EXTENSIONS: &[&str] = &["bmp", "tga", "ozj", "ozt"];
const OZJ_HEADER_BYTES: usize = 24;
const OZT_HEADER_BYTES: usize = 4;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TextureConversionReport {
    pub source_root: Utf8PathBuf,
    pub output_root: Utf8PathBuf,
    pub converted: Vec<AssetManifestEntry>,
    pub rejected: Vec<TextureRejection>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TextureRejection {
    pub path: Utf8PathBuf,
    pub reason: TextureRejectionReason,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TextureRejectionReason {
    UnsupportedExtension,
    SourceOutsideRoot,
    ReadFailed,
    HeaderTooShort,
    DecodeFailed,
    WriteFailed,
}

impl TextureRejectionReason {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::UnsupportedExtension => "unsupported-extension",
            Self::SourceOutsideRoot => "source-outside-root",
            Self::ReadFailed => "read-failed",
            Self::HeaderTooShort => "header-too-short",
            Self::DecodeFailed => "decode-failed",
            Self::WriteFailed => "write-failed",
        }
    }
}

#[derive(Debug, Error)]
pub enum TextureConversionError {
    #[error("unsupported texture extension at {path}: {extension}")]
    UnsupportedTextureExtension {
        path: Utf8PathBuf,
        extension: String,
    },
    #[error("texture source path escapes the source root: {path}")]
    SourceOutsideRoot { path: Utf8PathBuf },
    #[error("failed to read texture {path}: {source}")]
    ReadTexture {
        path: Utf8PathBuf,
        #[source]
        source: std::io::Error,
    },
    #[error(
        "texture payload too short at {path}: expected at least {expected} bytes, found {found}"
    )]
    HeaderTooShort {
        path: Utf8PathBuf,
        expected: usize,
        found: usize,
    },
    #[error("failed to decode texture {path}: {source}")]
    DecodeTexture {
        path: Utf8PathBuf,
        #[source]
        source: image::ImageError,
    },
    #[error("failed to encode texture {path}: {source}")]
    EncodeTexture {
        path: Utf8PathBuf,
        #[source]
        source: image::ImageError,
    },
    #[error("failed to write texture {path}: {source}")]
    WriteTexture {
        path: Utf8PathBuf,
        #[source]
        source: std::io::Error,
    },
}

impl From<&TextureConversionError> for TextureRejectionReason {
    fn from(error: &TextureConversionError) -> Self {
        match error {
            TextureConversionError::UnsupportedTextureExtension { .. } => {
                Self::UnsupportedExtension
            }
            TextureConversionError::SourceOutsideRoot { .. } => Self::SourceOutsideRoot,
            TextureConversionError::ReadTexture { .. } => Self::ReadFailed,
            TextureConversionError::HeaderTooShort { .. } => Self::HeaderTooShort,
            TextureConversionError::DecodeTexture { .. } => Self::DecodeFailed,
            TextureConversionError::EncodeTexture { .. } => Self::WriteFailed,
            TextureConversionError::WriteTexture { .. } => Self::WriteFailed,
        }
    }
}

pub fn convert_textures(
    source_root: &Utf8Path,
    output_root: &Utf8Path,
) -> Result<TextureConversionReport, TextureConversionError> {
    let mut report = TextureConversionReport {
        source_root: source_root.to_path_buf(),
        output_root: output_root.to_path_buf(),
        converted: Vec::new(),
        rejected: Vec::new(),
    };

    visit_texture_tree(source_root, source_root, output_root, &mut report)?;
    report
        .converted
        .sort_by(|left, right| left.source_path.cmp(&right.source_path));
    report
        .rejected
        .sort_by(|left, right| left.path.cmp(&right.path));
    Ok(report)
}

pub fn convert_texture_file(
    source_root: &Utf8Path,
    source_path: &Utf8Path,
    output_root: &Utf8Path,
) -> Result<AssetManifestEntry, TextureConversionError> {
    let relative_path = source_path.strip_prefix(source_root).map_err(|_| {
        TextureConversionError::SourceOutsideRoot {
            path: source_path.to_path_buf(),
        }
    })?;

    let extension = normalized_extension(source_path);
    if !is_supported_extension(&extension) {
        return Err(TextureConversionError::UnsupportedTextureExtension {
            path: source_path.to_path_buf(),
            extension,
        });
    }

    let source_bytes =
        fs::read(source_path).map_err(|source| TextureConversionError::ReadTexture {
            path: source_path.to_path_buf(),
            source,
        })?;
    let image = decode_texture(source_path, &extension, &source_bytes)?;
    let converted_bytes = encode_png(source_path, image)?;

    let converted_relative_path = relative_path.with_extension(PNG_EXTENSION);
    let converted_path = output_root.join(&converted_relative_path);
    if let Some(parent) = converted_path.parent() {
        fs::create_dir_all(parent).map_err(|source| TextureConversionError::WriteTexture {
            path: converted_path.clone(),
            source,
        })?;
    }
    fs::write(&converted_path, &converted_bytes).map_err(|source| {
        TextureConversionError::WriteTexture {
            path: converted_path.clone(),
            source,
        }
    })?;

    Ok(AssetManifestEntry {
        source_path: relative_path.to_string(),
        converted_path: converted_relative_path.to_string(),
        kind: TEXTURE_KIND.to_string(),
        source_hash: sha256_hex(&source_bytes),
        converted_hash: sha256_hex(&converted_bytes),
        dependencies: Vec::new(),
    })
}

fn visit_texture_tree(
    source_root: &Utf8Path,
    current_root: &Utf8Path,
    output_root: &Utf8Path,
    report: &mut TextureConversionReport,
) -> Result<(), TextureConversionError> {
    let mut entries = Vec::new();
    for entry in
        fs::read_dir(current_root).map_err(|source| TextureConversionError::ReadTexture {
            path: current_root.to_path_buf(),
            source,
        })?
    {
        let entry = entry.map_err(|source| TextureConversionError::ReadTexture {
            path: current_root.to_path_buf(),
            source,
        })?;
        let path = Utf8PathBuf::from_path_buf(entry.path()).map_err(|_| {
            TextureConversionError::SourceOutsideRoot {
                path: current_root.to_path_buf(),
            }
        })?;
        entries.push(path);
    }

    entries.sort();

    for path in entries {
        if path.is_dir() {
            visit_texture_tree(source_root, &path, output_root, report)?;
            continue;
        }

        if !is_supported_extension(&normalized_extension(&path)) {
            continue;
        }

        match convert_texture_file(source_root, &path, output_root) {
            Ok(entry) => report.converted.push(entry),
            Err(error) => report.rejected.push(TextureRejection {
                path: path.to_path_buf(),
                reason: TextureRejectionReason::from(&error),
            }),
        }
    }

    Ok(())
}

fn decode_texture(
    path: &Utf8Path,
    extension: &str,
    source_bytes: &[u8],
) -> Result<DynamicImage, TextureConversionError> {
    let payload = match extension {
        "ozj" => strip_header(path, source_bytes, OZJ_HEADER_BYTES)?,
        "ozt" => strip_header(path, source_bytes, OZT_HEADER_BYTES)?,
        _ => source_bytes,
    };

    let format = match extension {
        "bmp" => ImageFormat::Bmp,
        "tga" | "ozt" => ImageFormat::Tga,
        "ozj" => ImageFormat::Jpeg,
        _ => ImageFormat::Png,
    };

    image::load_from_memory_with_format(payload, format).map_err(|source| {
        TextureConversionError::DecodeTexture {
            path: path.to_path_buf(),
            source,
        }
    })
}

fn strip_header<'a>(
    path: &Utf8Path,
    bytes: &'a [u8],
    header_bytes: usize,
) -> Result<&'a [u8], TextureConversionError> {
    if bytes.len() < header_bytes {
        return Err(TextureConversionError::HeaderTooShort {
            path: path.to_path_buf(),
            expected: header_bytes,
            found: bytes.len(),
        });
    }

    Ok(&bytes[header_bytes..])
}

fn encode_png(path: &Utf8Path, image: DynamicImage) -> Result<Vec<u8>, TextureConversionError> {
    let mut output = Cursor::new(Vec::new());
    image
        .write_to(&mut output, ImageFormat::Png)
        .map_err(|source| TextureConversionError::EncodeTexture {
            path: path.to_path_buf(),
            source,
        })?;

    Ok(output.into_inner())
}

fn normalized_extension(path: &Utf8Path) -> String {
    path.extension()
        .map(str::to_ascii_lowercase)
        .unwrap_or_default()
}

fn is_supported_extension(extension: &str) -> bool {
    SUPPORTED_EXTENSIONS.contains(&extension)
}

#[cfg(test)]
mod tests {
    use std::fs;
    use std::io::Cursor;

    use camino::Utf8PathBuf;
    use image::{DynamicImage, ImageFormat, RgbaImage};

    use super::{convert_texture_file, convert_textures, TextureRejectionReason};

    fn temp_root(label: &str) -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_asset_pipeline_{label}_{}_{}",
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

    fn write_image(path: &Utf8PathBuf, format: ImageFormat, rgba: [u8; 4]) {
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent).unwrap();
        }

        let mut bytes = Cursor::new(Vec::new());
        DynamicImage::ImageRgba8(single_pixel(rgba))
            .write_to(&mut bytes, format)
            .unwrap();
        fs::write(path, bytes.into_inner()).unwrap();
    }

    fn write_container_image(
        path: &Utf8PathBuf,
        header_bytes: usize,
        format: ImageFormat,
        rgba: [u8; 4],
    ) {
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent).unwrap();
        }

        let mut bytes = vec![0u8; header_bytes];
        let mut encoded = Cursor::new(Vec::new());
        DynamicImage::ImageRgba8(single_pixel(rgba))
            .write_to(&mut encoded, format)
            .unwrap();
        bytes.extend_from_slice(&encoded.into_inner());
        fs::write(path, bytes).unwrap();
    }

    fn single_pixel(rgba: [u8; 4]) -> RgbaImage {
        RgbaImage::from_raw(1, 1, rgba.to_vec()).unwrap()
    }

    #[test]
    fn converts_supported_texture_formats_into_pngs() {
        let source_root = temp_root("source");
        let output_root = temp_root("output");

        write_image(
            &source_root.join("Interface/icon.bmp"),
            ImageFormat::Bmp,
            [10, 20, 30, 255],
        );
        write_image(
            &source_root.join("Interface/overlay.tga"),
            ImageFormat::Tga,
            [40, 50, 60, 128],
        );
        write_container_image(
            &source_root.join("World1/terrain.ozj"),
            24,
            ImageFormat::Jpeg,
            [70, 80, 90, 255],
        );
        write_container_image(
            &source_root.join("World1/mask.ozt"),
            4,
            ImageFormat::Tga,
            [100, 110, 120, 255],
        );

        let report = convert_textures(&source_root, &output_root).unwrap();

        assert_eq!(report.converted.len(), 4);
        assert!(report.rejected.is_empty());
        assert_eq!(report.converted[0].kind, "texture");
        assert!(report
            .converted
            .iter()
            .all(|entry| entry.converted_path.ends_with(".png")));

        let icon = image::open(output_root.join("Interface/icon.png"))
            .unwrap()
            .to_rgba8();
        assert_eq!(icon.get_pixel(0, 0).0, [10, 20, 30, 255]);

        let overlay = image::open(output_root.join("Interface/overlay.png"))
            .unwrap()
            .to_rgba8();
        assert_eq!(overlay.get_pixel(0, 0).0[3], 128);
    }

    #[test]
    fn reports_decode_failures_as_rejections() {
        let source_root = temp_root("reject-source");
        let output_root = temp_root("reject-output");

        fs::create_dir_all(source_root.join("World1")).unwrap();
        fs::write(source_root.join("World1/broken.ozj"), vec![0u8; 24]).unwrap();

        let report = convert_textures(&source_root, &output_root).unwrap();

        assert_eq!(report.converted.len(), 0);
        assert_eq!(report.rejected.len(), 1);
        assert_eq!(
            report.rejected[0].reason,
            TextureRejectionReason::DecodeFailed
        );
    }

    #[test]
    fn convert_texture_file_rejects_paths_outside_the_source_root() {
        let source_root = temp_root("single-source");
        let output_root = temp_root("single-output");
        let source_path = temp_root("outside").join("icon.bmp");
        write_image(&source_path, ImageFormat::Bmp, [5, 6, 7, 255]);

        let error = convert_texture_file(&source_root, &source_path, &output_root).unwrap_err();

        assert!(matches!(
            error,
            super::TextureConversionError::SourceOutsideRoot { .. }
        ));
    }
}
