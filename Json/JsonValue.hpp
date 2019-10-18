#ifndef JSON_VALUE_HPP
#define JSON_VALUE_HPP
#include "Define.h"

#include <vector>
#include <unordered_map>
class Test;
namespace Json
{

    struct Number
    {
        enum NumberType
        {
            kFloat,
            kInterger,
        };
        union
        {
            int64_t i64;
            double f64;
        };
        NumberType type_;
    };


    class Value
    {
    public:
        using ArrayContainer  = std::vector<Value>;
        using ObjectContainer = std::unordered_map<std::string, Value>;
        friend class Test;
        enum ValueType
        {
            kNull = 0,
            kBoolean,
            kNumbmer,
            kString,
            kArray,
            kObject,
        };
        static const size_t kAlignment = 16;
        static const Value EmptyValue;
        Value() = default;
        Value(const Value& other) { (*this) = other; }
        Value& operator=(const Value& other);
        Value& operator=(Value&& rval)
        {
            SetNull();
            memcpy(this, &rval, sizeof(rval));
            memset(&rval, 0, sizeof(rval));
        }
        Value(Value&& rval) { *this = std::move(rval); }
        ~Value() { SetNull(); }

        void SetNull()
        {
            switch (GetType())
            {
            case ValueType::kString:
                data_.complexData.extParam = 0;
                free(data_.complexData.dataPtr);
                break;
            case ValueType::kArray:
            {
                auto arrayPtr = static_cast<ArrayContainer*>(data_.complexData.dataPtr);
                delete arrayPtr;
                data_.complexData.dataPtr = nullptr;
            }
                break;
            case ValueType::kObject:
            {
                auto objectPtr = static_cast<ObjectContainer*>(data_.complexData.dataPtr);
                delete objectPtr;
                data_.complexData.dataPtr = nullptr;
            }
                break;
            default:
                break;
            }
            type_ = ValueType::kNull;
        }
        ValueType GetType() const   { return type_; }
        bool IsNull() const         { return GetType() == ValueType::kNull; }
        bool IsArray() const        { return GetType() == ValueType::kArray; }
        bool IsObject() const       { return GetType() == ValueType::kObject; }
        bool AsBoolean() const      { return data_.b; }
        template<typename NumType>
        NumType AsNumber() const 
        {
            NumType number = NumType(0);
            switch (GetType())
            {
            case ValueType::kNumbmer:
                number = NumType(data_.num.type_  == Number::NumberType::kFloat ? data_.num.f64 : data_.num.i64);
                break;
            case ValueType::kNull:
                break;
            case ValueType::kString:
            {
                std::istringstream iss((char*)(data_.complexData.dataPtr));
                iss >> number;
            }
                break;
            default:
                JSON_RT_ASSERT(false);
                break;
            }
            return number;
        }
        const char* AsCString()   const { return (char*)data_.complexData.dataPtr; }
        std::string AsStdString() const { return std::string((char*)data_.complexData.dataPtr, data_.complexData.extParam - 1); }

        void SetBoolean(bool value) 
        {
            data_.b = value;
            type_ = ValueType::kBoolean;
        }

        void SetInteger(int64_t value)
        {
            data_.num.i64 = value;
            data_.num.type_ = Number::kInterger;
            type_ = ValueType::kNumbmer;
        }

        void SetFloat(double value)
        {
            data_.num.f64 = value;
            data_.num.type_ = Number::kFloat;
            type_ = ValueType::kNumbmer;
        }

        void SetString(const char* str, size_t length)
        {
            data_.complexData.extParam = length + 1;
            data_.complexData.dataPtr = calloc(data_.complexData.extParam, sizeof(char));
            JSON_RT_ASSERT(data_.complexData.dataPtr);
            if (data_.complexData.dataPtr)
            {
                memcpy(data_.complexData.dataPtr, str, length);
            }
            type_ = ValueType::kString;
        }

        void AddArrayElement(const Value& jsonValue)
        {
            if (GetType() != ValueType::kArray)
            {
                SetNull();
                data_.complexData.dataPtr = new ArrayContainer;
                JSON_RT_ASSERT(data_.complexData.dataPtr);
            }
            auto arrayPtr = static_cast<ArrayContainer*>(data_.complexData.dataPtr);
            arrayPtr->push_back(jsonValue);
        }

        const ArrayContainer& AsArray()
        {
            if (IsArray())
            {
                auto arrayPtr = static_cast<ArrayContainer*>(data_.complexData.dataPtr);
                JSON_RT_ASSERT(arrayPtr);
                return *arrayPtr;
            }
            static const ArrayContainer Empty;
            return Empty;
        }
        
        const ObjectContainer& AsDictionary()
        {
            if (IsObject())
            {
                auto dict = static_cast<ObjectContainer*>(data_.complexData.dataPtr);
                JSON_RT_ASSERT(dict);
                return *dict;
            }
            static const ObjectContainer Empty;
            return Empty;
        }
        const Value& At(size_t index) 
        {
            auto& arrayObject = AsArray();
            index = index < arrayObject.size() ? index : arrayObject.size();
            return arrayObject[index];
        }

        const Value& At(const std::string& token)
        {
            auto& dict = const_cast<ObjectContainer&>(AsDictionary());
            return dict[token];
        }

        //template<typename T>
    private:
        union InternalData
        {
            Number num;
            bool b;
            byte layout[kAlignment];
            struct ComplexData
            {
                void* dataPtr;
                size_t extParam;
            } complexData;
        } data_ {};

        ValueType type_ = ValueType::kNull;
    };



    Json::Value& Value::operator=(const Value& other)
    {
        //if (GetType() == ValueType::kNull)
        //{
        //    switch (other.GetType())
        //    {
        //    case ValueType::kString:
        //        break;
        //    case ValueType::kArray:
        //    case ValueType::kObject:
        //        break;
        //    default:
        //        break;
        //    }
        //}
        //else if (GetType() == other.GetType())
        //{
        //    switch (GetType())
        //    {
        //    case ValueType::kString:
        //        JSON_RT_ASSERT(data_.complexData.dataPtr);
        //        //memcpy(data_.complexData.dataPtr, );
        //        break;
        //    }
        //}
        SetNull();
        type_ = other.type_;
        switch (GetType())
        {
        case ValueType::kString:
            SetString(other.AsCString(), other.data_.complexData.extParam);
            break;
        case ValueType::kArray:
            data_.complexData.dataPtr = new ArrayContainer(*static_cast<ArrayContainer*>(other.data_.complexData.dataPtr));
            break;
        case ValueType::kObject:
            data_.complexData.dataPtr = new ObjectContainer(*static_cast<ObjectContainer*>(other.data_.complexData.dataPtr));
            break;
        default:
            memcpy(&data_, &other.data_, sizeof(data_));
            break;
        }
        return *this;
    }

    const Json::Value Value::EmptyValue;

    
}


#endif