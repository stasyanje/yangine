#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class AsyncLogger
{
public:
    // Enum to represent log levels
    enum LogLevel
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    AsyncLogger() :
        done(false)
    {
        worker = std::thread([this] {
            std::filesystem::create_directories("logs");

            HANDLE h = CreateFileW(
                L"logs\\engine.log",
                GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                nullptr
            );
            assert(h != INVALID_HANDLE_VALUE);

            auto write_line = [&](std::string_view s) {
                DWORD written = 0;
                WriteFile(h, s.data(), static_cast<DWORD>(s.size()), &written, nullptr);
                static const char nl = '\n';
                WriteFile(h, &nl, 1, &written, nullptr);
                FlushFileBuffers(h); // гарантируем, что OS сразу выкинет на диск
            };

            std::unique_lock<std::mutex> lk(mu);
            for (;;) {
                cv.wait(lk, [this]{ return done || !q.empty(); });
                while (!q.empty()) {
                    // Create timestamp
                    time_t now = time(0);
                    tm timeinfo;
                    localtime_s(&timeinfo, &now);
                    char timestamp[20];
                    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

                    // Create log entry
                    std::ostringstream logEntry;

                    logEntry
                        << "[" << std::string(timestamp) << "] "
                        << levelToString(LogLevel::DEBUG) << ": "
                        << std::move(q.front()); q.pop();

                    auto message = logEntry.str();

                    lk.unlock();
                    write_line(message);
                    lk.lock();
                }
                if (done && q.empty()) break;
            } });
    }

    ~AsyncLogger()
    {
        {
            std::lock_guard<std::mutex> lk(mu);
            done = true;
        }
        cv.notify_all();
        if (worker.joinable()) worker.join();
        fout.close();
    }

    void log(std::string s)
    {
        std::lock_guard<std::mutex> lk(mu);
        q.push(std::move(s));
        cv.notify_one();
    }

    // Converts log level to a string for output
    std::string levelToString(LogLevel level)
    {
        switch (level) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

private:
    std::ofstream fout;
    std::queue<std::string> q;
    std::mutex mu;
    std::condition_variable cv;
    std::thread worker;
    std::atomic<bool> done;
};