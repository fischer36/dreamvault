-- init.sql

-- Create the database if it does not exist
CREATE DATABASE IF NOT EXISTS dreamvault;

-- Use the created database
USE dreamvault;

-- Drop and create the tables

-- DROP TABLE IF EXISTS `users`;
CREATE TABLE `users` (
  `user_id` int NOT NULL AUTO_INCREMENT,
  `email` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `creation_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `storage_limit` bigint NOT NULL,
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `email` (`email`)
) ENGINE=InnoDB AUTO_INCREMENT=52 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
-- DROP TABLE IF EXISTS `pages`;
CREATE TABLE `pages` (
  `page_id` int NOT NULL AUTO_INCREMENT,
  `owner_id` int NOT NULL,
  `vault_id` int NOT NULL DEFAULT '0',
  `file_path` varchar(255) NOT NULL,
  `file_size` bigint NOT NULL DEFAULT '0',
  `creation_date` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `last_modified` bigint unsigned NOT NULL,
  `title` varchar(255) NOT NULL,
  PRIMARY KEY (`page_id`),
  UNIQUE KEY `file_path` (`file_path`),
  KEY `owner_id` (`owner_id`),
  KEY `vault_id` (`vault_id`),
  CONSTRAINT `owner_fk` FOREIGN KEY (`owner_id`) REFERENCES `users` (`user_id`)
) ENGINE=InnoDB AUTO_INCREMENT=117 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- DROP TABLE IF EXISTS `sessions`;
CREATE TABLE `sessions` (
  `user_id` int NOT NULL,
  `token` varchar(32) NOT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  UNIQUE KEY `unique_token` (`token`),
  UNIQUE KEY `unique_user_id` (`user_id`),
  CONSTRAINT `fk_user_id` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;


-- DROP TABLE IF EXISTS `vaults`;
CREATE TABLE `vaults` (
  `vault_id` int NOT NULL AUTO_INCREMENT,
  `owner_id` int NOT NULL,
  `name` varchar(255) NOT NULL,
  PRIMARY KEY (`vault_id`),
  KEY `owner_id` (`owner_id`),
  CONSTRAINT `vaults_ibfk_1` FOREIGN KEY (`owner_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=32 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
