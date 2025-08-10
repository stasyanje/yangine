#pragma once

#include <fstream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <filesystem>
#include <iostream>

class AsyncLogger
{
public:
    static AsyncLogger& shared()
    {
        static AsyncLogger instance;
        return instance;
    }

    AsyncLogger() :
        done(false)
    {
        worker = std::thread([this]
        {
            std::filesystem::create_directories("logs");
            
            HANDLE h = CreateFileW(
                L"logs\\engine.log",
                FILE_APPEND_DATA,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                OPEN_ALWAYS,
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
                    auto s = std::move(q.front()); q.pop();
                    lk.unlock();
                    write_line(s);
                    lk.lock();
                }
                if (done && q.empty()) break;
            }
        });
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

private:
    std::ofstream fout;
    std::queue<std::string> q;
    std::mutex mu;
    std::condition_variable cv;
    std::thread worker;
    std::atomic<bool> done;
};