FROM ubuntu:latest

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install dependencies
RUN apt-get update && \
    apt-get install -y build-essential libmysqlclient-dev gcc make libssl-dev mysql-server mysql-client && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Copy server source code into the container
COPY ./server /usr/src/myapp
COPY ./client /usr/src/client

# Set the working directory
WORKDIR /usr/src/myapp

# Build C server using the Makefile
RUN make

# Command to run the MySQL server and server
CMD service mysql start && ./server
