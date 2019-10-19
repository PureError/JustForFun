#ifndef JSON_VALUE_HPP
#define JSON_VALUE_HPP
#include "Define.h"

#include <vector>
#include <map>
class Test;
namespace YTCJson
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
        using ObjectContainer = std::map<std::string, Value>;
		using TokenType	      = std::string;
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
			return *this;
        }
        Value(Value&& rval) { *this = std::move(rval); }
        ~Value() { SetNull(); }

        void SetNull()
        {
            switch (GetType())
            {
			case ValueType::kString: SafeDelete<std::string>(data_.complexData); break;
            case ValueType::kArray:  SafeDelete<ArrayContainer>(data_.complexData); break;
            case ValueType::kObject: SafeDelete<ObjectContainer>(data_.complexData); break;
			default:break;
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
                std::istringstream iss(static_cast<std::string*>(data_.complexData)->c_str());
                iss >> number;
            }
                break;
            default:
                JSON_RT_ASSERT(false);
                break;
            }
            return number;
        }
		std::string AsStdString() const 
		{ 
			std::string str;
			switch (GetType())
			{
			case ValueType::kBoolean:
				str = data_.b ? "true" : "false";
				break;
			case ValueType::kNull:
				str = "null";
				break;
			case ValueType::kString:
				str = *static_cast<std::string*>(data_.complexData);
				break;
			case ValueType::kNumbmer:
				str = std::to_string((data_.num.type_ == Number::kInterger) ? data_.num.i64 : data_.num.f64);
				break;
			default:
				JSON_LOG("Can not convert to string!");
				JSON_RT_ASSERT(false);
				break;
			}
			return str;
		}

        void SetBoolean(bool value) 
        {
			SetNull();
            data_.b = value;
            type_ = ValueType::kBoolean;
        }

        void SetInteger(int64_t value)
        {
			SetNull();
            data_.num.i64 = value;
            data_.num.type_ = Number::kInterger;
            type_ = ValueType::kNumbmer;
        }

        void SetFloat(double value)
        {
			SetNull();
            data_.num.f64 = value;
            data_.num.type_ = Number::kFloat;
            type_ = ValueType::kNumbmer;
        }

		void SetString(const std::string& s)
		{
			if (GetType() != ValueType::kString)
			{
				SetNull();
				data_.complexData = new std::string();
				type_ = ValueType::kString;
			}
			JSON_RT_ASSERT(data_.complexData);
			*static_cast<std::string*>(data_.complexData) = s;
		}
        void AddArrayElement(const Value& jsonValue)
        {
            if (GetType() != ValueType::kArray)
            {
                SetNull();
                data_.complexData= new ArrayContainer();
				type_ = ValueType::kArray;
                JSON_RT_ASSERT(data_.complexData);
            }
            auto arrayPtr = static_cast<ArrayContainer*>(data_.complexData);
            arrayPtr->push_back(jsonValue);
        }

        const ArrayContainer& AsArray() const 
        {
            if (IsArray())
            {
                auto arrayPtr = static_cast<ArrayContainer*>(data_.complexData);
                JSON_RT_ASSERT(arrayPtr);
                return *arrayPtr;
            }
            static const ArrayContainer Empty;
            return Empty;
        }
        
        const ObjectContainer& AsDictionary() const 
        {
            if (IsObject())
            {
                auto dict = static_cast<ObjectContainer*>(data_.complexData);
                JSON_RT_ASSERT(dict);
                return *dict;
            }
            static const ObjectContainer Empty;
            return Empty;
        }
        const Value& At(size_t index) const 
        {
            auto& arrayObject = AsArray();
			return index < arrayObject.size() ? arrayObject[index] : Value::EmptyValue;
        }

        const Value& At(const TokenType& token) const
        {
            auto& dict = const_cast<ObjectContainer&>(AsDictionary());
			auto it = dict.find(token);
			return it == dict.end() ? Value::EmptyValue : it->second;
        }

		const Value& operator[](size_t index)			const { return At(index); }
		const Value& operator[](const TokenType& token) const { return At(token); }

		Value& operator[](const TokenType& token) 
		{
			if (type_ != ValueType::kObject)
			{
				SetNull();
				data_.complexData = new ObjectContainer();
				type_ = ValueType::kObject;
			}
			JSON_RT_ASSERT(data_.complexData);
			return static_cast<ObjectContainer*>(data_.complexData)->operator[](token);
		}
    private:
        union InternalData
        {
            Number num;
            bool b;
            byte layout[kAlignment];
            //struct ComplexData
            //{
            //    void* dataPtr;
            //    size_t extParam;
            //} complexData;
			void* complexData;
        } data_ {};

        ValueType type_ = ValueType::kNull;
    };



    YTCJson::Value& Value::operator=(const Value& other)
    {
        switch (other.type_)
        {
        case ValueType::kString:
            SetString(other.AsStdString());
            break;
        case ValueType::kArray:
            data_.complexData = new ArrayContainer(*static_cast<ArrayContainer*>(other.data_.complexData));
            break;
        case ValueType::kObject:
            data_.complexData = new ObjectContainer(*static_cast<ObjectContainer*>(other.data_.complexData));
            break;
        default:
            memcpy(&data_, &other.data_, sizeof(data_));
            break;
        }
		type_ = other.type_;
        return *this;
    }

    const YTCJson::Value Value::EmptyValue;

}


#endif