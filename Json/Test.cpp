
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
		YTCJson::Parser parser;
		cout << "Number-Parse Test>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
		for (const auto& testCase : TestGroup)
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
			"\"s\\\"hit\"",
		};
		YTCJson::Parser parser;
		cout << "String-Parse Test>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
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
			"[1,2,3,4,5,6,7,8]"
		};
		YTCJson::Parser parser;
		cout << "Array-Parse Test>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
		for (auto json : TestGroup)
		{
			parser.Parse(json);
			auto arr = parser.GetValue().AsArray();
			//cout << arr.back().AsStdString() << endl;
			cout << arr.size();
		}
	}

	void TestParseObject()
	{
		static const char* TestGroup[] = {
			"{ \"name\" : \"cxk\" , \"age\" : 38, \n \"gender\" : \"unkown\", \"hobbies\" : [\"singing\", \"jump\", \"rap\"] \n}"
		};
		YTCJson::Parser parser;
		cout << "Object-Parse Test>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
		auto json = TestGroup[0];
		parser.Parse(json);
		auto dict = parser.GetValue().AsDictionary();

		cout << dict["name"].AsStdString() << endl;
		cout << dict["age"].AsNumber<int>() << endl;
		cout << dict["gender"].AsStdString() << endl;
		auto arr = dict["hobbies"].AsArray();
		for(const auto& s : arr)
		{
			cout << s.AsStdString() << endl;
		}

	}

    void Start()
    {
        TestParseNumber();
        TestParseString();
		TestParseArray();
		TestParseObject();
    }
};
int main()
{
    Test t;
    t.Start();
    return 0;
}