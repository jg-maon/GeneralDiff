#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include <iostream>
#include <vector>
#include <conio.h>
#include <unordered_map>
#include <string>
using namespace rapidjson;

#define MAP
namespace GeneralDiff
{
#ifdef MAP
	typedef std::unordered_map<std::string, Value*> Map;
#else
	typedef std::vector<std::pair<std::string, Value*>> Map;
#endif
	static const char* s_json = R"(
{
	"Shader":
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
	},
	"Test":
	{
		"@IgnoreCondition":{"name": "Used", "value" : false}
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
			stream << std::boolalpha << value.GetBool();
			break;
		case rapidjson::kObjectType:
		{
			stream << std::endl;
			//stream << "object:" << std::endl;
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
			stream << "\"" << value.GetString() << "\"";
			break;
		case rapidjson::kNumberType:
			stream << value.GetDouble();
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

	enum class ValueType
	{
		Null,	
		Number,
		Object,
		Array,
		Boolean,
		String,
	};
	struct Node
	{

		virtual double GetDouble() const = 0;
		virtual bool GetBool() const = 0;
		virtual std::string GetString() const = 0;
		virtual ValueType GetType() const = 0;

		virtual Value& operator[](unsigned int index) = 0;
		virtual const Value& operator[](unsigned int index) const = 0;
		virtual Value& operator[](const std::string& name) = 0;
		virtual const Value& operator[](const std::string& name) const = 0;

		virtual size_t Size() const = 0;


		virtual size_t MemberCount() const = 0;



		virtual bool IsArray(){ return false; }
		virtual bool IsBool() { return false; }
		virtual bool IsNumber() { return false; }
		virtual bool IsString() { return false; }
		virtual bool IsObject() { return false; }

		virtual void SetRootNode(const Value& value) = 0;

		std::vector<Node*> children;
		//const Value& value;
		const Node* pParent;
		//Node(const Value& value, const Node* pParent = nullptr)
		//	: value(value)
		//	, pParent(pParent)
		//{}
	};

	class RapidNode : public Node
	{
	public:
		RapidNode(rapidjson::Value& value) :m_value(value){}
		virtual ~RapidNode()
		{
			_ClearChildrenRecursively(*this);
		}
		virtual double GetDouble() const override{	return m_value.GetDouble();	}
		virtual bool GetBool() const override{ return m_value.GetBool(); }
		virtual std::string GetString() const override{ return m_value.GetString(); }
		virtual ValueType GetType() const override
		{
			switch (m_value.GetType())
			{
			case rapidjson::kFalseType:
			case rapidjson::kTrueType:
				return ValueType::Boolean;

			case rapidjson::kObjectType:
				return ValueType::Object;

			case rapidjson::kArrayType:
				return ValueType::Array;

			case rapidjson::kStringType:
				return ValueType::String;

			case rapidjson::kNumberType:
				return ValueType::Number;

			case rapidjson::kNullType:
			default:
				return ValueType::Null;
			}
		}

		virtual Value& operator[](unsigned int index) override{ return m_value[index]; }
		virtual const Value& operator[](unsigned int index) const override{ return m_value[index]; }
		virtual Value& operator[](const std::string& name) override{ return m_value[name.c_str()]; }
		virtual const Value& operator[](const std::string& name) const override{ return m_value[name.c_str()]; }

		virtual size_t Size() const override{ return m_value.Size(); }


		virtual size_t MemberCount() const override{ return m_value.MemberCount(); }



		virtual bool IsArray(){ return m_value.IsArray(); }
		virtual bool IsBool() { return m_value.IsBool(); }
		virtual bool IsNumber() { return m_value.IsNumber(); }
		virtual bool IsString() { return m_value.IsString(); }
		virtual bool IsObject() { return m_value.IsObject(); }

		virtual void SetRootNode(const Value& value) override
		{
			if (!value.IsObject())
			{
				return;
			}

			_ClearChildrenRecursively(*this);
			


		}

	private:

		void _AddChildrenRecursively(rapidjson::Value& value, Node* pParent)
		{
			RapidNode* pNode = new RapidNode(value);
			pParent->children.emplace_back(pNode);
			if (value.IsObject())
			{
				for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it)
				{
					_AddChildrenRecursively(it->value, pNode);
				}
			}
			else if (value.IsArray())
			{ 
				for (auto it = value.Begin(); it != value.End(); ++it)
				{
					_AddChildrenRecursively(*it, pNode);
				}
			}
		}

		void _ClearChildrenRecursively(Node& node) const
		{
			for (Node* pNode : node.children)
			{
				delete pNode;
			}
			node.children.clear();
		}

	private:
		rapidjson::Value&	m_value;
		
	};


	int add(Map& valueMap, Value& value, const std::string& name)
	{
		if (value.IsObject())
		{
			for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it)
			{
				std::string n = name + "/" + it->name.GetString();
#ifdef MAP
				valueMap.insert(std::make_pair(n, &(it->value)));
#else
				valueMap.push_back(std::make_pair(n, &(it->value)));
#endif
				add(valueMap, it->value, n);
			}
		}
		else if (value.IsArray())
		{
			int i = 0;
			for (auto it = value.Begin(); it != value.End(); ++it)
			{
				std::string n = name + "[" + std::to_string(i++) + "]";
#ifdef MAP
				valueMap.insert(std::make_pair(n, &(*it)));
#else
				valueMap.push_back(std::make_pair(n, &(*it)));
#endif
				add(valueMap, *it, n);
			}
		}
		return 0;
	}

	int Execute(int argc, const char* argv[])
	{

		Document document;

		document.Parse(s_json);
		bool error = document.HasParseError();
		if (error)
		{
			std::cerr << "parse error.\n";
			size_t offset = document.GetErrorOffset();
			ParseErrorCode code = document.GetParseError();
			const char* msg = GetParseError_En(code);
			printf("%d:%d(%s)\n", offset, code, msg);
#define max(a,b) (((a) > (b)) ? (a) : (b))
			offset = max(0, offset -0);
			std::cerr << &s_json[offset] << std::endl;
			_getch();
			return -1;
		}
		printObject(std::cout, document);

		Map valueMap;
		add(valueMap, document, "");

		for (auto& a : valueMap)
		{
			std::cout << "==========================" << a.first << "==========================" << std::endl;
			printObject(std::cout, *(a.second));
			std::cout << std::endl;
		}
		_getch();
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