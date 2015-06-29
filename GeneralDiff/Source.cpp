#include "rapidjson/document.h"
#include <iostream>
using namespace rapidjson;

namespace GeneralDiff
{
	static const char* s_json = R"(
{
    "string" : "foo",
    "number" : 123,
    "array" : [
        0,
        1,
        2,
        3
    ],
    "object" : {
        "v0" : "bar",
        "v1" : 456,
        "v2" : 0.123
    }
}
)";

	int Execute(int argc, const char* argv[])
	{

		Document document;

		document.Parse(s_json);
		bool error = document.HasParseError();
		if (error)
		{
			std::cerr << "parse error.\n";
			return -1;
		}


		// string
		auto it = document.FindMember("string");
		if(it != document.MemberEnd())
		{
			const Value& v = it->value;
			printf("string = %s\n", v.GetString());
		}

		// number
		{
			int v = document["number"].GetInt();
			printf("number = %d\n", v);
		}

		// array
		{
			const Value& a = document["array"];
			SizeType num = a.Size();

			for (SizeType i = 0; i < num; i++){
				int v = a[i].GetInt();
				printf("array[%d] = %d\n", i, v);
			}
		}

		// object
		{
			const Value& o = document["object"];

			// enumerate members in object
			for (Value::ConstMemberIterator itr = o.MemberBegin();
				 itr != o.MemberEnd(); itr++)
			{
				const char* name = itr->name.GetString();
				const Value& value = itr->value;
				Type type = value.GetType();

				printf("%s = ", name);
				switch (type){
				case kStringType:
					printf("%s", value.GetString());
					break;

				case kNumberType:
					if (value.IsDouble())
						printf("%f", value.GetDouble());
					else
						printf("%d", value.GetInt());
					break;

				default:
					printf("(unknown type)");
					break;
				}
				printf("\n");
			}
		}

		


		return 0;
	}
}

int main(int argc, const char* argv[])
{
	return GeneralDiff::Execute(argc, argv);
}