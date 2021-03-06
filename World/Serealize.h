#pragma once
#ifndef SERIALIZE_H
#define SERIALIZE_H
#include <string>
#include <sstream>
#include <functional>
#include <boost\format.hpp>
#include <glm\glm.hpp>

#include "rapidjson\document.h"
#include <tools\StringIntern.h>

#define NVP(T) sge::make_nvp(#T, T)
#define JSONLOAD(...) DeserializeHelper::deserialize(val, __VA_ARGS__)
#define BINSAVE(...) BinSaveHelper::bserialize(val, __VA_ARGS__)
#define BINLOAD(...) BinLoadHelper::bdeserialize(val, __VA_ARGS__)

namespace sge {
	template <class T>
	std::pair<const char *, T> make_nvp(const char *name, T &&value) {
		return{ name, std::forward<T>(value) };
	}
}

class DeserializeHelper
{
public:
	static void deserialize(const rapidjson::Value &val)
	{
		(void)val;
	}

	template <typename Last>
	static void deserialize(const rapidjson::Value &val, const Last &last)
	{
		__deserialize(val, last.first, last.second);
	}

	template <typename First, typename... Rest>
	static void deserialize(const rapidjson::Value &val, const First &first, const Rest&... rest)
	{
		__deserialize(val, first.first, first.second);
		deserialize(val, rest...);
	}
};

namespace {
	template<typename _Ty>
	void __deserialize_array_part(const rapidjson::Value &val, _Ty &target)
	{
		target.JsonLoad(val);
	}

	template<typename _Ty>
	void __deserialize_array_part(const rapidjson::Value &arr, std::vector<_Ty> &target)
	{
		if (arr.IsArray())
		{
			for (decltype(arr.Size()) i = 0; i < arr.Size(); i++)
			{
				_Ty part;
				__deserialize_array_part(arr[i], part);
				target.push_back(std::move(part));
			}
		}
	}

	template<typename _Ty>
	void __deserialize(const rapidjson::Value &val, const char *s, _Ty &target)
	{
		if (!val.HasMember(s))
			return;

		const rapidjson::Value &v = val[s];
		target.JsonLoad(v);
	}

	template<typename _Ty>
	void __deserialize(const rapidjson::Value &val, const char *s, std::shared_ptr<_Ty> &target)
	{
		if (!val.HasMember(s))
			return;

		const rapidjson::Value &v = val[s];
		target->JsonLoad(v);
	}

