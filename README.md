# Dreamvault

- Server: Sets up a TCP listener configured in config.h
- Api: Interface between the TCP listener and back-end code.
- http-parser: Parses the raw bytes into an HTTP-request struct.
- Worker: Given a task from api will call the appropriate functions
- Db: Handles MySQL database interaction
- Sys: Queries the files.
- Util: Hashing, compares hash and generates token.

# Dependencies:
- OpenSSL for hashing and generating tokens and MySQL for database interaction.

# TODO
- [x] Port to Linux (sys and the removal of the libs folder).
- [ ] Client for making requests in Rust.
- [X] Remove Sodium dependency.
- [X] Revamp worker.c.
- [X] task_handler and it's called task functions need to fill out the http response based on the task status.
- [ ] Fix Namings, consistent and simple.
- [X] De-spaghettify (maybe).
- [X] Makefile.
- [ ] Makefile cross-platform and release build.
- [ ] Integrate docker.
- [ ] Add SSL.
- [ ] SQL database documentation.
- [ ] Finalize and start using.

## API & Worker changes:
- Tasks User
    - login [x] 
       - request: POST /login 
       - response: (Body) Token: <token>
    - logout [x]
       - request: POST /logout (Body)Token: <token>
    - register[x]
       - request: POST /register 
    - unregister [x]
       - request: POST /unregister (Body)Token: <token> 
- Tasks Page
    - read [x]
       - request: GET /user/<page_id>/page/<page_id>
       - response: (Body) (raw page_data)
    - create [x]
       - request: POST /user/<page_id>/page/<page_id> (Body) Title: title Content: content
       - response: (Body) page_id: <page_id>
    - write [x]
       - request: PATCH /user/<page_id>/page/<page_id>
    - delete [x]
       - request: DELETE /user/<page_id>/page/<page_id>

- HTTP-parser 
    - [ ] need a way to sort request based on URI aswell as extracting URI information such as user_id, page_id, vaultid. Should probably just count number of slashes in URI to determine the task.
- Client 
    - [ ] Make use of the API functions and make it usable.
