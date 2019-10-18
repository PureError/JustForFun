
#include "Parser.hpp"
#include <iostream>
using namespace std;

class Test
{
public:

    void TestParseNumber()
    {
        static const std::pair<double, const char*> TestGroup[] = {
            { 0.0, "0" },
            { 0.0, "-0" },
            { 0.0, "-0.0" },
            { -1.0, "-1" },
            { 1.5, "1.5" },
            { -1.5, "-1.5" },
            { 3.1416, "3.1416" },
            { 1E10, "1E10" },
            { 1e10, "1e10" },
            { 1E+10, "1E+10" },
            { 1E-10, "1E-10" },
            { -1E10, "-1E10" },
            { -1e10, "-1e10" },
            { -1E+10, "-1E+10" },
            { -1E-10, "-1E-10" },
            { 1.234E+10, "1.234E+10" },
            { 1.234E-10, "1.234E-10" },
        };
        Json::Parser parser;
        for (const auto& testCase:TestGroup)
        {
            parser.Parse(testCase.second);
            auto actualResult = parser.GetValue().template AsNumber<double>();
            double diff = std::fabs(testCase.first - actualResult);
            if (diff < 0.00000001)
            {
                cout << "PASS!" << endl;
            }
            else
            {
                cout << "Shit!" << "Expect:" << testCase.first << ",Result:" << actualResult << endl;
            }
        }
    }
    void TestParseString()
    {
        static const char* TestGroup[] = {
            "\"yutc\"",
            "\"yu\ntc\"",
            "\"y\tutc\"",
            "\"\u20AC\"",
            "\"f\\\"ucku\"",
        };
        Json::Parser parser;
        for (auto json : TestGroup)
        {
            parser.Parse(json);
            auto& actualResult = parser.GetValue();
            auto s = actualResult.AsStdString();
            cout << s.c_str() << endl;
        }
    }


    void TestParseArray()
    {
        static const char* TestGroup[] = {
            "[\n 1, [2,3 ], \"van\"]",
        };
        Json::Parser parser;
        for (auto json : TestGroup)
        {
            parser.Parse(json);
            
        }
    }
    void Start()
    {
        //TestParseNumber();
        TestParseString();
    }
};
int main()
{
    Test t;
    t.Start();
    return 0;
}