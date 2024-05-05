/*
 * TODO :
 * - Push pages, create a new page for each id which is 0, then update responsed id.
 * */

#![allow(dead_code)]
use crate::{
    commands::{self, get_page},
    sys::{self, modified},
};
use std::{
    io::{Read, Write},
    path::PathBuf,
};

#[derive(Debug, serde::Serialize, serde::Deserialize)]
pub struct Vault {
    name: String,
    pages: Vec<Page>,
}

#[derive(Debug, serde::Serialize, serde::Deserialize)]
pub struct Page {
    id: u64,       // ID recieved from server
    modified: u64, // Last modified in seconds
    path: String,  // Corresponding file path
}

impl Page {
    fn new(path: &str) -> Page {
        return Page {
            id: 0,
            modified: sys::modified(path),
            path: path.to_string(),
        };
    }
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
                        .duration_since(std::time::UNIX_EPOCH)
                        .unwrap()
                        .as_secs(),
                    path: file.as_ref().unwrap().path().display().to_string(),
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
            pages: pages,
        };

        let mut ok = std::fs::File::create(format!("{}/vault.yaml", test_vault_path)).unwrap();
        let serialized = serde_yaml::to_string(&vault).unwrap();
        println!("serialized: {:?}", serialized);
        ok.write(serialized.as_bytes()).unwrap();
        return Ok(vault);
    }

    pub fn load() -> Result<Vault, ()> {
        let vault_yaml = std::fs::read_to_string("./.vault/vault.yaml");

        if vault_yaml.is_err() {
            return Err(());
        }
        let vault: Vault = serde_yaml::from_str(&vault_yaml.unwrap()).unwrap();
        // println!("Loaded vault: {:?}", vault);
        return Ok(vault);
    }

    pub fn save(&self) -> Result<(), ()> {
        let serialized = serde_yaml::to_string(&self).unwrap();
        std::fs::write("./.vault/vault.yaml", serialized).unwrap();
        // println!("Saved vault: {:?}", self);
        return Ok(());
    }
    fn pull() -> Result<(), ()> {
        return Ok(());
    }

    fn push(&self) -> Result<(), ()> {
        for page in self.pages.iter() {
            if page.id == 0 {
                println!("New page: {:?}", page);
                // commands::create_page(token)
            }
        }
        return Ok(());
    }

    pub fn scan(&mut self) -> Result<(), ()> {
        for saved_page in self.pages.iter_mut() {
            if saved_page.modified < sys::modified(saved_page.path.as_str()) {
                println!("Updating page: {:?}", saved_page.path);
                saved_page.modified = sys::modified(&saved_page.path);
            }
        }

        let new_pages: Vec<PathBuf> = sys::get_md_files()
            .into_iter()
            .filter(|path_buf| {
                !self
                    .pages
                    .iter()
                    .any(|p| p.path == path_buf.display().to_string())
            })
            .collect();

        for file in new_pages.iter() {
            self.pages.push(Page::new(&file.display().to_string()));
        }

        // println!("New pages: {:?}", new_pages);
        return Ok(());
    }
}
