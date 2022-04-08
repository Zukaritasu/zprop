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

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#endif

#include "Properties.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>


Properties::~Properties()
{
	if (elem) 
	{
		elem->~Elem();
		elem = nullptr;
	}
	for (auto elem_prop : props)  
	{
		if (elem_prop != nullptr)
		{
			elem_prop->~basic_string();
		}
	}
}

bool Properties::Contains(const std::string& key)
{
	return IndexOf(key) != -1;
}

int Properties::IndexOf(const std::string& key)
{
	for (size_t i = 0; i < this->props.size(); i += 2) 
	{
		if ((*props[i]) == key) 
		{
			return (int)i;
		}
	}
	return -1;
}

bool Properties::IsEmpty(const std::string& key)
{
	for (auto c : key) 
	{
		if (!isblank(c)) 
		{
			return false;
		}
	}
	return true;
}

std::string* Properties::Convert(const std::string& value)
{
	std::string* str = new std::string;
	for (size_t i = 0; i < value.size(); i++) 
	{
		if (value[i] == '\\') 
		{
			if (i + 1 == value.size())
				break;
			switch (value[++i]) 
			{
			case 't': *str += '\t'; break;
			case 'f': *str += '\f'; break;
			case 'r': *str += '\r'; break;
			case 'n': *str += '\n'; break;
			case '\\': *str += '\\'; break;
			}
			continue;
		}

		*str += value[i];
	}
	return str;
}

void Properties::WriteValue(std::ofstream& out, 
							const std::string& value)
{
	for (size_t i = 0; i < value.size() && !out.fail(); i++)
	{
		switch (value[i]) 
		{
		case '\t': out << "\\t"; break;
		case '\f': out << "\\f"; break;
		case '\r': out << "\\r"; break;
		case '\n': out << "\\n"; break;
		case '\\': out << "\\\\"; break;
		default:
			out << value[i];
			break;
		}
	}
}

void Properties::AddValue(const std::string &value, bool is_value)
{
	if (is_value)
		props.push_back(value.size() == 0 ? nullptr : Convert(value));
	else
		props.push_back(new std::string(value));
}

void Properties::AddProperty(const std::string &line)
{
	bool space = true;
	bool cp_key = true;

	std::string temp;

	for (size_t i = 0; i < line.size(); i++) 
	{
		char c = line[i];
		if (cp_key && (c == '=' || c == ':')) 
		{
		_value:
			if (temp.size() == 0)
				break;
			AddValue(temp);
			temp.clear();
			cp_key = false;
			continue;
		}

		if (space && isblank((unsigned char)c))
		{
			if (cp_key && i + 1 < line.size()) 
			{
				c = line[i + 1];
				if (!isblank((unsigned char)c) && c != '=' && c != ':')
					goto _value;
			}
			continue;
		}

		if (!cp_key && space)
			space = false;

		temp += c;

		if (!cp_key && i + 1 == line.size()) 
		{
			std::string aux = temp;
			for (int i = (int)(aux.size() - 1); i >= 0; i--) 
			{
				if (!isblank((unsigned char)aux[i]))
					break;
				temp = aux.substr(0, i);
			}
		}
	}

	if (cp_key) 
	{
		if (temp.size() != 0) 
		{
			AddValue(temp);
			AddValue("", true);
		}
	} 
	else 
	{
		AddValue(temp, true);
	}
}

void Properties::RemoveRange(int begin, int end)
{
	for (int i = begin; i < end; i++) 
	{
		if (props[i] != nullptr) 
		{
			props[i]->~basic_string();
		}
	}
	props.erase(props.begin() + begin, props.begin() + end);
}

bool Properties::ReadLine(std::string& line, std::ifstream& in)
{
	int c;
	bool begin = true;
	bool comment = false;
	line.clear();

	while (true) 
	{
		if ((c = in.get()) == EOF || c == '\n' || c == '\r') 
		{
			if (comment && c != EOF) 
			{
				begin = true;
				comment = false;
				continue;
			}
			break;
		}

		if (begin) 
		{
			if (begin = isblank(c) != 0)
				continue;
			else if (c == '#' || c == ';')
				comment = true;
		}

		if (!comment)
			line += (char)c;
	}

	return c != EOF;
}

bool Properties::Remove(const std::string &key)
{
	const int index = this->IndexOf(key);
	if (index != -1) 
	{
		RemoveRange(index, index + 2);
		return true;
	}
	return false;
}

bool Properties::Replace(const std::string &key, 
						 const std::string &value)
{
	int index = IndexOf(key);
	if (index++ != -1) 
	{
		RemoveRange(index, index + 1);
		props.insert(props.begin() + index, value.size() == 0 ? nullptr : 
			new std::string(value));
		return true;
	}
	return false;
}

