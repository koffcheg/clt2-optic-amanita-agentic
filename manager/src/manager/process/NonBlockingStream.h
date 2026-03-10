#include <boost/config.hpp>
#include <boost/process/detail/config.hpp>
#include <streambuf>
#include <istream>
#include <ostream>
#include <vector>

namespace cm
{
struct NonBlockingStream : std::basic_streambuf<char, std::char_traits<char>>
{
    typedef char CharT;
    typedef std::char_traits<CharT> Traits;

    typedef boost::process::detail::posix::basic_pipe<CharT, Traits> pipe_type;

    typedef           CharT            char_type  ;
    typedef           Traits           traits_type;
    typedef  typename Traits::int_type int_type   ;
    typedef  typename Traits::pos_type pos_type   ;
    typedef  typename Traits::off_type off_type   ;

    constexpr static int default_buffer_size = 1024;

    ///Default constructor, will also construct the pipe.
    NonBlockingStream() : _write(default_buffer_size), _read(default_buffer_size)
    {
        this->setg(_read.data(),  _read.data()+ 128,  _read.data() + 128);
        this->setp(_write.data(), _write.data() + _write.size());
    }
    ///Copy Constructor.
    NonBlockingStream(const NonBlockingStream & ) = default;
    ///Move Constructor
    NonBlockingStream(NonBlockingStream && ) = default;

    ///Destructor -> writes the frest of the data
    ~NonBlockingStream()
    {
        try 
        {
            if (NonBlockingStream::is_open())
                NonBlockingStream::overflow(Traits::eof());
        }
        catch (const boost::process::process_error & )
        {
        }
    }

    ///Move construct from a pipe.
    NonBlockingStream(pipe_type && p) : _pipe(std::move(p)),
                                    _write(default_buffer_size),
                                    _read(default_buffer_size)
    {
        this->setg(_read.data(),  _read.data()+ 128,  _read.data() + 128);
        this->setp(_write.data(), _write.data() + _write.size());
    }
    ///Construct from a pipe.
    NonBlockingStream(const pipe_type & p) : _pipe(p),
                                        _write(default_buffer_size),
                                        _read(default_buffer_size)
    {
        this->setg(_read.data(),  _read.data()+ 128,  _read.data() + 128);
        this->setp(_write.data(), _write.data() + _write.size());
    }
    ///Copy assign.
    NonBlockingStream& operator=(const NonBlockingStream & ) = delete;
    ///Move assign.
    NonBlockingStream& operator=(NonBlockingStream && ) = default;
    ///Move assign a pipe.
    NonBlockingStream& operator=(pipe_type && p)
    {
        _pipe = std::move(p);
        return *this;
    }
    ///Copy assign a pipe.
    NonBlockingStream& operator=(const pipe_type & p)
    {
        _pipe = p;
        return *this;
    }
    ///Writes characters to the associated output sequence from the put area
    int_type overflow(int_type ch = traits_type::eof()) override
    {
        if (_pipe.is_open() && (ch != traits_type::eof()))
        {
            if (this->pptr() == this->epptr())
            {
                bool wr = this->_write_impl();
                if (wr)
                {
                    *this->pptr() = ch;
                    this->pbump(1);
                    return ch;
                }
            }
            else
            {
                *this->pptr() = ch;
                this->pbump(1);
                if (this->_write_impl())
                    return ch;
            }
        }
        else if (ch == traits_type::eof())
           this->sync();

        return traits_type::eof();
    }
    ///Synchronizes the buffers with the associated character sequence
    int sync() override { return this->_write_impl() ? 0 : -1; }

    ///Reads characters from the associated input sequence to the get area
    int_type underflow() override
    {
        if (!_pipe.is_open())
            return traits_type::eof();

        if (this->egptr() == &_read.back()) //ok, so we're at the end of the buffer
            this->setg(_read.data(),  _read.data()+ 10,  _read.data() + 10);


        auto len = &_read.back() - this->egptr() ;

        pollfd fd;
        fd.fd = _pipe.native_source();
        fd.events = POLLIN|POLLPRI;
        fd.revents = 0;

        if (::poll(&fd, 1, 50) <= 0)
            return traits_type::eof();

        auto res = _pipe.read(
                        this->egptr(),
                        static_cast<typename pipe_type::int_type>(len));
        if (res == 0)
            return traits_type::eof();

        this->setg(this->eback(), this->gptr(), this->egptr() + res);
        auto val = *this->gptr();

        return traits_type::to_int_type(val);
    }


    ///Set the pipe of the streambuf.
    void pipe(pipe_type&& p)      {_pipe = std::move(p); }
    ///Set the pipe of the streambuf.
    void pipe(const pipe_type& p) {_pipe = p; }
    ///Get a reference to the pipe.
    pipe_type &      pipe() &       {return _pipe;}
    ///Get a const reference to the pipe.
    const pipe_type &pipe() const & {return _pipe;}
    ///Get a rvalue reference to the pipe. Qualified as rvalue.
    pipe_type &&     pipe()  &&     {return std::move(_pipe);}

    ///Check if the pipe is open
    bool is_open() const {return _pipe.is_open(); }

    ///Open a new pipe
    NonBlockingStream* open()
    {
        if (is_open())
            return nullptr;
        _pipe = pipe();
        return this;
    }

    // ///Open a new named pipe
    // NonBlockingStream* open(const std::string & name)
    // {
    //     if (is_open())
    //         return nullptr;
    //     _pipe = pipe(name);
    //     return this;
    // }

    ///Flush the buffer & close the pipe
    NonBlockingStream* close()
    {
        if (!is_open())
            return nullptr;
        overflow(Traits::eof());
        return this;
    }

    bool will_underflow() const
    {
        auto ninp = gptr();
        auto einp = egptr();
        return ninp == einp;
    }
private:
    pipe_type _pipe;
    std::vector<char_type> _write;
    std::vector<char_type> _read;

    bool _write_impl()
    {
        if (!_pipe.is_open())
            return false;

        auto base = this->pbase();

        if (base == this->pptr())
            return true;

        std::ptrdiff_t wrt = _pipe.write(base,
                static_cast<typename pipe_type::int_type>(this->pptr() - base));

        std::ptrdiff_t diff = this->pptr() - base;

        if (wrt < diff)
            std::move(base + wrt, base + diff, base);
        else if (wrt == 0) //broken pipe
            return false;

        this->pbump(static_cast<int>(-wrt));

        return true;
    }
};

}
