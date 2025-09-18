// AsyncBuf.h
#pragma once

#include <streambuf>
#include <string>

#include "AsyncLogger.h"

struct AsyncBuf : std::streambuf
{
    explicit AsyncBuf(AsyncLogger& L) :
        logger(L)
    {
    }

    // буфер для накопления до \n
    std::string buf;

    int overflow(int ch) override
    {
        if (ch == traits_type::eof()) return 0;
        buf.push_back(static_cast<char>(ch));
        return ch;
    }

    int sync() override
    {
        flush_();
        return 0;
    }

private:
    void flush_()
    {
        if (!buf.empty()) {
            logger.log(std::move(buf));
            buf.clear();
        }
    }
    AsyncLogger& logger;
};