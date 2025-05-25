#include <sys/socket.h>
#include <map>
#include <string>
#include <cstring>
#include <functional>
#include <netinet/in.h>
#include <unistd.h>
#include <Logger.hpp>
#include <Database.hpp>
#include <iostream>

#define PORT 80

using RequestHandle = std::function<std::string(const std::string&)>;
std::map<std::string, RequestHandle> get_routes;
std::map<std::string, RequestHandle> post_routes;
Database db("users.db");

std::map<std::string, std::string> parseFormBody(const std::string& body) {
    std::map<std::string, std::string> params;
    std::stringstream ss(body);

    std::string s;
    while (getline(ss, s, '&')) {
        std::size_t pos = s.find('=');
        if (pos != std::string::npos) {
            std::string key = s.substr(0, pos);
            std::string value = s.substr(pos + 1);
            params[key] = value;

            LOG_INFO("Parsed key-value pair: %s = %s" , key.c_str(), value.c_str());  // 记录每个解析出的键值对
        } else {
            // 错误处理：找不到 '=' 分隔符
            std::string error_msg = "Error parsing: " + s;
            LOG_ERROR(error_msg.c_str());  // 记录错误信息
            std::cerr << error_msg << std::endl;
        }
    }
    return params;
}
void setupRoutes() {
    get_routes["/"] = [](const std::string& request) {
        return "Hello, World!";
    };
    get_routes["/register"] = [](const std::string& request) {
        // TODO: 实现用户注册逻辑
        return "Please use POST to register";
    };

    get_routes["/login"] = [](const std::string& request) {
        // TODO: 实现用户登录逻辑

        return "Please use POST to login";
    };

    // POST请求处理
    post_routes["/register"] = [](const std::string& request) {
        // TODO: 实现用户注册逻辑
        auto params = parseFormBody(request);
        std::string username = params["username"];
        std::string password = params["password"];

        if (db.registerUser(username, password)) {
            return "Register Success!";
        } else {
            return "Register Failed!";
        }
    };
    post_routes["/login"] = [](const std::string& request) {
        // TODO: 实现用户登录逻辑
        auto params = parseFormBody(request);
        std::string username = params["username"];
        std::string password = params["password"];

        if (db.loginUser(username, password)) {
            return "Login Success!";
        } else {
            return "Login Failed!";
        }
    };

    // TODO: 添加其他路径和处理函数
}

std::tuple<std::string, std::string, std::string> parseHttpRequest(std::string_view request) {
    std::string method, uri;
    std::size_t pos = request.find(' ');
    method = request.substr(0, pos);
    uri = request.substr(pos + 1, request.find(' ', pos + 1) - pos - 1);

    std::string body;
    if (method == "POST") {
        pos = request.find("\r\n\r\n");
        if (pos != std::string::npos) {
            body = request.substr(pos + 4);
        }
    }
    return {method, uri, body};
}

std::string handleHttpRequest(const std::string& method, const std::string& uri, const std::string& body) {
    if (method == "GET" && get_routes.contains(uri)) {
        return get_routes[uri](body);
    }
    else if (method == "POST" && post_routes.contains(uri)) {
        return post_routes[uri](body);
    }
    else {
        return "404 Not Found";
    }
}
int main() {
    int server_fd, new_socked;
    struct sockaddr_in address{};
    int addr_len = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    LOG_INFO("Socket created");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (sockaddr * )&address, sizeof (address)) == -1) {
        LOG_ERROR("bind failed: %s", strerror(errno));
        return 0;
    }
    LOG_INFO("bind successful");

    if (listen(server_fd, 3) == -1) {
        LOG_ERROR("listen failed: %s", strerror(errno));
        return 0;
    }
    LOG_INFO("Server listening on port %d", PORT);

    setupRoutes();
    LOG_INFO("Server starting");

    while (true) {
        new_socked = accept(server_fd, (sockaddr *)&address, (socklen_t*)&addr_len);

        char buffer[1024]{};
        read(new_socked, buffer, 1024);
        std::string request(buffer);

        auto [method, uri, body] = parseHttpRequest(request);
        std::string response_body = handleHttpRequest(method, uri, body);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n" + response_body;

        send(new_socked, response.c_str(), response.size(), 0);
        close(new_socked);
    }

}