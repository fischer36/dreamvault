use std::io::{Read, Write};

pub static mut ADDRESS: Option<String> = None;
pub fn send(request: &[u8]) -> String {
    let addy: String;
    unsafe {
        if ADDRESS.is_none() {
            ADDRESS = format!("localhost:{}", read_port()).into();
        }
        addy = ADDRESS.clone().unwrap();
    }
    let mut stream = std::net::TcpStream::connect(addy).unwrap();
    let result = stream.write_all(request);

    let mut response = String::new();
    if stream.read_to_string(&mut response).is_ok() {
        // println!("Response: {}", response);
    }
    return response;
}

pub fn parse(request_line: String, headers: String, body: String) -> String {
    return format!("{}\r\n{}\r\n\r\n{}\r\n", request_line, headers, body);
}

pub fn read_port() -> i32 {
    let result = std::fs::read_to_string("../server/src/config.h");
    let pos = result.as_ref().unwrap().find("#define SERVER_PORT");
    let ok = result.unwrap().split_off(pos.unwrap() + 20);
    let my_int = &ok[0..ok.len() - 1].parse::<i32>().unwrap();
    print!("{}", *my_int);
    return *my_int;
}
