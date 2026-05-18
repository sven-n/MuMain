use serde::{Deserialize, Serialize};
use sha2::{Digest, Sha256};

pub const MANIFEST_FILE_NAME: &str = "manifest.muasset.json";
pub const SUPPORTED_SCHEMA_VERSION: u32 = 1;

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct AssetManifest {
    pub schema_version: u32,
    pub source_client_version: String,
    pub content_hash: String,
    pub generated_at: String,
    pub entries: Vec<AssetManifestEntry>,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct AssetManifestEntry {
    pub source_path: String,
    pub converted_path: String,
    pub kind: String,
    pub source_hash: String,
    pub converted_hash: String,
    #[serde(default)]
    pub dependencies: Vec<String>,
}

#[derive(Debug, Serialize)]
struct ManifestDigest<'a> {
    schema_version: u32,
    source_client_version: &'a str,
    generated_at: &'a str,
    entries: &'a [AssetManifestEntry],
}

impl AssetManifest {
    pub fn compute_content_hash(&self) -> String {
        let digest = ManifestDigest {
            schema_version: self.schema_version,
            source_client_version: &self.source_client_version,
            generated_at: &self.generated_at,
            entries: &self.entries,
        };

        let digest_bytes =
            serde_json::to_vec(&digest).expect("asset manifest digest serialization is infallible");
        sha256_hex(&digest_bytes)
    }

    pub fn with_computed_content_hash(mut self) -> Self {
        self.content_hash = self.compute_content_hash();
        self
    }

    pub fn to_json_string(&self) -> Result<String, serde_json::Error> {
        serde_json::to_string_pretty(self)
    }
}

pub fn sha256_hex(bytes: &[u8]) -> String {
    let digest = Sha256::digest(bytes);
    hex_lower(digest.as_ref())
}

fn hex_lower(bytes: &[u8]) -> String {
    const HEX: &[u8; 16] = b"0123456789abcdef";
    let mut output = String::with_capacity(bytes.len() * 2);

    for byte in bytes {
        output.push(HEX[(byte >> 4) as usize] as char);
        output.push(HEX[(byte & 0x0f) as usize] as char);
    }

    output
}

#[cfg(test)]
mod tests {
    use super::{AssetManifest, AssetManifestEntry, SUPPORTED_SCHEMA_VERSION};

    #[test]
    fn content_hash_tracks_manifest_fields() {
        let manifest = AssetManifest {
            schema_version: SUPPORTED_SCHEMA_VERSION,
            source_client_version: "1.0.0".to_string(),
            content_hash: String::new(),
            generated_at: "2026-05-18T00:00:00Z".to_string(),
            entries: vec![AssetManifestEntry {
                source_path: "src/legacy.dat".to_string(),
                converted_path: "converted/legacy.dat".to_string(),
                kind: "data".to_string(),
                source_hash: "a".repeat(64),
                converted_hash: "b".repeat(64),
                dependencies: vec!["converted/shared.dat".to_string()],
            }],
        };

        let computed = manifest.compute_content_hash();

        assert_eq!(computed.len(), 64);
        assert_eq!(manifest.with_computed_content_hash().content_hash, computed);
    }

    #[test]
    fn manifest_json_roundtrips() {
        let manifest = AssetManifest {
            schema_version: SUPPORTED_SCHEMA_VERSION,
            source_client_version: "1.0.0".to_string(),
            content_hash: "c".repeat(64),
            generated_at: "2026-05-18T00:00:00Z".to_string(),
            entries: Vec::new(),
        };

        let json = manifest.to_json_string().unwrap();

        assert!(json.contains("\"schema_version\": 1"));
        assert!(json.contains("\"content_hash\":"));
        assert!(json.contains("\"entries\": []"));
    }
}
