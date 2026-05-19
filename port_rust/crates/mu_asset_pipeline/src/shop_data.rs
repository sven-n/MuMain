use std::fs;
use std::io;

use camino::{Utf8Path, Utf8PathBuf};
use thiserror::Error;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct LegacyTimestamp {
    pub year: u16,
    pub month: u8,
    pub day: u8,
    pub hour: Option<u8>,
    pub minute: Option<u8>,
    pub second: Option<u8>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ShopCategoryRecord {
    pub product_display_seq: u32,
    pub name: String,
    pub event_flag: u32,
    pub open_flag: u32,
    pub parent_product_display_seq: u32,
    pub display_order: u32,
    pub root: u32,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ShopPackageRecord {
    pub product_display_seq: u32,
    pub view_order: u32,
    pub package_product_seq: u32,
    pub package_product_name: String,
    pub package_product_type: u32,
    pub price: u32,
    pub description: String,
    pub caution: String,
    pub sales_flag: u32,
    pub gift_flag: u32,
    pub start_date: LegacyTimestamp,
    pub end_date: LegacyTimestamp,
    pub capsule_flag: u32,
    pub capsule_count: u32,
    pub product_cash_name: String,
    pub price_unit_name: String,
    pub delete_flag: u32,
    pub event_flag: u32,
    pub product_amount: u32,
    pub product_seq_list: Vec<u32>,
    pub in_game_package_id: String,
    pub product_cash_seq: u32,
    pub price_count: u32,
    pub price_seq_list: Vec<u32>,
    pub deduct_mileage_flag: bool,
    pub cash_type: u32,
    pub cash_type_flag: u32,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ShopProductRecord {
    pub product_seq: u32,
    pub product_name: String,
    pub property_name: String,
    pub value: String,
    pub unit_name: String,
    pub price: u32,
    pub price_seq: u32,
    pub property_type: u32,
    pub must_flag: u32,
    pub view_order: u32,
    pub delete_flag: u32,
    pub storage_group: u32,
    pub share_flag: u32,
    pub in_game_package_id: String,
    pub property_seq: u32,
    pub product_type: u32,
    pub unit_type: u32,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ShopBannerRecord {
    pub banner_seq: u32,
    pub banner_name: String,
    pub banner_image_url: String,
    pub banner_order: u32,
    pub banner_direction: u32,
    pub start_date: LegacyTimestamp,
    pub end_date: LegacyTimestamp,
    pub banner_link_url: String,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ShopDataReport {
    pub script_root: Utf8PathBuf,
    pub banner_root: Utf8PathBuf,
    pub category_file: Utf8PathBuf,
    pub package_file: Utf8PathBuf,
    pub product_file: Utf8PathBuf,
    pub banner_file: Utf8PathBuf,
    pub categories: Vec<ShopCategoryRecord>,
    pub packages: Vec<ShopPackageRecord>,
    pub products: Vec<ShopProductRecord>,
    pub banners: Vec<ShopBannerRecord>,
}

#[derive(Debug, Error)]
pub enum ShopDataError {
    #[error("failed to read {path}: {source}")]
    Io {
        path: Utf8PathBuf,
        #[source]
        source: io::Error,
    },
    #[error("{kind} line {line} in {path} has {actual} fields, expected {expected}")]
    InvalidFieldCount {
        path: Utf8PathBuf,
        kind: &'static str,
        line: usize,
        expected: usize,
        actual: usize,
    },
    #[error("{kind} line {line} in {path} has invalid integer in field {field}: {value}")]
    InvalidInteger {
        path: Utf8PathBuf,
        kind: &'static str,
        line: usize,
        field: &'static str,
        value: String,
    },
    #[error("{kind} line {line} in {path} has invalid timestamp in field {field}: {value}")]
    InvalidTimestamp {
        path: Utf8PathBuf,
        kind: &'static str,
        line: usize,
        field: &'static str,
        value: String,
    },
    #[error("missing {file_name} under {root}")]
    MissingFile {
        root: Utf8PathBuf,
        file_name: &'static str,
    },
}

pub fn load_shop_data(
    script_root: &Utf8Path,
    banner_root: &Utf8Path,
) -> Result<ShopDataReport, ShopDataError> {
    let category_file = find_latest_named_file(script_root, "IBSCategory.txt")?;
    let package_file = find_latest_named_file(script_root, "IBSPackage.txt")?;
    let product_file = find_latest_named_file(script_root, "IBSProduct.txt")?;
    let banner_file = find_latest_named_file(banner_root, "IBSBanner.txt")?;

    Ok(ShopDataReport {
        script_root: script_root.to_path_buf(),
        banner_root: banner_root.to_path_buf(),
        categories: parse_category_file(&category_file)?,
        packages: parse_package_file(&package_file)?,
        products: parse_product_file(&product_file)?,
        banners: parse_banner_file(&banner_file)?,
        category_file,
        package_file,
        product_file,
        banner_file,
    })
}

pub fn parse_category_file(path: &Utf8Path) -> Result<Vec<ShopCategoryRecord>, ShopDataError> {
    parse_legacy_lines(path, "category", 7, |path, line, fields| {
        Ok(ShopCategoryRecord {
            product_display_seq: parse_u32_field(
                path,
                "category",
                line,
                "product_display_seq",
                fields[0],
            )?,
            name: fields[1].to_owned(),
            event_flag: parse_u32_field(path, "category", line, "event_flag", fields[2])?,
            open_flag: parse_u32_field(path, "category", line, "open_flag", fields[3])?,
            parent_product_display_seq: parse_u32_field(
                path,
                "category",
                line,
                "parent_product_display_seq",
                fields[4],
            )?,
            display_order: parse_u32_field(path, "category", line, "display_order", fields[5])?,
            root: parse_u32_field(path, "category", line, "root", fields[6])?,
        })
    })
}

pub fn parse_package_file(path: &Utf8Path) -> Result<Vec<ShopPackageRecord>, ShopDataError> {
    parse_legacy_lines(path, "package", 27, |path, line, fields| {
        Ok(ShopPackageRecord {
            product_display_seq: parse_u32_field(
                path,
                "package",
                line,
                "product_display_seq",
                fields[0],
            )?,
            view_order: parse_u32_field(path, "package", line, "view_order", fields[1])?,
            package_product_seq: parse_u32_field(
                path,
                "package",
                line,
                "package_product_seq",
                fields[2],
            )?,
            package_product_name: fields[3].to_owned(),
            package_product_type: parse_u32_field(
                path,
                "package",
                line,
                "package_product_type",
                fields[4],
            )?,
            price: parse_u32_field(path, "package", line, "price", fields[5])?,
            description: fields[6].to_owned(),
            caution: fields[7].to_owned(),
            sales_flag: parse_u32_field(path, "package", line, "sales_flag", fields[8])?,
            gift_flag: parse_u32_field(path, "package", line, "gift_flag", fields[9])?,
            start_date: parse_timestamp_field(path, "package", line, "start_date", fields[10])?,
            end_date: parse_timestamp_field(path, "package", line, "end_date", fields[11])?,
            capsule_flag: parse_u32_field(path, "package", line, "capsule_flag", fields[12])?,
            capsule_count: parse_u32_field(path, "package", line, "capsule_count", fields[13])?,
            product_cash_name: fields[14].to_owned(),
            price_unit_name: fields[15].to_owned(),
            delete_flag: parse_u32_field(path, "package", line, "delete_flag", fields[16])?,
            event_flag: parse_u32_field(path, "package", line, "event_flag", fields[17])?,
            product_amount: parse_u32_field(path, "package", line, "product_amount", fields[18])?,
            product_seq_list: parse_u32_list(fields[19]),
            in_game_package_id: fields[20].to_owned(),
            product_cash_seq: parse_u32_field(
                path,
                "package",
                line,
                "product_cash_seq",
                fields[21],
            )?,
            price_count: parse_u32_field(path, "package", line, "price_count", fields[22])?,
            price_seq_list: parse_u32_list(fields[23]),
            deduct_mileage_flag: parse_u32_field(
                path,
                "package",
                line,
                "deduct_mileage_flag",
                fields[24],
            )? != 0,
            cash_type: parse_u32_field(path, "package", line, "cash_type", fields[25])?,
            cash_type_flag: parse_u32_field(path, "package", line, "cash_type_flag", fields[26])?,
        })
    })
}

pub fn parse_product_file(path: &Utf8Path) -> Result<Vec<ShopProductRecord>, ShopDataError> {
    parse_legacy_lines(path, "product", 17, |path, line, fields| {
        Ok(ShopProductRecord {
            product_seq: parse_u32_field(path, "product", line, "product_seq", fields[0])?,
            product_name: fields[1].to_owned(),
            property_name: fields[2].to_owned(),
            value: fields[3].to_owned(),
            unit_name: fields[4].to_owned(),
            price: parse_u32_field(path, "product", line, "price", fields[5])?,
            price_seq: parse_u32_field(path, "product", line, "price_seq", fields[6])?,
            property_type: parse_u32_field(path, "product", line, "property_type", fields[7])?,
            must_flag: parse_u32_field(path, "product", line, "must_flag", fields[8])?,
            view_order: parse_u32_field(path, "product", line, "view_order", fields[9])?,
            delete_flag: parse_u32_field(path, "product", line, "delete_flag", fields[10])?,
            storage_group: parse_u32_field(path, "product", line, "storage_group", fields[11])?,
            share_flag: parse_u32_field(path, "product", line, "share_flag", fields[12])?,
            in_game_package_id: fields[13].to_owned(),
            property_seq: parse_u32_field(path, "product", line, "property_seq", fields[14])?,
            product_type: parse_u32_field(path, "product", line, "product_type", fields[15])?,
            unit_type: parse_u32_field(path, "product", line, "unit_type", fields[16])?,
        })
    })
}

pub fn parse_banner_file(path: &Utf8Path) -> Result<Vec<ShopBannerRecord>, ShopDataError> {
    parse_legacy_lines(path, "banner", 8, |path, line, fields| {
        Ok(ShopBannerRecord {
            banner_seq: parse_u32_field(path, "banner", line, "banner_seq", fields[0])?,
            banner_name: fields[1].to_owned(),
            banner_image_url: fields[2].to_owned(),
            banner_order: parse_u32_field(path, "banner", line, "banner_order", fields[3])?,
            banner_direction: parse_u32_field(path, "banner", line, "banner_direction", fields[4])?,
            start_date: parse_timestamp_field(path, "banner", line, "start_date", fields[5])?,
            end_date: parse_timestamp_field(path, "banner", line, "end_date", fields[6])?,
            banner_link_url: fields[7].to_owned(),
        })
    })
}

fn parse_legacy_lines<T, F>(
    path: &Utf8Path,
    kind: &'static str,
    expected_fields: usize,
    parse_line: F,
) -> Result<Vec<T>, ShopDataError>
where
    F: Fn(&Utf8Path, usize, &[&str]) -> Result<T, ShopDataError>,
{
    let content = read_legacy_text_file(path)?;
    let mut records = Vec::new();

    for (index, line) in content.lines().enumerate() {
        if line.trim().is_empty() {
            continue;
        }

        let fields = split_legacy_fields(path, kind, index + 1, line, expected_fields)?;
        records.push(parse_line(path, index + 1, &fields)?);
    }

    Ok(records)
}

fn split_legacy_fields<'a>(
    path: &Utf8Path,
    kind: &'static str,
    line: usize,
    raw_line: &'a str,
    expected_fields: usize,
) -> Result<Vec<&'a str>, ShopDataError> {
    let fields: Vec<&str> = raw_line.split('@').collect();

    if fields.len() != expected_fields {
        return Err(ShopDataError::InvalidFieldCount {
            path: path.to_path_buf(),
            kind,
            line,
            expected: expected_fields,
            actual: fields.len(),
        });
    }

    Ok(fields)
}

fn parse_u32_field(
    path: &Utf8Path,
    kind: &'static str,
    line: usize,
    field: &'static str,
    value: &str,
) -> Result<u32, ShopDataError> {
    value
        .trim()
        .parse()
        .map_err(|_| ShopDataError::InvalidInteger {
            path: path.to_path_buf(),
            kind,
            line,
            field,
            value: value.to_owned(),
        })
}

fn parse_timestamp_field(
    path: &Utf8Path,
    kind: &'static str,
    line: usize,
    field: &'static str,
    value: &str,
) -> Result<LegacyTimestamp, ShopDataError> {
    LegacyTimestamp::parse(value).map_err(|_| ShopDataError::InvalidTimestamp {
        path: path.to_path_buf(),
        kind,
        line,
        field,
        value: value.to_owned(),
    })
}

fn parse_u32_list(value: &str) -> Vec<u32> {
    value
        .split('|')
        .filter_map(|token| {
            let token = token.trim();
            if token.is_empty() {
                None
            } else {
                token.parse().ok()
            }
        })
        .collect()
}

fn read_legacy_text_file(path: &Utf8Path) -> Result<String, ShopDataError> {
    let bytes = fs::read(path).map_err(|source| ShopDataError::Io {
        path: path.to_path_buf(),
        source,
    })?;

    Ok(decode_legacy_bytes(&bytes))
}

fn decode_legacy_bytes(bytes: &[u8]) -> String {
    if bytes.starts_with(&[0xFF, 0xFE]) {
        let mut units = Vec::with_capacity(bytes.len() / 2);
        for chunk in bytes[2..].chunks_exact(2) {
            units.push(u16::from_le_bytes([chunk[0], chunk[1]]));
        }
        return String::from_utf16_lossy(&units);
    }

    if bytes.starts_with(&[0xEF, 0xBB, 0xBF]) {
        return String::from_utf8_lossy(&bytes[3..]).into_owned();
    }

    String::from_utf8_lossy(bytes).into_owned()
}

fn find_latest_named_file(
    root: &Utf8Path,
    file_name: &'static str,
) -> Result<Utf8PathBuf, ShopDataError> {
    let mut matches = Vec::new();
    collect_named_files(root, root, file_name, &mut matches)?;
    matches.sort();

    matches
        .pop()
        .map(|path| root.join(path))
        .ok_or_else(|| ShopDataError::MissingFile {
            root: root.to_path_buf(),
            file_name,
        })
}

fn collect_named_files(
    root: &Utf8Path,
    current: &Utf8Path,
    file_name: &'static str,
    matches: &mut Vec<Utf8PathBuf>,
) -> Result<(), ShopDataError> {
    for entry in fs::read_dir(current).map_err(|source| ShopDataError::Io {
        path: current.to_path_buf(),
        source,
    })? {
        let entry = entry.map_err(|source| ShopDataError::Io {
            path: current.to_path_buf(),
            source,
        })?;

        let path = match Utf8PathBuf::from_path_buf(entry.path()) {
            Ok(path) => path,
            Err(_) => continue,
        };

        if path.is_dir() {
            collect_named_files(root, &path, file_name, matches)?;
            continue;
        }

        if path.file_name() == Some(file_name) {
            let relative = path.strip_prefix(root).unwrap_or(&path).to_path_buf();
            matches.push(relative);
        }
    }

    Ok(())
}

impl LegacyTimestamp {
    fn parse(value: &str) -> Result<Self, ()> {
        let digits = value.trim();
        if digits.len() != 8 && digits.len() != 14 {
            return Err(());
        }

        let year = digits[0..4].parse().map_err(|_| ())?;
        let month = digits[4..6].parse().map_err(|_| ())?;
        let day = digits[6..8].parse().map_err(|_| ())?;

        if digits.len() == 8 {
            return Ok(Self {
                year,
                month,
                day,
                hour: None,
                minute: None,
                second: None,
            });
        }

        Ok(Self {
            year,
            month,
            day,
            hour: Some(digits[8..10].parse().map_err(|_| ())?),
            minute: Some(digits[10..12].parse().map_err(|_| ())?),
            second: Some(digits[12..14].parse().map_err(|_| ())?),
        })
    }
}

#[cfg(test)]
mod tests {
    use std::fs::{self, File};

    use camino::Utf8PathBuf;

    use super::{
        find_latest_named_file, load_shop_data, parse_banner_file, parse_category_file,
        parse_package_file, parse_product_file, LegacyTimestamp, ShopBannerRecord,
        ShopCategoryRecord,
    };

    #[test]
    fn parses_legacy_records() {
        let root = test_root();
        let category_path = root.join("IBSCategory.txt");
        let package_path = root.join("IBSPackage.txt");
        let product_path = root.join("IBSProduct.txt");
        let banner_path = root.join("IBSBanner.txt");

        fs::write(&category_path, "10@W Coin (C)@200@201@10@1@1\n").unwrap();
        fs::write(
            &package_path,
            "13@1@413@Gold Channel Ticket@170@190@This is an exclusive ticket to enter Gold Channel.@@182@185@20111109010000@20151109180000@177@1@W Coin(C)@W Coin(C)@181@200@0@528|@6780@2@4@607|608|609|610|@0@508@669\n",
        )
        .unwrap();
        fs::write(
            &product_path,
            "1@testddd@Quantity@test@EA@1000@1@142@145@1@144@673@518@1200@7@138@680\n",
        )
        .unwrap();
        fs::write(
            &banner_path,
            "3@BlueMu GameShop@http://upload.webzen.com/Files/SUN/GameGuide/2011/03/14/20110314VG0RO1MJ5IMOVFML.jpg@1@66@20110314@20120331@http://muonline.webzen.com/News/Notice/Default.aspx?iBS=2120\n",
        )
        .unwrap();

        let categories = parse_category_file(&category_path).unwrap();
        let packages = parse_package_file(&package_path).unwrap();
        let products = parse_product_file(&product_path).unwrap();
        let banners = parse_banner_file(&banner_path).unwrap();

        assert_eq!(
            categories,
            vec![ShopCategoryRecord {
                product_display_seq: 10,
                name: "W Coin (C)".to_owned(),
                event_flag: 200,
                open_flag: 201,
                parent_product_display_seq: 10,
                display_order: 1,
                root: 1,
            }]
        );

        assert_eq!(packages[0].product_seq_list, vec![528]);
        assert_eq!(packages[0].price_seq_list, vec![607, 608, 609, 610]);
        assert_eq!(packages[0].start_date.year, 2011);
        assert_eq!(packages[0].end_date.day, 9);

        assert_eq!(products[0].product_seq, 1);
        assert_eq!(products[0].unit_name, "EA");
        assert_eq!(banners[0].banner_name, "BlueMu GameShop");
        assert_eq!(
            banners[0],
            ShopBannerRecord {
                banner_seq: 3,
                banner_name: "BlueMu GameShop".to_owned(),
                banner_image_url:
                    "http://upload.webzen.com/Files/SUN/GameGuide/2011/03/14/20110314VG0RO1MJ5IMOVFML.jpg"
                        .to_owned(),
                banner_order: 1,
                banner_direction: 66,
                start_date: LegacyTimestamp {
                    year: 2011,
                    month: 3,
                    day: 14,
                    hour: None,
                    minute: None,
                    second: None,
                },
                end_date: LegacyTimestamp {
                    year: 2012,
                    month: 3,
                    day: 31,
                    hour: None,
                    minute: None,
                    second: None,
                },
                banner_link_url:
                    "http://muonline.webzen.com/News/Notice/Default.aspx?iBS=2120".to_owned(),
            }
        );
    }

    #[test]
    fn loads_latest_files_from_versioned_directories() {
        let script_root = test_root().join("scripts");
        let banner_root = test_root().join("banners");
        fs::create_dir_all(script_root.join("512.2012.076")).unwrap();
        fs::create_dir_all(script_root.join("512.2012.084")).unwrap();
        fs::create_dir_all(banner_root.join("583.2010.005")).unwrap();
        fs::create_dir_all(banner_root.join("583.2011.001")).unwrap();

        fs::write(
            script_root.join("512.2012.076").join("IBSCategory.txt"),
            "1@Old@1@1@1@1@1\n",
        )
        .unwrap();
        fs::write(
            script_root.join("512.2012.076").join("IBSPackage.txt"),
            "1@1@1@Old@1@1@@@1@1@20110101000000@20110102000000@1@1@Cash@Unit@1@1@1@1|@Package@1@1@1|@0@0@0\n",
        )
        .unwrap();
        fs::write(
            script_root.join("512.2012.076").join("IBSProduct.txt"),
            "1@Old@Kind@Value@Unit@1@1@1@1@1@1@1@1@Pkg@1@1@1\n",
        )
        .unwrap();
        fs::write(
            script_root.join("512.2012.084").join("IBSCategory.txt"),
            "2@New@2@2@2@2@2\n",
        )
        .unwrap();
        fs::write(
            script_root.join("512.2012.084").join("IBSPackage.txt"),
            "2@2@2@New@2@2@@@2@2@20120101000000@20120102000000@2@2@Cash@Unit@2@2@2@2|@Package@2@2@2|@0@0@0\n",
        )
        .unwrap();
        fs::write(
            script_root.join("512.2012.084").join("IBSProduct.txt"),
            "2@New@Kind@Value@Unit@2@2@2@2@2@2@2@2@Pkg@2@2@2\n",
        )
        .unwrap();
        fs::write(
            banner_root.join("583.2010.005").join("IBSBanner.txt"),
            "1@Old@http://old.example/banner.jpg@1@1@20100101@20110101@http://old.example\n",
        )
        .unwrap();
        fs::write(
            banner_root.join("583.2011.001").join("IBSBanner.txt"),
            "3@BlueMu GameShop@http://upload.webzen.com/Files/SUN/GameGuide/2011/03/14/20110314VG0RO1MJ5IMOVFML.jpg@1@66@20110314@20120331@http://muonline.webzen.com/News/Notice/Default.aspx?iBS=2120\n",
        )
        .unwrap();

        let report = load_shop_data(&script_root, &banner_root).unwrap();

        assert!(report
            .category_file
            .ends_with("512.2012.084/IBSCategory.txt"));
        assert!(report.package_file.ends_with("512.2012.084/IBSPackage.txt"));
        assert!(report.product_file.ends_with("512.2012.084/IBSProduct.txt"));
        assert!(report.banner_file.ends_with("583.2011.001/IBSBanner.txt"));
        assert_eq!(report.categories[0].name, "New");
        assert_eq!(report.banners[0].banner_name, "BlueMu GameShop");
    }

    #[test]
    fn finds_latest_file_by_name() {
        let root = test_root();
        fs::create_dir_all(root.join("512.2012.076")).unwrap();
        fs::create_dir_all(root.join("512.2012.084")).unwrap();
        File::create(root.join("512.2012.076").join("IBSCategory.txt")).unwrap();
        File::create(root.join("512.2012.084").join("IBSCategory.txt")).unwrap();

        let found = find_latest_named_file(&root, "IBSCategory.txt").unwrap();
        assert!(found.ends_with("512.2012.084/IBSCategory.txt"));
    }

    fn test_root() -> Utf8PathBuf {
        let root = std::env::temp_dir().join(format!(
            "mu_shop_data_{}_{}",
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
}
