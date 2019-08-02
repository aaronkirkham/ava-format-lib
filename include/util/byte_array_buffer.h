#include <assert.h>
#include <cstdint>
#include <istream>
#include <streambuf>

class byte_array_buffer : public std::streambuf
{
  public:
    byte_array_buffer(const uint8_t* begin, const size_t size)
        : begin_(begin)
        , end_(begin + size)
        , current_(begin_)
    {
        assert(std::less_equal<const uint8_t*>()(begin_, end_));
    }

  private:
    int_type underflow()
    {
        if (current_ == end_)
            return traits_type::eof();

        return traits_type::to_int_type(*current_);
    }

    int_type uflow()
    {
        if (current_ == end_)
            return traits_type::eof();

        return traits_type::to_int_type(*current_++);
    }

    int_type pbackfail(int_type ch)
    {
        if (current_ == begin_ || (ch != traits_type::eof() && ch != current_[-1]))
            return traits_type::eof();

        return traits_type::to_int_type(*--current_);
    }

    std::streamsize showmanyc()
    {
        assert(std::less_equal<const uint8_t*>()(current_, end_));
        return end_ - current_;
    }

    std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way,
                           std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
    {
        if (way == std::ios_base::beg) {
            current_ = begin_ + off;
        } else if (way == std::ios_base::cur) {
            current_ += off;
        } else if (way == std::ios_base::end) {
            current_ = end_;
        }

        if (current_ < begin_ || current_ > end_)
            return -1;

        return current_ - begin_;
    }

    std::streampos seekpos(std::streampos sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
    {
        current_ = begin_ + sp;

        if (current_ < begin_ || current_ > end_)
            return -1;

        return current_ - begin_;
    }

    // copy ctor and assignment not implemented;
    // copying not allowed
    byte_array_buffer(const byte_array_buffer&) = delete;
    byte_array_buffer& operator=(const byte_array_buffer&) = delete;

  private:
    const uint8_t* const begin_;
    const uint8_t* const end_;
    const uint8_t*       current_;
};