	template<typename _Ty>
	void __deserialize(const rapidjson::Value &val, const char *s, std::vector<_Ty> &target)
	{

		if (val.HasMember(s) && val[s].IsArray())
		{
			const rapidjson::Value &arr = val[s];
			for (decltype(arr.Size()) i = 0; i < arr.Size(); i++)
			{
				_Ty part;
				__deserialize_array_part(arr[i], part);
				target.push_back(std::move(part));
			}
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, int &target)
	{
		if (val.HasMember(s))
		{
			if (!val[s].IsInt())
				throw std::invalid_argument((boost::format("value %1% is not a integer") % s).str());
			target = val[s].GetInt();
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, std::string &target)
	{
		if (val.HasMember(s))
		{
			if (!val[s].IsString())
				throw std::invalid_argument((boost::format("value %1% is not a string") % s).str());
			target = val[s].GetString();
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, StringIntern &target)
	{
		if (val.HasMember(s))
		{
			if (!val[s].IsString())
				throw std::invalid_argument((boost::format("value %1% is not a string") % s).str());
			target = StringIntern(val[s].GetString());
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, float &target)
	{
		if (val.HasMember(s))
		{
			if (!val[s].IsNumber())
				throw std::invalid_argument((boost::format("value %1% is not a number") % s).str());
			target = static_cast<float>(val[s].GetDouble());
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, bool &target)
	{
		if (val.HasMember(s))
		{
			if (!val[s].IsBool_())
				throw std::invalid_argument((boost::format("value %1% is not a bool") % s).str());
			target = val[s].GetBool_();
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, glm::vec2 &target)
	{
		if (val.HasMember(s) && val[s].IsArray())
		{
			const rapidjson::Value &arr = val[s];
			if (arr.Size() != 2)          throw std::invalid_argument((boost::format("value %1% is not vec2") % s).str());
			if (!arr.Begin()->IsNumber()) throw std::invalid_argument((boost::format("value %1%[0] is not a number") % s).str());
			if (!arr[1].IsNumber())       throw std::invalid_argument((boost::format("value %1%[1] is not a number") % s).str());

			target.x = static_cast<float>(arr.Begin()->GetDouble());
			target.y = static_cast<float>(arr[1].GetDouble());
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, glm::vec3 &target)
	{
		if (val.HasMember(s) && val[s].IsArray())
		{
			const rapidjson::Value &arr = val[s];
			if (arr.Size() != 3)          throw std::invalid_argument("value is not vec3");
			if (!arr.Begin()->IsNumber()) throw std::invalid_argument((boost::format("value %1%[0] is not a number") % s).str());
			if (!arr[1].IsNumber())       throw std::invalid_argument((boost::format("value %1%[1] is not a number") % s).str());
			if (!arr[2].IsNumber())       throw std::invalid_argument((boost::format("value %1%[2] is not a number") % s).str());

			target.x = static_cast<float>(arr.Begin()->GetDouble());
			target.y = static_cast<float>(arr[1].GetDouble());
			target.z = static_cast<float>(arr[2].GetDouble());
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, glm::vec4 &target)
	{
		if (val.HasMember(s) && val[s].IsArray())
		{
			const rapidjson::Value &arr = val[s];
			if (arr.Size() != 4)          throw std::invalid_argument("target variable is not vec4");
			if (!arr.Begin()->IsNumber()) throw std::invalid_argument((boost::format("value %1%[0] is not a number") % s).str());
			if (!arr[1].IsNumber())       throw std::invalid_argument((boost::format("value %1%[1] is not a number") % s).str());
			if (!arr[2].IsNumber())       throw std::invalid_argument((boost::format("value %1%[2] is not a number") % s).str());
			if (!arr[3].IsNumber())       throw std::invalid_argument((boost::format("value %1%[3] is not a number") % s).str());

			target.x = static_cast<float>(arr.Begin()->GetDouble());
			target.y = static_cast<float>(arr[1].GetDouble());
			target.z = static_cast<float>(arr[2].GetDouble());
			target.w = static_cast<float>(arr[3].GetDouble());
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, std::vector<int> &target)
	{
		if (val.HasMember(s))
		{
			const rapidjson::Value &arr = val[s];
			for (decltype(arr.Size()) i = 0; i < arr.Size(); i++)
			{
				if (!arr[i].IsInt())
					throw std::invalid_argument((boost::format("value %1%[%2%] is not a integer") % s % i).str());
				target.push_back(arr[i].GetInt());
			}
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, std::vector<std::string> &target)
	{
		if (val.HasMember(s) && val[s].IsArray())
		{
			const rapidjson::Value &arr = val[s];
			for (decltype(arr.Size()) i = 0; i < arr.Size(); i++)
			{
				if (!arr[i].IsString())
					throw std::invalid_argument((boost::format("value %1%[%2%] is not a string") % s % i).str());
				target.push_back(arr[i].GetString());
			}
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, std::vector<StringIntern> &target)
	{
		if (val.HasMember(s) && val[s].IsArray())
		{
			const rapidjson::Value &arr = val[s];
			for (decltype(arr.Size()) i = 0; i < arr.Size(); i++)
			{
				if (!arr[i].IsString())
					throw std::invalid_argument((boost::format("value %1%[%2%] is not a string") % s % i).str());
				target.push_back(StringIntern(arr[i].GetString()));
			}
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, std::vector<bool> &target)
	{
		if (val.HasMember(s) && val[s].IsArray())
		{
			const rapidjson::Value &arr = val[s];
			for (decltype(arr.Size()) i = 0; i < arr.Size(); i++)
			{
				if (!arr[i].IsBool_())
					throw std::invalid_argument((boost::format("value %1%[%2%] is not a bool") % s % i).str());
				target.push_back(arr[i].GetBool_());
			}
		}
	}

	template<>
	void __deserialize(const rapidjson::Value &val, const char *s, std::vector<float> &target)
	{
		if (val.HasMember(s) && val[s].IsArray())
		{
			const rapidjson::Value &arr = val[s];
			for (decltype(arr.Size()) i = 0; i < arr.Size(); i++)
			{
				if (!arr[i].IsDouble())
					throw std::invalid_argument((boost::format("value %1%[%2%] is not a number") % s % i).str());
				target.push_back(static_cast<float>(arr[i].GetDouble()));
			}
		}
	}
}

class BinSaveHelper
{
public:
	static void bserialize(std::ostream &val)
	{
		(void)val;
	}

	template <typename Last>
	static void bserialize(std::ostream &val, const Last &last)
	{
		__bserialize(val, last);
	}

	template <typename First, typename... Rest>
	static void bserialize(std::ostream &val, const First &first, const Rest&... rest)
	{
		__bserialize(val, first);
		bserialize(val, rest...);
	}
};

namespace {
	template<typename _Ty>
	void __bserialize(std::ostream &val, const _Ty &target)
	{
		target.BinSave(val);
	}

	template<typename _Ty>
	void __bserialize(std::ostream &val, const std::shared_ptr<_Ty> &target)
	{
		__bserialize(val, !!target);
		if(target)
			target->BinSave(val);
	}

	template<typename _Ty>
	void __bserialize(std::ostream &val, const std::vector<_Ty> &target)
	{
		__bserialize(val, target.size());
		for (decltype(target.size()) i = 0; i < target.size(); ++i)
		{
			__bserialize(val, target[i]);
		}
	}

	template<>
	void __bserialize(std::ostream &val, const int &target)
	{
		val.write(reinterpret_cast<const char *>(&target), sizeof(target));
	}

	template<>
	void __bserialize(std::ostream &val, const unsigned short &target)
	{
		val.write(reinterpret_cast<const char *>(&target), sizeof(target));
	}

	template<>
	void __bserialize(std::ostream &val, const bool &target)
	{
		val.write(reinterpret_cast<const char *>(&target), sizeof(target));
	}

	template<>
	void __bserialize(std::ostream &val, const unsigned int &target)
	{
		val.write(reinterpret_cast<const char *>(&target), sizeof(target));
	}

	template<>
	void __bserialize(std::ostream &val, const float &target)
	{
		val.write(reinterpret_cast<const char *>(&target), sizeof(target));
	}

	template<>
	void __bserialize(std::ostream &val, const std::string &target)
	{
		__bserialize(val, target.size());
		val.write(target.c_str(), target.size());
	}

	template<>
	void __bserialize(std::ostream &val, const StringIntern &target)
	{
		__bserialize(val, target.get());
	}

	template<>
	void __bserialize(std::ostream &val, const glm::vec2 &target)
	{
		__bserialize(val, target.x);
		__bserialize(val, target.y);
	}

	template<>
	void __bserialize(std::ostream &val, const glm::vec3 &target)
	{
		__bserialize(val, target.x);
		__bserialize(val, target.y);
		__bserialize(val, target.z);
	}

	template<>
	void __bserialize(std::ostream &val, const glm::vec4 &target)
	{
		__bserialize(val, target.x);
		__bserialize(val, target.y);
		__bserialize(val, target.z);
		__bserialize(val, target.w);
	}

	template<>
	void __bserialize(std::ostream &val, const glm::ivec2 &target)
	{
		__bserialize(val, target.x);
		__bserialize(val, target.y);
	}

	template<>
	void __bserialize(std::ostream &val, const glm::ivec3 &target)
	{
		__bserialize(val, target.x);
		__bserialize(val, target.y);
		__bserialize(val, target.z);
	}

	template<>
	void __bserialize(std::ostream &val, const glm::ivec4 &target)
	{
		__bserialize(val, target.x);
		__bserialize(val, target.y);
		__bserialize(val, target.z);
		__bserialize(val, target.w);
	}
}

class BinLoadHelper
{
public:
	static void bdeserialize(std::istream &val)
	{
		(void)val;
	}

	template <typename Last>
	static void bdeserialize(std::istream &val, Last &last)
	{
		__bdeserialize(val, last);
	}

	template <typename First, typename... Rest>
	static void bdeserialize(std::istream &val, First &first, Rest&... rest)
	{
		__bdeserialize(val, first);
		bdeserialize(val, rest...);
	}
};

namespace {
	template<typename _Ty>
	void __bdeserialize(std::istream &val, _Ty &target)
	{
		target.BinLoad(val);
	}

	template<typename _Ty>
	void __bdeserialize(std::istream &val, std::shared_ptr<_Ty> &target)
	{
		bool exist;
		__bdeserialize(val, exist);
		if (exist)
			target->BinLoad(val);
		else
			target = nullptr;
	}

	template<typename _Ty>
	void __bdeserialize(std::istream &val, std::vector<_Ty> &target)
	{
		size_t size;
		__bdeserialize(val, size);
		target.resize(size);

		for (decltype(target.size()) i = 0; i < target.size(); ++i)
		{
			__bdeserialize(val, target[i]);
		}
	}

	template<>
	void __bdeserialize(std::istream &val, int &target)
	{
		val.read(reinterpret_cast<char *>(&target), sizeof(target));
	}

	template<>
	void __bdeserialize(std::istream &val, unsigned short &target)
	{
		val.read(reinterpret_cast<char *>(&target), sizeof(target));
	}

	template<>
	void __bdeserialize(std::istream &val, bool &target)
	{
		val.read(reinterpret_cast<char *>(&target), sizeof(target));
	}

	template<>
	void __bdeserialize(std::istream &val, unsigned int &target)
	{
		val.read(reinterpret_cast<char *>(&target), sizeof(target));
	}

	template<>
	void __bdeserialize(std::istream &val, float &target)
	{
		val.read(reinterpret_cast<char *>(&target), sizeof(target));
	}

	template<>
	void __bdeserialize(std::istream &val, std::string &target)
	{
		size_t size;
		__bdeserialize(val, size);

		target.resize(size);
		val.read(&target[0], size);
	}

	template<>
	void __bdeserialize(std::istream &val, StringIntern &target)
	{
		std::string s;
		__bdeserialize(val, s);
		target = StringIntern(s);
	}

	template<>
	void __bdeserialize(std::istream &val, glm::vec2 &target)
	{
		__bdeserialize(val, target.x);
		__bdeserialize(val, target.y);
	}

	template<>
	void __bdeserialize(std::istream &val, glm::vec3 &target)
	{
		__bdeserialize(val, target.x);
		__bdeserialize(val, target.y);
		__bdeserialize(val, target.z);
	}

	template<>
	void __bdeserialize(std::istream &val, glm::vec4 &target)
	{
		__bdeserialize(val, target.x);
		__bdeserialize(val, target.y);
		__bdeserialize(val, target.z);
		__bdeserialize(val, target.w);
	}

	template<>
	void __bdeserialize(std::istream &val, glm::ivec2 &target)
	{
		__bdeserialize(val, target.x);
		__bdeserialize(val, target.y);
	}

	template<>
	void __bdeserialize(std::istream &val, glm::ivec3 &target)
	{
		__bdeserialize(val, target.x);
		__bdeserialize(val, target.y);
		__bdeserialize(val, target.z);
	}

	template<>
	void __bdeserialize(std::istream &val, glm::ivec4 &target)
	{
		__bdeserialize(val, target.x);
		__bdeserialize(val, target.y);
		__bdeserialize(val, target.z);
		__bdeserialize(val, target.w);
	}
}

#endif // SERIALIZE_H