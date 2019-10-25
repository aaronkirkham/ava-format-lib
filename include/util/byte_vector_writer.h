#include <assert.h>
#include <vector>

class byte_vector_writer
{
  public:
    byte_vector_writer(std::vector<uint8_t>* buffer)
        : buffer_(buffer)
        , offset_(0)
    {
        assert(buffer != nullptr);
    }

    void write(const void* data, const size_t size)
    {
        const size_t buffer_size = buffer_->size();
        if (buffer_size < (offset_ + size)) {
            buffer_->resize(buffer_size + size);
        }

        std::memcpy(buffer_->data() + offset_, data, size);
        offset_ += size;
    }

    const size_t tellp() const
    {
        return offset_;
    }

  private:
    std::vector<uint8_t>* buffer_;
    size_t                offset_;
};
