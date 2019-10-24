#pragma once

#include "Define.h"
#include <windows.h>
#include "Data.hpp"
namespace Ytc 
{
    enum FileAttribute
    {
        kInvalidFileAttribute = 0,
        kReadOnly = 1,
        kHidden = 1 << 1,
        kSystem = 1 << 2,
        kDirectory = 1 << 4,
        kArchive = 1 << 5,
        kDevice = 1 << 6,
        kNormal = 1 << 7, 
        kTemporary = 1 << 8,
        kSparseFile = 1 << 9,
        kReparsePoint = 1 << 10,
        kCompressed = 1 << 11,
        kOffline = 1 << 12,
        kNotContentIndexed = 1 << 13,
        kEncrypted = 1 << 14,
        kIntegrityStream = 1 << 15,

        kNoScrubData = 1 << 17,
    };

    class FileStream
    {
    public:
        enum AccessMode
        {
            kReadOnly = 0,
            kWriteOnly,
            kReadWrite,
        };

        enum SeekOrigin
        {
            kBegin   = SEEK_SET,
            kCurrent = SEEK_CUR,
            kEnd     = SEEK_END,
        };
        static const char* GetModeString(AccessMode mode)
        {
            static const char* StringMap[] = { "rb", "wb", "w+b" };
            return StringMap[mode];
        }

        FileStream(const std::string& path, AccessMode mode = AccessMode::kReadOnly) :mode_(mode)
        {
            fopen_s(&fp_, path.c_str(), GetModeString(mode));
        }

        size_t GetSize() const
        {
            fpos_t size = 0;
            if (fp_)
            {
                fpos_t position = GetPosition();
                if (fseek(fp_, 0, SEEK_END) == 0)
                {
                    size = GetPosition();
                    fsetpos(fp_, &position);
                }
            }
            return size_t(size);
        }

        bool CanRead() const { return fp_ && (mode_ == AccessMode::kReadOnly || mode_ == AccessMode::kReadWrite); }
        bool CanPeek() const { return CanRead(); }
        bool CanSeek() const { return fp_ != nullptr; }
        void Close()         { if (fp_) { fclose(fp_); fp_ = nullptr; } }
        
        template<typename Ch>
        Ch Peek() 
        {
            Ch ch(EOF);
            Read(&ch, 0, sizeof(Ch));
            return ch;
        }
        
        void Seek(int diff, SeekOrigin origin)
        {
            fpos_t from = 0;
            const auto size = GetSize();
            switch (origin)
            {
            case kBegin:break;
            case kCurrent:from = GetPosition(); break;
            case kEnd:from = size; break;
            default:assert(false);
            }

            if ((from + diff) < size)
            {
                fseek(fp_, diff, origin);
            }
        }

        size_t Read(void* buffer, int offset, size_t byptes)
        {
            if (!buffer) return 0;
            return fread(buffer, 1, byptes, fp_);
        }

        size_t Write(void* buffer, int offset, size_t byptes)
        {
            if (!buffer) return 0;
            return fwrite(buffer, 1, byptes, fp_);
        }

        ~FileStream() { Close(); }
    private:
        fpos_t GetPosition() const 
        {
            fpos_t position = 0;
            if (fp_)
            {
                fgetpos(fp_, &position);
            }
            return position;
        }

        FILE* fp_;
        AccessMode mode_;
    };

    class File
    {
    public:
        static const size_t kMaxPath = 512;
        static bool Exists(const std::string& path)
        {
            if (path.empty()) return false;
            return (GetAttributes(path) & (FileAttribute::kDirectory)) == 0;
        }

        static FileAttribute GetAttributes(const std::string& path)
        {
#ifdef _MSC_VER
            wchar_t buffer[kMaxPath]{};
            ::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, buffer, sizeof(buffer) / sizeof(buffer[0]));
            DWORD attribution = ::GetFileAttributesW(buffer);
            return (INVALID_FILE_ATTRIBUTES == attribution) ? FileAttribute::kInvalidFileAttribute : FileAttribute(attribution);
#endif
        }

        static void Move(const std::string& src, const std::string& dest)
        {

        }


        static Data ReadAllBytes(const std::string& path)
        {
            FileStream fs(path);
            if (!fs.CanRead()) return Data::Empty;
            auto fileSize = fs.GetSize();
            Data data(fileSize);
            fs.Read(const_cast<void*>(data.GetBuffer()), 0, fileSize);
            return data;
        }
    };
}