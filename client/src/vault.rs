use std::io::{Read, Write};
#[derive(Debug, serde::Serialize, serde::Deserialize)]
pub struct Vault {
    name: String,
    version: u32,
    pages: Vec<Page>,
}

#[derive(Debug, serde::Serialize, serde::Deserialize)]
pub struct Page {
    id: u64,        // ID recieved from server
    modified: u128, // Last modified in seconds
    path: String,   // Corresponding file path
    version: i32,   // Version number
}
impl Vault {
    pub fn create() -> Result<Vault, ()> {
        let test_vault_path = "./.vault";

        if std::fs::create_dir(test_vault_path).is_err() {
            // return Err("Vault folder already exists".to_string());
            return Err(());
        }

        let mut pages = vec![];

        for file in std::fs::read_dir(".").unwrap().into_iter() {
            if file.as_ref().is_ok_and(|entry| {
                entry.path().extension().and_then(std::ffi::OsStr::to_str) == Some("md")
            }) {
                let page = Page {
                    id: 0,
                    modified: file
                        .as_ref()
                        .unwrap()
                        .path()
                        .metadata()
                        .unwrap()
                        .modified()
                        .unwrap()
                        .elapsed()
                        .unwrap()
                        .as_secs() as u128,
                    path: file.as_ref().unwrap().path().display().to_string(),
                    version: 0,
                };

                pages.push(page);
            }
        }
        println!("Page count: {:?}", pages.len());
        for page in pages.iter() {
            println!("Page - modified: {:?} path: {:?}", page.modified, page.path);
        }
        let vault_name = "test";
        let vault = Vault {
            name: vault_name.to_string(),
            version: 0,
            pages: pages,
        };

        let mut ok = std::fs::File::create(format!("{}/vault.yaml", test_vault_path)).unwrap();
        let serialized = serde_yaml::to_string(&vault).unwrap();
        println!("serialized: {:?}", serialized);
        ok.write(serialized.as_bytes());
        return Ok(vault);
    }

    pub fn load() -> Result<Vault, ()> {
        let vault_yaml = std::fs::read_to_string("./.vault/vault.yaml").unwrap();
        let vault: Vault = serde_yaml::from_str(&vault_yaml).unwrap();
        println!("Loaded vault: {:?}", vault);
        return Ok(vault);
    }

    fn save() -> Result<(), ()> {
        return Ok(());
    }
    fn pull() -> Result<(), ()> {
        return Ok(());
    }
    fn push() -> Result<(), ()> {
        return Ok(());
    }

    fn sync() -> Result<(), ()> {
        return Ok(());
    }
}
