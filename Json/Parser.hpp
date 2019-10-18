#ifndef PARSER_HPP
#define PARSER_HPP

#include "Utils.hpp"
#include "JsonValue.hpp"

class Test;
namespace Json
{
    class Parser
    {
    public:
        friend class Test;
        enum ParseMessage
        {
            kCorrectly,
            kInvalidJson,
        };

    private:
        template <char... Letters>
        bool ParseLiteral(const char*& json)
        {
            static const char Characters[] = { Letters... };
            bool result = true;
            if (memcmp(Characters, json, sizeof(Characters)))
            {
                result = ParseMessage::kInvalidJson;
            }
            else
            {
                json += sizeof(Characters);
            }
            return result;
        }

        ParseMessage ParseNull(const char*& json, Json::Value& value)
        {
            if (ParseLiteral<'n', 'u', 'l', 'l'>(json))
            {
                value.SetNull();
                return ParseMessage::kCorrectly;
            }
            return ParseMessage::kInvalidJson;
        }

        ParseMessage ParseTrue(const char*& json, Json::Value& value)
        {
            if (ParseLiteral<'t', 'r', 'u', 'e'>(json))
            {
                value.SetBoolean(true);
                return ParseMessage::kCorrectly;
            }
            return ParseMessage::kInvalidJson;
        }

        ParseMessage ParseFalse(const char*& json, Json::Value& value)
        {
            if (ParseLiteral<'f', 'a', 'l', 's', 'e'>(json))
            {
                value.SetBoolean(false);
                return ParseMessage::kCorrectly;
            }
            return ParseMessage::kInvalidJson;
        }

        ParseMessage ParseNumber(const char*& json, Json::Value& value)
        {
            const char* begin = json;
            char c = *json;
            bool hasFraction = false;
            bool hasExpSymbol = false;
            if (c == '-') c = *(++json);

            if (c == '0') 
            {
                c = *(++json);
            }
            else
            {
                if (!IS_DIGIT(c)) return ParseMessage::kInvalidJson;
                do { c = *(++json); } while (IS_DIGIT(c));
            }

            if (c == '.')
            {
                hasFraction = true;
                c = *(++json);
                
                if (!IS_DIGIT(c)) return ParseMessage::kInvalidJson;
                do { c = *(++json); } while (IS_DIGIT(c));
            }
          
            if (IS_EXP_SYMBOL(c))
            {
                hasExpSymbol = true;
                c = *(++json);
                if (IS_SIGN_SYMBOL(c)) c = *(++json);
                if (!IS_DIGIT(c)) return ParseMessage::kInvalidJson;
                do { c = *(++json); } while (IS_DIGIT(c));
            }

            char buffer[64] = {};
            int len = json - begin;
            memcpy(buffer, begin, len);
 
            if (hasFraction || hasExpSymbol)
            {
                double dVal = strtod(buffer, nullptr);
                if (dVal == HUGE_VAL)
                {
                    JSON_LOG("The number is too huge!");
                    return ParseMessage::kInvalidJson;
                }
                value.SetFloat(dVal);
            }
            else
            {
                value.SetInteger(atoll(buffer));
            }
            return ParseMessage::kCorrectly;
        }


        enum { kCodePointCharacters = 4 };

        ParseMessage ParseString(const char*& json, Json::Value& value)
        {
            char c = *(++json);
            const char* begin = json;
            StringParseState state = StringParseState::kParsingOridnary;
            std::string str;

            while (!(c == '\"' && state == StringParseState::kParsingOridnary))
            {
                switch (state)
                {
                case StringParseState::kParsingEscape:
                    switch (c)
                    {
                    case '\\':
                    case  '/':
                    case '\f':
                    case '\n':
                    case '\r':
                    case '\t':
                    case '\b':
                    case '\"':
                        str += c;
                        state = StringParseState::kParsingOridnary;
                        break;
                    case 'u':
                        state = StringParseState::kParsingCodePoint;
                        break;
                    }
                    c = *(++json);
                    begin = json;
                    break;
                case StringParseState::kParsingCodePoint:
                {
                    auto codePoint = HexString2Uint32(json, (json += kCodePointCharacters));
                    if (IS_LOW_SURROGATE(codePoint))
                    {
                        auto low = HexString2Uint32(json, (json += kCodePointCharacters));
                        if (!IS_HIGH_SURROGATE(low))
                        {
                            JSON_LOG("Invalid CodePoint!");
                            return ParseMessage::kInvalidJson;
                        }
                        codePoint = SURROGATE_CODE_POINT(codePoint, low);
                    }
                    std::string actualString = Codepoint2StdString(codePoint);
                    str += actualString;
                    state = StringParseState::kParsingOridnary;
                    begin = json;
                }
                    break;
                default:
                    switch (c)
                    {
                    case '\0':
                        JSON_LOG("\" missed!");
                        return ParseMessage::kInvalidJson;
                        break;
                    case '\\':
                        str += std::string(begin, json);
                        state = StringParseState::kParsingEscape;
                        break;
                    default:
                        break;
                    }
                    c = *(++json);
                    break;
                }
            }
            str += std::string(begin, ++json);
            value.SetString(str.c_str(), str.length());
            return ParseMessage::kCorrectly;
        }


        enum StringParseState
        {
            kParsingOridnary,
            kParsingEscape,
            kParsingCodePoint,  
        };

        ParseMessage ParseArray(const char*& json, Json::Value& value)
        {
            Json::Value element;
            for (;;)
            {
                ParseMessage message = ParseValue(++json, element);
                if (message != ParseMessage::kCorrectly)
                {
                    return ParseMessage::kInvalidJson;
                }
                value.AddArrayElement(element);
                SkipWhiteSpace(json);
                switch (*json)
                {
                case ',':
                    ++json;
                	break;
                case ']':
                    ++json;
                    return ParseMessage::kCorrectly;
                    break;
                default:
                    JSON_LOG("Unexcept character!");
                    JSON_RT_ASSERT(false);
                    return ParseMessage::kInvalidJson;
                    break;
                }
            }
            return ParseMessage::kInvalidJson;
        }


        ParseMessage ParseObject(const char*& json, Json::Value& value)
        {
            return ParseMessage::kCorrectly;
        }
    public:

        ParseMessage ParseValue(const char* json, Json::Value& value)
        {
            ParseMessage result = ParseMessage::kInvalidJson;
            if (json)
            {
                char c;
                while (*json)
                {
                    json = SkipWhiteSpace(json);
                    c = *json;
                    switch (c)
                    {
                    case 'n': result = ParseNull(json, value); break;
                    case 'f': result = ParseFalse(json, value);break;
                    case 't': result = ParseTrue(json, value); break;
                    case '\"':result = ParseString(json, value); break;
                    case '[': result = ParseArray(json, value); break;
                    case '{': result = ParseObject(json, value); break;
                    default:  result = ParseNumber(json, value); break;
                    }
                }
                JSON_LOG("Done");
            }
            return result;
        }

        ParseMessage Parse(const char* json) { return ParseValue(json, value_); }
        const Value& GetValue() const { return value_; }
    private:
        Value value_;
    };
}

#endif 