FROM ubuntu:latest

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y build-essential libmysqlclient-dev gcc make libssl-dev


# Copy your server source code into the container
COPY ./server /usr/src/myapp

# Set the working directory
WORKDIR /usr/src/myapp

# Build your C program using the Makefile
RUN make

# Command to run the application
CMD ["./server"]
