/*
 * TODO :
 * - Push pages, create a new page for each id which is 0, then update responsed id.
 * */

#![allow(dead_code)]
use crate::{
    commands::{self, get_page},
    sys::{self, modified, page_data},
};
use std::{
    io::{Read, Write},
    path::PathBuf,
};

#[derive(Debug, serde::Serialize, serde::Deserialize)]
pub struct Vault {
    pub name: String,
    pub pages: Vec<Page>,
}

#[derive(Debug, serde::Serialize, serde::Deserialize, Clone)]
pub struct Page {
    pub id: u64,       // ID recieved from server
    pub modified: u64, // Last modified in seconds
    pub path: String,  // Corresponding file path
}

impl Vault {
    pub fn create() -> Result<Vault, ()> {
        if sys::path_exists(".vault/") == true {
            return Err(());
        }

        let test_vault_path = "./.vault";

        std::fs::create_dir(test_vault_path).expect("Vault folder already exists");

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
                        .expect("metadata")
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
    pub fn pull(&mut self) -> Result<(), ()> {
        let (token, user_id) = commands::login(crate::EMAIL, crate::PASSWORD);
        let pages: Vec<(u32, u64)> = commands::get_pages(&token, user_id);

        println!("Page count: {:?}", self.pages.len());
        for local_page in self.pages.iter_mut() {
            println!(
                "Local Page - id: {:?} modified: {:?}",
                local_page.id, local_page.modified
            );
            for page in pages.iter() {
                if page.0 as u64 == local_page.id {
                    if page.1 > local_page.modified {
                        println!("Pulling page: {:?}", local_page.path);
                        let new_page_content = commands::get_page(local_page.clone());
                        sys::write_file(&local_page.path, &new_page_content);
                        local_page.modified = page.1;
                    }
                }
            }
        }
        return Ok(());
    }

    pub fn push(&mut self) -> Result<(), ()> {
        let (token, user_id) = commands::login(crate::EMAIL, crate::PASSWORD);
        let pages: Vec<(u32, u64)> = commands::get_pages(&token, user_id);

        for local_page in self.pages.iter_mut() {
            if local_page.id == 0 {
                println!("Creating new page: {:?}", local_page.path);
                let title = &local_page.path.to_string()[(local_page.path.find('/').unwrap() + 1)
                    ..(local_page.path.to_string().len() - 3)];

                let page_id = commands::create_page(
                    title,
                    &page_data(&local_page.path).unwrap(),
                    local_page.modified,
                );
                local_page.id = page_id;
            } else {
                for page in pages.iter() {
                    if page.0 as u64 == local_page.id {
                        if page.1 < local_page.modified {
                            println!("Pushing page: {:?}", local_page.path);
                            let new_page_content = commands::get_page(local_page.clone());
                            sys::write_file(&local_page.path, &new_page_content);
                            local_page.modified = page.1;
                        } else {
                            println!("Page is already updated: {:?}", local_page.path);
                        }
                    }
                }
            }
        }
        return Ok(());
    }

    pub fn sync(&mut self) -> Result<(), ()> {
        for page in self.pages.iter_mut() {}
        self.pull()?;
        self.push()?;
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
            self.pages.push(Page {
                id: 0,
                modified: sys::modified(&file.display().to_string()),
                path: file.display().to_string(),
            });
        }

        // println!("New pages: {:?}", new_pages);
        return Ok(());
    }
}
/*
           for page in pages.iter() {
               if page.0 as u64 == local_page.id {
                   println!("\n");
                   if page.1 > local_page.modified {
                       println!("Updating page: {:?}", local_page.path);
                       let new_page_content = commands::get_page(local_page.clone());
                       sys::write_file(&local_page.path, &new_page_content);
                       local_page.modified = page.1;
                   } else if page.1 < local_page.modified {

                       println!("Need update Page: {:?}", local_page.path);
                   } else if page.1 as u64 == local_page.modified {
                       println!("Page updated already: {:?}", local_page.path);
                   }
                   println!("\n");
               }
           }
           //
* sys*/
