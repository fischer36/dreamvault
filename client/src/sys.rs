use std::fs::read_dir;
use std::path::PathBuf;

pub fn get_md_files() -> Vec<PathBuf> {
    let md_files: Vec<PathBuf> = read_dir(".")
        .unwrap()
        .filter_map(|entry| {
            let path = entry.ok()?.path();
            if path.extension()?.to_str()? == "md" {
                Some(path)
            } else {
                None
            }
        })
        .collect();
    return md_files;
}

pub fn page_data(path: &str) -> Result<String, ()> {
    std::fs::read_to_string(path).map_err(|_| ())
}
pub fn modified(path: &str) -> u64 {
    let result = std::fs::metadata(path);
    assert!(result.is_ok());

    let modified = result
        .unwrap()
        .modified()
        .unwrap()
        .duration_since(std::time::UNIX_EPOCH)
        .unwrap()
        .as_secs();

    // println!("{:?}", modified);
    return modified;
}
