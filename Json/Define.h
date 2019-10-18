#pragma once

#include <cstdint>
#include <cassert>
#include <string>

#include <sstream>
#define JSON_RT_ASSERT assert
#define JSON_STATIC_ASSERT static_assert

using byte = unsigned char;

#define IS_DIGIT(x)                     ((x) <= '9' && (x) >= '0') 
#define IS_EXP_SYMBOL(x)                ((x) == 'e' || (x) == 'E')
#define IS_SIGN_SYMBOL(x)               ((x) == '+' || (x) == '-')
#define IS_LOW_SURROGATE(hex)           ((hex) <= 0xDBFF && (hex) >= 0xD800)
#define IS_HIGH_SURROGATE(hex)          ((hex) <= 0xDFFF && (hex) >= 0xDC00)
#define SURROGATE_CODE_POINT(h, l)      (0x10000 + (((h) - 0xD800) * 0x400) + ((l) - 0xDC00))