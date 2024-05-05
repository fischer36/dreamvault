use commands::{create_page, get_pages};

// mod main_client;
mod commands;
mod requests;
pub mod vault;
fn main() {
    // commandsxd::register("ssample@gmail.com", "samplePass123");
    // let token = commands::login("ssample@gmail.com", "samplePass123");
    // let vault = vault::Vault::create();
    let vault = vault::Vault::load();

    println!("Hello, world!{:?}", vault);
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
