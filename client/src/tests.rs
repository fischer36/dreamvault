use std::fs::read_dir;
use std::path::PathBuf;

#[test]
fn get_md() {
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

    for file in md_files.iter() {
        println!("{:?}", file);
    }
}

#[test]
fn modified() {
    let result = std::fs::metadata("./sample.md");
    assert!(result.is_ok());

    let modified = result
        .unwrap()
        .modified()
        .unwrap()
        .duration_since(std::time::UNIX_EPOCH)
        .unwrap()
        .as_secs() as u128;

    println!("{:?}", modified);
}

#[test]
pub fn remove_file() {
    let path = ".auth";
    assert!(std::fs::File::create_new(".auth").is_ok());
    println!("File created");

    assert!(std::fs::remove_file(path).is_ok());
    println!("File deleted");
}
