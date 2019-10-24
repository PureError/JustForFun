#ifndef DATA_HPP
#define DATA_HPP
#include "Define.h"
namespace Ytc 
{
    class Data final
    {
    public:
        static Data Empty;
        Data(size_t size) : size_(size) 
        {
            buffer_ = (char*)malloc(size);
        }
        Data(void* buffer, size_t size)  { SetBuffer(buffer, size); }
        Data() : size_(0), buffer_(nullptr) {}

        Data(Data&& data)
        {
            buffer_ = data.buffer_;
            size_ = data.GetSize();
            data.buffer_ = nullptr;
        }
        ~Data() 
        { 
            if (buffer_) free(buffer_); 
        }
        std::string ToString() { return std::string(buffer_, GetSize()); }
        void SetBuffer(void* buffer, size_t size) { buffer_ = static_cast<char*>(buffer), size_ = size; }
        size_t GetSize() const { return size_; }
        const void* GetBuffer() const { return static_cast<void*>(buffer_); }
    private:
        size_t size_;
        char* buffer_;
    };

    Ytc::Data Data::Empty;

}
#endif 