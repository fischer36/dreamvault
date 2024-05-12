mod commands;
mod requests;
mod sys;
#[cfg(test)]
mod tests;

pub mod vault;
pub const EMAIL: &str = "ssample@gmail.com";
pub const PASSWORD: &str = "samplePass123";

fn main() {
    let mut vault = vault::Vault::load().unwrap();
    commands::query::view_vault(&vault);
    vault.scan();
    vault.save();
    return;
}
