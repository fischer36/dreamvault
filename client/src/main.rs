use commands::{create_page, get_pages};

#[cfg(test)]
mod tests;
// mod main_client;
mod commands;
mod requests;
mod sys;

pub mod vault;
pub const EMAIL: &str = "ssample@gmail.com";
pub const PASSWORD: &str = "samplePass123";
fn main() {
    // commandsxd::register("ssample@gmail.com", "samplePass123");
    // let token = commands::login("ssample@gmail.com", "samplePass123");
    // let vault = vault::Vault::create();
    // let mut vault = vault::Vault::load().unwrap();
    // vault.scan();
    // vault.save();
    // let xd = sys::page_data("./sample.md");
    // println!("xd: {:?}", xd.unwrap());
    let response = commands::login(EMAIL, PASSWORD);

    // println!("Hello, world!{:?}", vault);
    // commandsxd::logout(&token);
    //
    // create_page(&token);
    //
    // get_pages(token.as_str());
    // let token = "1af8e96bd6225004ab131ee6ff7f34d8";
    // requests::get_page();
    // let user_id = requests::get_user_id(&token);
    //
    // print!("HAHHAuser_id: {}", user_id);
    // // let token =
    //requests::login("ssample@gmail.com", "samplePass123");
    // requests::unregister(&token);
}
