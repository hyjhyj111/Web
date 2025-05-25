#pragma once
#include <cstdarg>
#include <fstream>
#include <chrono>

enum LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger{
public:
    static void logMessage(LogLevel level, const char *format, ...) {
        std::ofstream logFile("server.log", std::ios::app);

        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);

        std::string str_level;
        switch (level) {
            case LogLevel::INFO:
                str_level = "INFO";
                break;
            case LogLevel::WARNING:
                str_level = "WARNING";
                break;
            case LogLevel::ERROR:
                str_level = "ERROR";
                break;
        }

        va_list arg;
        va_start(arg, format);
        char buffer[2048] {};
        vsnprintf(buffer, sizeof buffer, format, arg);
        va_end(arg);

        logFile << std::ctime(&now_c) << " [" << str_level << "] " << buffer << std::endl;
        logFile.close();
    }
};

#define LOG_INFO(...) Logger::logMessage(LogLevel::INFO, __VA_ARGS__)
#define LOG_WARNING(...) Logger::logMessage(LogLevel::WARNING, __VA_ARGS__)
#define LOG_ERROR(...) Logger::logMessage(LogLevel::ERROR, __VA_ARGS__)
