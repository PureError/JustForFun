#ifndef UTILS_HPP
#define UTILS_HPP
#include "Define.h"
#include <algorithm>

#define JSON_LOG(...)  \
do {\
    printf("[%s][%d][%s]:", __FILE__, __LINE__, __FUNCTION__); \
    printf(__VA_ARGS__);\
    printf("\n");\
} while(false)


namespace Json
{
    template<typename T, size_t N>
    inline bool IsInArray(const T(&arr) [N], const T v) 
    {
        for (auto i = 0; i < N; ++i)
            if (arr[i] == v)
                return true;
        return false;
    }

    inline bool IsWhiteSpace(char c)
    {
        static const char whiteSpace[] = { ' ', '\t', '\n', '\r', };
        return IsInArray(whiteSpace, c);
    }

    inline const char* SkipWhiteSpace(const char* str)
    {
        while (IsWhiteSpace(*str)) ++str;
        return str;
    }

    inline int HexString2Uint32(const char* begin, const char* end)
    {
        char integerString[8] {};
        unsigned number;
        size_t size = end - begin;
        memcpy(integerString, begin, size > sizeof(integerString) ? sizeof(integerString) : size);
        sscanf_s(integerString, "%x", &number);
        return number;
    }


    // 0000~007F       0XXXXXXX
    // 0080~07FF       110XXXXX   10XXXXXX
    // 0080~FFFF       1110XXXX   10XXXXXX   10XXXXXX       
    // 10000~10FFFF    111110XX   10XXXXXX   10XXXXXX   10XXXXXX
    inline std::string Codepoint2StdString(unsigned codepoint)
    {
        static const int kMaxBytes = 4;
        char utfString[kMaxBytes + 1] {};
        if (codepoint < 0x0080)
        {
            utfString[0] = codepoint;
        }
        else if (codepoint < 0x0800)
        {
            utfString[0] = (0xC0) | ((codepoint >> 6) & 0x1F);
            utfString[1] = (0x80) | (codepoint & 0x3F);
        }
        else if (codepoint < 0x10000)
        {
            utfString[0] = (0xE0) | ((codepoint >> 12) & 0x0F);
            utfString[1] = (0x80) | ((codepoint >> 6)  & 0x3F);
            utfString[2] = (0x80  | (codepoint & 0x3F));
        }
        else
        {
            utfString[0] = (0xF8) | ((codepoint >> 18) & 0x03);
            utfString[1] = (0x80) | ((codepoint >> 12) & 0x3F);
            utfString[2] = (0x80) | ((codepoint >> 6) & 0x3F);
            utfString[3] = (0x80) | (codepoint & 0x3F);
        }
        return std::string(utfString, kMaxBytes);
    }
}

#endif 