#![allow(dead_code)]

use std::io::{Read, Write};

use crate::commands;
use crate::requests;
use crate::sys;
pub fn register(email: &str, password: &str) {
    let method = "POST";
    let uri = "/register";

    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Host: localhost\r\n
                \r\n
                email: {}\r\n
                password: {}\r\n",
        method,
        uri,
        email.to_string(),
        password.to_string()
    );

    let response = requests::send(request.as_bytes());
    // println!("{}", response);
}

pub fn unregister(token: &str) {
    let method = "POST";
    let uri = "/unregister";

    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Host: localhost\r\n
                Authentication: {}\r\n
                \r\n",
        method, uri, token
    );

    let response = requests::send(request.as_bytes());
}
use std::fs::{self, File};
use std::time::{SystemTime, UNIX_EPOCH};

pub fn login(email: &str, password: &str) -> (String, u32) {
    let auth_path = ".auth";
    if let Ok(mut file) = File::open(auth_path) {
        let last_modified = file
            .metadata()
            .unwrap()
            .modified()
            .unwrap()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs();

        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs();

        let seconds_in_day = 86400;

        if now - last_modified < seconds_in_day {
            let mut contents = String::new();
            file.read_to_string(&mut contents).unwrap();

            let parts: Vec<&str> = contents.split(':').collect();
            if parts.len() == 2 {
                let token = parts[0];
                let user_id = parts[1].parse::<u32>().unwrap_or(0);

                // println!("token {}", token);
                // println!("user_id {}", user_id);
                return (token.to_string(), user_id);
            }
        } else {
            println!("Token is expired");
            sys::remove_file(".auth");
        }
    }
    let method = "POST";
    let uri = "/login";

    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Host: localhost\r\n
                \r\n
                email: {}\r\n
                password: {}\r\n",
        method,
        uri,
        crate::EMAIL,
        crate::PASSWORD
    );

    let response = requests::send(request.as_bytes());

    let token_pos = &response.find("Token:").unwrap();
    let token = &response[token_pos + 7..token_pos + 39];

    let user_pos = response
        .find("User_id:")
        .expect("User_id: not found in response");
    let user_pos_end = response[user_pos..]
        .find("\r\n")
        .expect("End of line not found after User_id:")
        + user_pos;
    let user_id = &response[user_pos + 8..user_pos_end];
    // println!("token {}", token);
    // println!("user_id {}", user_id);

    let new_contents = format!("{}:{}", token, user_id);
    fs::write(auth_path, new_contents).unwrap();
    return (token.to_string(), user_id.parse::<u32>().unwrap());
}

pub fn logout(token: &str) {
    let method = "POST";
    let uri = "/logout";

    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Host: localhost\r\n
                Authentication: {}\r\n
                \r\n",
        method, uri, token
    );

    let response = requests::send(request.as_bytes());
}

pub fn get_user_id(token: &str) -> u32 {
    let method = "GET";
    let uri = "/users";
    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Host: localhost\r\n
                Authentication: {}\r\n
                \r\n",
        method, uri, token
    );

    println!("{}", request);

    let response = requests::send(request.as_bytes());

    let user_id = &response.find("User_id:").unwrap();
    let user_id_str = &response[user_id + 9..response.len() - 2];

    println!("{}", user_id_str);
    return user_id_str.parse::<u32>().unwrap();
}

pub fn create_page(title: &str, content: &str, modified: u64) -> u64 {
    let (token, user_id) = commands::login(crate::EMAIL, crate::PASSWORD);
    let method = "POST";
    let uri = format!("/users/{}/pages", user_id);

    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Authentication: {}\r\n
                \r\n
                page_title: {} \r\n
                page_body: {} \r\n
                modified: {}\r\n",
        method, uri, token, title, content, modified
    );
    println!("request - {}", request);

    let response = requests::send(request.as_bytes());
    println!("response - {}", response);

    let page_id = &response[response.find("page_id: ").unwrap() + 9..];

    return page_id.parse::<u64>().unwrap();
}

pub fn get_page(page: crate::vault::Page) -> String {
    let (token, user_id) = commands::login(crate::EMAIL, crate::PASSWORD);
    let method = "GET";
    let uri = format!("/users/{}/pages/{}", user_id, page.id);

    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Host: localhost\r\n
                Authentication: {}\r\n
                \r\n",
        method, uri, token
    );

    let response = requests::send(request.as_bytes());

    println!("Response: {}", response);
    response.find("Page_content: ").unwrap();

    let page_content = &response[response.find("Page_content: ").unwrap() + 15..];
    return page_content.to_string();
}

pub fn get_pages(token: &str, user_id: u32) -> Vec<(u32, u64)> {
    let method = "GET";
    let uri = format!("/users/{}/pages", user_id);

    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Authentication: {}\r\n
                \r\n",
        method, uri, token,
    );
    let response = requests::send(request.as_bytes());

    println!("Response: {}", response);

    let mut pages = vec![];
    for line in response.lines() {
        if line.starts_with("id: ") {
            let page = line;
            let parts: Vec<&str> = page.split(", ").collect();
            // Extract the id part and parse it as u32
            let id_str = parts[0].split(": ").nth(1).unwrap();
            let id: u32 = id_str.parse().unwrap();

            // Extract the modified part and parse it as u64
            let modified_str = parts[1].split(": ").nth(1).unwrap();
            let modified: u64 = modified_str.parse().unwrap();

            pages.push((id, modified));
        }
    }
    println!("Pages len: {:?}", pages.len());
    return pages;
}

pub mod query {
    use crate::requests::read_port;

    pub fn view_vault(vault: &crate::vault::Vault) {
        let (token, user_id) = crate::commands::login(crate::EMAIL, crate::PASSWORD);
        let port = read_port();
        println!("Server Address: localhost:");
        println!("");
        println!("Name: {} ", vault.name);
        println!("User id: {} Token: {} ", user_id, token);

        println!("Pages ({}): ", vault.pages.len());

        let curr = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_secs();

        for page in vault.pages.iter() {
            let last = (curr - page.modified) / 60 / 60;
            let synced = if page.id == 0 { "✖" } else { "✔" };

            println!(
                "Id: {} Modified: {}-hours ago Synced: {} Path: {}",
                page.id, last, synced, page.path
            );
        }

        println!("");
    }
}
