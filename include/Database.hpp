#pragma once
#include <sqlite3.h>
#include <string>
#include <stdexcept>
#include <Logger.hpp>

class Database {
private:
    sqlite3 *db;
public:
    explicit Database(const std::string &path) : db{} {
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Failed to open database");
        }
        const char *sql = "create table if not exists users (username TEXT PRIMARY KEY, password TEXT);";
        char *errmsg;

        if (sqlite3_exec(db, sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
            std::string err(errmsg);
            sqlite3_free(errmsg);
            throw std::runtime_error("Failed to create table: " + err);
        }
    }
    ~Database() {
        sqlite3_close(db);
    }

    bool userExit(const std::string& username) {
        std::string sql = "select * from users where username = ?;";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            LOG_INFO("Failed to prepare query user SQL for user: %s" , username.c_str());
            return false;
        }
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc == SQLITE_ROW) {
            // 用户存在
            return true;
        } else if (rc == SQLITE_DONE) {
            // 用户不存在
            return false;
        } else {
            // 查询出错
            return false;
        }
    }
    bool registerUser(const std::string& username, const std::string& password) {
        if (userExit(username)) {
            LOG_INFO("Registration failed for user: %s already exist ", username.c_str());
            return false;
        }
        std::string sql = "INSERT INTO users (username, password) VALUES (?, ?);";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            LOG_INFO("Failed to prepare registration SQL for user: %s" , username.c_str()); // 记录日志
            return false;
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            LOG_INFO("Registration failed for user: %s " , username.c_str());
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
        LOG_INFO("User registered: %s with password: %s" , username.c_str(), password.c_str()); // 记录日志
        return true;
    }

    bool loginUser(const std::string& username, const std::string& password) {
        std::string sql = "select password from users where username = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            LOG_INFO("Failed to prepare login SQL for user: %s" , username.c_str());
            return false;
        }
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            LOG_INFO("User not found: %s" , username.c_str());
            sqlite3_finalize(stmt);
            return false;
        }

        const char* stored_password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        std::string password_str(stored_password, sqlite3_column_bytes(stmt, 0));

        sqlite3_finalize(stmt);

        if (stored_password == nullptr or password_str != password) {
            LOG_INFO("Login failed for user: %s password:%s stored password is %s" ,username.c_str() ,password.c_str(), password_str.c_str());
            return false;
        }

        LOG_INFO("User logged in: %s" , username.c_str());
        return true;
    }
};