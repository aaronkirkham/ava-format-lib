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
        if (!buffer->empty()) {
            offset_ = buffer->size();
        }
    }

    void write(const void* data, const size_t size)
    {
        buffer_alloc(size);
        std::memcpy(buffer_->data() + offset_, data, size);
        offset_ += size;
    }

    void write(const void* data, const size_t size, const size_t count)
    {
        if (count == 0) {
            return;
        }

        buffer_alloc(size * count);
        for (size_t i = 0; i < count; ++i) {
            std::memcpy(buffer_->data() + (offset_ + (i * size)), data, size);
        }
        offset_ += (size * count);
    }

    void setp(const size_t pos)
    {
        offset_ = pos;
    }

    const size_t tellp() const
    {
        return offset_;
    }

  private:
    std::vector<uint8_t>* buffer_;
    size_t                offset_;

    void buffer_alloc(const size_t size)
    {
        const size_t buffer_size = buffer_->size();
        if (buffer_size < (offset_ + size)) {
            buffer_->resize(buffer_size + size);
        }
    }
};
