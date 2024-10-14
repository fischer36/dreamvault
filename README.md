# Dreamvault
Dreamvault is a cross-platform backend web service—**written in pure C**—for managing users, sessions, vaults, and markdown-based notes. It exposes a RESTful HTTP API for handling user authentication, session management, and markdown content storage. The system uses MySQL for storing user data and metadata, and it interacts with the filesystem to handle vault and page content.
## Features
- Server API: Handles HTTP and RESTful requests.
- User management: Registration, login, logout, and deletion.
- Session management: Token-based session authentication.
- Vault and page management: CRUD (Create, Read, Update, Delete) operations for vaults and pages.
- Filesystem interaction: Manages user-created content stored in markdown format.
## Dependencies:
- mysql-client, mysql-server & libmysqlclient-dev
- libssl-dev 
- build-essential, gcc & make
## License
This project is licensed under the Apache License 2.0. See the **LICENSE** file for details.
