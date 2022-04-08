// Copyright (C) 2021-2022 Zukaritasu
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <string>
#include <vector>
#include <cstdint>

#define PROP_USE_GEOMETRY 1

// In the event that the project implements GUIs, it is possible to save
// the size and position of the window, among other controls.
#if PROP_USE_GEOMETRY || defined(ZPROP_CPP)

struct Size
{
public:
	Size() = default;
	Size(int width, int height) : width(width), height(height) {}

	int width;
	int height;
};

struct Point
{
public:
	Point() = default;
	Point(int x, int y) : x(x), y(y) {}

	int x;
	int y;
};

struct Rect
{
public:
	Rect() = default;
	Rect(const Size& size)
		: width(size.width), height(size.height) {}
	Rect(int width, int height)
		: width(width), height(height) {}
	Rect(int x, int y, int width, int height)
		: x(x), y(y), width(width), height(height) {}
	Rect(const Point& p, const Size& s)
		: x(p.x), y(p.y), width(s.width), height(s.height) {}

	int x;
	int y;
	int width;
	int height;
};

#endif // PROP_USE_GEOMETRY || defined(ZPROP_CPP)

namespace std {

	// Do not use these functions!!! these functions were implemented
	// so that the Properties::Add template can work with multiple values
	
	inline string to_string(const string& value) 
	{ return value; }

	inline string to_string(char value) 
	{
		string str;
		str += value;
		return str;
	}

#if PROP_USE_GEOMETRY || defined(ZPROP_CPP)
	string to_string(const Rect& rect);
	string to_string(const Size& size);
	string to_string(const Point& point);
#endif // PROP_USE_GEOMETRY || defined(ZPROP_CPP)
}

class Properties
{
public:

	class Elem final
	{
	private:

		std::string key;
		const char* value = nullptr;
		Properties* props = nullptr;

		friend class Properties;

	private:

		Elem() = delete;
		Elem(const Elem&) = delete;
		Elem(Properties* props) { this->props = props; }

		Elem& Set(const std::string& key, const char* value = nullptr)
		{
			this->key = key;
			this->value = value;
			return (*this);
		}

		void StringNumbersToArray(int* arr, int c) const;
		
		Rect GetRect() const;
		Size GetSize() const;
		Point GetPoint() const;

	public:
	
		operator int() const
		{ return value ? std::stoi(value) : 0; }
		operator unsigned int() const
		{ return value ? std::stoul(value) : 0; }
		operator long long() const
		{ return value ? std::stoll(value) : 0; }
		operator unsigned long long() const
		{ return value ? std::stoull(value) : 0; }
		operator double() const
		{ return value ? std::stod(value) : 0; }
		operator float() const
		{ return value ? std::stof(value) : 0; }
		operator long double() const
		{ return value ? std::stold(value) : 0; }
		operator char() const
		{ return value ? value[0] : '\0'; }
		operator const char*() const
		{ return value; }
		operator Rect() const
		{ return GetRect(); }
		operator Size() const
		{ return GetSize(); }
		operator Point() const
		{ return GetPoint(); }

		operator bool() const
		{
			return value ? std::strcmp(value, "true") == 0 ||
				value[0] == '1' : false;
		}

		Elem& operator=(int i)
		{
			props->Add(key, i);
			return (*this);
		}
		Elem& operator=(unsigned int ui)
		{
			props->Add(key, ui);
			return (*this);
		}
		Elem& operator=(long long ll)
		{
			props->Add(key, ll);
			return (*this);
		}
		Elem& operator=(unsigned long long ull)
		{
			props->Add(key, ull);
			return (*this);
		}
		Elem& operator=(double d)
		{
			props->Add(key, d);
			return (*this);
		}
		Elem& operator=(float f)
		{
			props->Add(key, f);
			return (*this);
		}
		Elem& operator=(long double ld)
		{
			props->Add(key, ld);
			return (*this);
		}

		Elem& operator=(bool b)
		{
			props->Add(key, b);
			return (*this);
		}

		Elem& operator=(const char* value)
		{
			props->Add(key, value);
			return (*this);
		}

		Elem& operator=(const std::string& value)
		{
			props->Add(key, value);
			return (*this);
		}
		
		Elem& operator=(const Rect& rect)
		{
			props->Add(key, rect);
			return (*this);
		}
		
		Elem& operator=(const Point& point)
		{
			props->Add(key, point);
			return (*this);
		}
		
		Elem& operator=(const Size& size)
		{
			props->Add(key, size);
			return (*this);
		}
	};
protected:

	std::vector<std::string*> props;
private:

	Elem* elem = nullptr;

	int IndexOf(const std::string& key);
	bool IsEmpty(const std::string& key);
	bool ReadLine(std::string& line, std::ifstream& in);
	std::string* Convert(const std::string &value);
	void WriteValue(std::ofstream& out, const std::string& value);
	void AddValue(const std::string &value, bool is_value = false);
	void AddProperty(const std::string &line);
	void RemoveRange(int begin, int end);
	bool AddValueType(const std::string& key, const std::string& value);
	bool Replace(const std::string& key, const std::string& value);

	bool Load(std::ifstream& in);
	bool Save(std::ofstream& out);
public:

	Properties() { elem = new Elem(this); }
	Properties(const Properties&) = delete;
	~Properties();

	bool Contains(const std::string& key);
	bool Remove(const std::string& key);

	template<typename V> 
	bool Add(const std::string& key, V value) 
	{ return AddValueType(key, std::to_string(value)); }

	int Size() const;

	bool Load(const std::string& filename);
	bool Load(const std::wstring& filename);
	bool Save(const std::string& filename);
	bool Save(const std::wstring& filename);
	void Print() const;

	const char* GetValue(int index);
	const char* GetValue(const std::string& key);

	Elem& GetElem(int index);
	Elem& GetElem(const std::string& key)
	{ return elem->Set(key, GetValue(key)); }

	// int value = prop[index];
	Elem& operator[](int index)
	{ return GetElem(index); }
	// prop["key"] = 1; or int value = prop["key"];
	Elem& operator[](const std::string& key) 
	{ return GetElem(key); }
};
