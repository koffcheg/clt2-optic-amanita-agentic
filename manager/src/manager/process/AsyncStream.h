//
//  AsyncStream.h
//  manager
//
//  Created by apple on 27.05.2024.
//

#pragma once

#include <boost/asio/streambuf.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/process/async_pipe.hpp>
#include <boost/process.hpp>

#include <vector>
#include <mutex>
#include <atomic>
#include <future>
#include <string>

#include "NonBlockingStream.h"


namespace cm
{
    class LineSplittingStream
    {
    public:
        LineSplittingStream():_dirtyFlag(true) {}
        ~LineSplittingStream() {}

        void append(const char* data, size_t len)
        {
            size_t at = _buffer.size();
            _buffer.resize(at + len);
            char* dst = _buffer.data() + at;
            memcpy(dst, data, len);
            _dirtyFlag = true;
        }

        void append(char c)
        {
            _buffer.push_back(c);
            _dirtyFlag = true;
        }

        void readAll(std::vector<std::string>& output)
        {
            if (_dirtyFlag)
            {
                _dirtyFlag = false;
                split();
            }
            std::swap(_lines, output);
        }

    private:
        void split()
        {
            size_t start = 0;
            size_t size = _buffer.size();
            const char* ptr = _buffer.data();
            for (size_t i = 0; i < size; ++i)
            {
                if (isDelimiter(_buffer[i]))
                {
                    _buffer[i] = '\0';

                    if (i > start)
                        _lines.push_back(std::string(ptr + start));

                    start = i + 1;
                }
            }

            if (start > 0)
                _buffer.erase(_buffer.begin(), _buffer.begin() + start);
        }

        bool isDelimiter(char c)
        {
            return c == '\n' || c == '\0';
        }
        
        std::vector<char> _buffer;
        std::vector<std::string> _lines;
        bool _dirtyFlag;
    };

    static const size_t kAsyncStreamBufferSize = 1024;
    class AsyncStream
    {
    public:
        static std::shared_ptr<AsyncStream> create();

        AsyncStream();
        ~AsyncStream();

        boost::process::async_pipe& pipe() { return _pipeStream; }
        boost::asio::io_service& ios() { return _ios; }
        boost::process::ipstream& ipstream() { return _ipstream; }
        boost::asio::streambuf& asiobuff() { return _asiobuff; }
        NonBlockingStream& nbs() { return _nbs; }

        void readAll(std::vector<std::string>& output);
        void write(const std::string& message);
        void start();
        void stop();
        
        void readOnce();

    private:

        std::atomic<bool> _started;
        std::atomic<bool> _running;

        std::mutex _messagesLock;
        std::vector<std::string> _messages;

        boost::asio::io_service _ios;
        boost::asio::streambuf _asiobuff;
        boost::process::async_pipe _pipeStream;
        std::weak_ptr<AsyncStream> _weakSelf;

        boost::process::ipstream _ipstream;
        LineSplittingStream _lstream;
        NonBlockingStream _nbs;

        char _buff[kAsyncStreamBufferSize];
    };
}

