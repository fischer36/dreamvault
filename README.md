# Dreamvault

- Server: Sets up a TCP listener configured in config.h
- Api: Interface between the TCP listener and back-end code.
- http-parser: Parses the raw bytes into an HTTP-request struct.
- Worker: Given a task from api will call the appropriate functions
- Db: Handles MySQL database interaction
- Sys: Queries the files.
- Util: Hashing, compares hash and generates token.

# Dependencies:
- OpenSSL for hashing and generating tokens.

# TODO
- [x] Port to Linux (sys and the removal of the libs folder).
- [ ] Client for making requests in Rust.
- [X] Remove Sodium dependency.
- [ ] Revamp worker.c.
- [ ] task_handler and it's called task functions need to fill out the http response based on the task status.
- [ ] Fix Namings, consistent and simple.
- [ ] De-spaghettify (maybe).
- [ ] Integrate docker.
- [ ] Add SSL.
- [ ] SQL database documentation.
- [ ] Finalize and start using.


