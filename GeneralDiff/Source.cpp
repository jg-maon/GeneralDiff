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

	void printObject(std::ostream& stream, const Value& value, unsigned int indent = 0)
	{
		static const int tabSpace = 4;
		for (unsigned int i = 0; i < indent * tabSpace; ++i)
		{
			stream << ' ';
		}
		stream << "{" << std::endl;
		for (unsigned int i = 0; i < (indent + 1)*tabSpace; ++i)
		{
			stream << ' ';
		}
		Type type = value.GetType();
		switch (type)
		{
		case rapidjson::kNullType:
			stream << "[NULL]";
			break;
		case rapidjson::kFalseType:
		case rapidjson::kTrueType:
			stream << "boolean:" << std::boolalpha << value.GetBool();
		case rapidjson::kObjectType:
		{
			stream << "object:" << std::endl;
			for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it)
			{
				printObject(stream, it->value, indent + 1);
			}
		}
			break;
		case rapidjson::kArrayType:
		{
			if (!value.Empty())
			{
				stream << "array:[ ";
				for (auto it = value.Begin(); it != value.End(); ++it)
				{

					stream << it->GetDouble() << " ";
					//printObject(stream, *it);
				}
				stream << "]";
			}
		}
			break;
		case rapidjson::kStringType:
			stream << "string:" << value.GetString();
			break;
		case rapidjson::kNumberType:
			stream << "number:" << value.GetDouble();
			break;
		default:
			break;
		}

		stream << std::endl;
		for (unsigned int i = 0; i < indent*tabSpace; ++i)
		{
			stream << ' ';
		}
		stream << "}" << std::endl;
	}

	template<class Value>
	struct Node
	{
		const Value& value;
		const Node* pParent;
		Node(const Value& value, const Node* pParent = nullptr)
			: value(value)
			, pParent(pParent)
		{}
	};

	template<class Value>
	std::string makePath(const Node<Value>& node, const std::string& path)
	{
		if (!node.pParent)
		{
			return path;
		}
		return makePath(*node.pParent, node.value.GetString() + path);
	}

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
		printObject(std::cout, document);


		return 0;

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