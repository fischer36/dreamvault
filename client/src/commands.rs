use crate::requests::ADDRESS;
use std::io::{Read, Write};

use crate::requests;

pub fn register(email: &str, password: &str) {
    let method = "POST";
    let uri = "/register";

    let token = "d7118ed59ab3b8f263dbdd54596a7d83";
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

pub fn login(email: &str, password: &str) -> String {
    let auth_path = ".auth";
    if let Ok(mut file) = File::open(auth_path) {
        let mut contents = String::new();
        file.read_to_string(&mut contents).unwrap();

        let parts: Vec<&str> = contents.split(',').collect();
        if parts.len() == 2 {
            let token = parts[0];
            let timestamp = parts[1].parse::<u64>().unwrap_or(0);

            println!("token {}", token);
            let current_time = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .unwrap()
                .as_secs();
            if current_time - timestamp < 24 * 3600 {
                println!("Token valid per time");
                return token.to_string();
            }

            println!("Token not valid per time");
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
        email.to_string(),
        password.to_string()
    );
    println!("ok");

    let response = requests::send(request.as_bytes());

    let token_pos = &response.find("Token:").unwrap();
    let token = &response[token_pos + 7..token_pos + 41];

    let current_time = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_secs();
    let new_contents = format!("{},{}", token, current_time);
    fs::write(auth_path, new_contents).unwrap();
    return token.to_string();
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

pub fn create_page(token: &str) {
    let title = "Ok new titlej";
    let body = "Ok new body";

    let user_id = get_user_id(token);
    let method = "POST";
    let uri = format!("/users/{}/pages", user_id);
    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Authentication: {}\r\n
                \r\n
                page_title: {} \r\n
                page_body: {} \r\n",
        method, uri, token, title, body
    );

    let response = requests::send(request.as_bytes());
}

pub fn get_page() {
    let page_id = 72;
    let user_id = 35;
    let token = "d7118ed59ab3b8f263dbdd54596a7d83";
    let method = "GET";
    let uri = format!("/users/{}/pages/{}", user_id, page_id);

    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Host: localhost\r\n
                Authentication: {}\r\n
                \r\n",
        method, uri, token
    );

    let response = requests::send(request.as_bytes());
}

pub fn get_pages(token: &str) {
    let user_id = get_user_id(token);

    let method = "GET";
    let uri = format!("/users/{}/pages", user_id);

    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Authentication: {}\r\n
                \r\n",
        method, uri, token,
    );

    let response = requests::send(request.as_bytes());
}

pub fn sync_page() {
    let page_title = "Ok new titlej";
    let page_id = 72;
    let page_content = "jaha";
    let token = "d7118ed59ab3b8f263dbdd54596a7d83";
    let user_id = 35;
    let method = "PATCH";
    let uri = format!("/users/{}/pages/{}", user_id, page_id);
    let title = "Ok new titlej";
    let body = "Ok new body";
    let request: String = format!(
        "{} {} HTTP/1.1\r\n
                Host: localhost\r\n
                Authentication: {}\r\n
                \r\n
                page_title: {} \r\n
                page_body: {} \r\n",
        method, uri, token, title, body
    );

    let response = requests::send(request.as_bytes());
}
