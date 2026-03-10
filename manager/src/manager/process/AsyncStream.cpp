//
//  AsyncStream.cpp
//  manager
//
//  Created by apple on 27.05.2024.
//

#include "AsyncStream.h"
#include <boost/asio/streambuf.hpp>
#include <boost/asio.hpp>
#include <sstream>
#include <iostream>

namespace cm
{
std::shared_ptr<AsyncStream> AsyncStream::create()
{
    std::shared_ptr<AsyncStream> result = std::make_shared<AsyncStream>();
    result->_weakSelf = result;
    return result;
}

AsyncStream::AsyncStream()
    : _running(true)
    , _started(false)
    , _ios()
    , _pipeStream(_ios)
{

}

AsyncStream::~AsyncStream()
{

}

void AsyncStream::readAll(std::vector<std::string>& output)
{
    if (!_running)
        return;
    {
        std::lock_guard<std::mutex> lock(_messagesLock);
        //std::swap(_messages, output);
        _lstream.readAll(output);
    }
}

void AsyncStream::write(const std::string& message)
{
    if (!_running)
        return;
    {
        std::lock_guard<std::mutex> lock(_messagesLock);
        _messages.push_back(message);
    }
}

void AsyncStream::start()
{   
    if (_started)
        return;

    _started = true;
}

void AsyncStream::stop()
{
    _running = false;
}

static std::string BufferToString(const boost::asio::streambuf &buffer)
{
    using boost::asio::buffers_begin;
  
    auto bufs = buffer.data();
    std::string result(buffers_begin(bufs), buffers_begin(bufs) + buffer.size());
    return result;
}

void AsyncStream::readOnce()
{
    if (!_running)
        return;

    std::lock_guard<std::mutex> lock(_messagesLock);

    _nbs.sync();

    while (true)
    {
        // if (_nbs.will_underflow())
        //     break;
        int ci = _nbs.sbumpc();
        if (ci == std::char_traits<char>::eof())
            break;

        _lstream.append(std::char_traits<char>::to_char_type(ci));
    }


    // size_t cnt = _ipstream.rdbuf()->in_avail();
    // for (size_t i = 0; i < cnt; ++i)
    // {
    //     _lstream.append(_ipstream.rdbuf()->sbumpc());
    // }

    // while (true)
    // {
    //     int ci = _ipstream.rdbuf()->sbumpc();
    //     if (ci == std::char_traits<char>::eof())
    //         break;

    //     _lstream.append(std::char_traits<char>::to_char_type(ci));
    // }

    // for (;;)
    // {
        // size_t readLen = _ipstream.readsome(_buff, kAsyncStreamBufferSize);
        // if (readLen > 0)
        // {
        //     _lstream.append(_buff, readLen);
        // }
        // if (readLen < kAsyncStreamBufferSize)
        //     break;
    // }

    // std::shared_ptr<boost::asio::streambuf> pendingOutput = std::make_shared<boost::asio::streambuf>();
    // typedef std::function<void(const boost::system::error_code & ec, std::size_t n)> Handler;
    // std::shared_ptr<AsyncStream> anchor = _weakSelf.lock();

    // std::cout << pendingOutput->size() << std::endl;
    // static const size_t kBufferSize = 1024;
    // auto buffer = std::make_shared<std::string>();
    // buffer->resize(kBufferSize + 1);

    // Handler outHandler = [buffer, anchor] (const boost::system::error_code & ec, size_t n)
    // {
    //     if (!ec)
    //     {
    //         anchor->write(*buffer);
    //         anchor->readOnce();
    //     }
    // };
    // _pipeStream.async_read_some(boost::asio::buffer(buffer->data(), kBufferSize), outHandler);
    // boost::asio::async_read_until(_pipeStream, *pendingOutput.get(), '\n', outHandler);
}

}