bool Properties::Load(std::ifstream& in)
{
	std::string line;
	while (ReadLine(line, in) || line.size() > 0)
		AddProperty(line);

	bool error = in.fail();
	in.close();
	return !error;
}

bool Properties::Save(std::ofstream& out)
{
	for (size_t i = 1; !out.fail() && i < props.size(); i += 2)
	{
		out << *(props[i - 1]) << '=';
		if (props[i] != nullptr)
			WriteValue(out, *props[i]);
		if (i + 2 < props.size())
			out << std::endl;
	}

	bool error = out.fail();
	out.close();
	return !error;
}

bool Properties::AddValueType(const std::string &key, 
							  const std::string &value)
{
	if (!IsEmpty(key))
	{
		if (Contains(key))
			Replace(key, value);
		else
		{
			props.push_back(new std::string(key));
			props.push_back(new std::string(value));
		}
		return true;
	}
	
	return false;
}

int Properties::Size() const
{
	return props.size() == 0 ? 0 : props.size() / 2;
}

bool Properties::Load(const std::string& filename)
{
	std::ifstream in(filename);
	return in.is_open() ? Load(in) : false;
}

bool Properties::Load(const std::wstring& filename)
{
	std::ifstream in(filename);
	return in.is_open() ? Load(in) : false;
}

bool Properties::Save(const std::string& filename)
{
	std::ofstream out(filename);
	return out.is_open() ? Save(out) : false;
}

bool Properties::Save(const std::wstring& filename)
{
	std::ofstream out(filename);
	return out.is_open() ? Save(out) : false;
}

void Properties::Print() const
{
	for (size_t i = 1; i < props.size(); i += 2) 
	{
		const char* value = props[i] == nullptr ? "" : props[i]->c_str();
		std::cout << *(props[i - 1]) << '=' << value << '\n';
	}
}

const char* Properties::GetValue(int index)
{
	if (index < 0 || index >= this->Size())
		throw std::exception("Index out Properties::GetValue(int index)");
	index = ((index + 1) * 2) - 1;
	if (props[index] != nullptr)
		return props[index]->c_str();
	return nullptr;
}

const char* Properties::GetValue(const std::string &key)
{
	const int i = IndexOf(key);
	return (i != -1 && props[i + 1] != nullptr) ? 
					   props[i + 1]->c_str() : nullptr;
}

Properties::Elem& Properties::GetElem(int index)
{
	if (index < 0 || index >= this->Size())
		throw std::exception("Index out Properties::GetElem(int index)");
	index = ((index + 1) * 2) - 2;

	return elem->Set(*props[index],
		props[index + 1] ? props[index + 1]->c_str() : nullptr);
}

static std::string ArrayToStringNumbers(const int* arr, int c)
{
	std::string numbers;
	for (int i = 0; i < c; i++)
	{
		numbers += std::to_string(arr[i]);
		if (i + 1 < c)
			numbers += ",";
	}
	return numbers;
}

std::string std::to_string(const Rect& rect)
{
	return ArrayToStringNumbers((const int*)&rect, 4);
}

std::string std::to_string(const Size& size)
{
	return ArrayToStringNumbers((const int*)&size, 2);
}

std::string std::to_string(const Point& point)
{
	return ArrayToStringNumbers((const int*)&point, 2);
}

void Properties::Elem::StringNumbersToArray(int* arr, int c) const
{
	memset(arr, 0, sizeof(int) * c);
	if (value)
	{
		int pos = 0;
		char* copy = strdup(value);
		if (copy == nullptr)
		{
			throw std::bad_alloc();
		}
			
		const int orig_len = strlen(copy);
		
		auto NextToken = [&pos, &copy](int orig_len)
		{
			int i, j = 0;
		next:
			if (pos >= orig_len)
				return false;

			for (i = pos; i < orig_len; i++) 
			{
				if (copy[i] == ',')
					break;
				copy[j++] = copy[i];
			}
			
			pos = i + 1;
			copy[j] = '\0';
			if (j == 0)
				goto next;
			return true;
		};
		
		for (int i = 0; i < c; i++)
		{
			if (!NextToken(orig_len))
				break;
			arr[i] = strtol(copy, nullptr, 10);
		}
		
		free(copy);
	}
}

Rect Properties::Elem::GetRect() const
{
	Rect rect{};
	StringNumbersToArray((int*)&rect, 4);
	return rect;
}

Size Properties::Elem::GetSize() const
{
	struct Size size{};
	StringNumbersToArray((int*)&size, 2);
	return size;
}

Point Properties::Elem::GetPoint() const
{
	Point point{};
	StringNumbersToArray((int*)&point, 2);
	return point;
}
