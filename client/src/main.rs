mod commands;
mod requests;
mod sys;
#[cfg(test)]
mod tests;

pub mod vault;
pub const EMAIL: &str = "4ssample@gmail.com";
pub const PASSWORD: &str = "2samplePass123";

fn main() {
    commands::register(EMAIL, PASSWORD);
    let (token, user_id) = commands::login(EMAIL, PASSWORD);
    commands::get_pages(&token, user_id);
    // let mut vault = vault::Vault::load().unwrap();
    // vault.push();
    //
    // commands::query::view_vault(&vault);
    // vault.scan();
    // vault.save();
    return;
}
