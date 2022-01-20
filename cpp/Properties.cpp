/* Copyright (C) 2021-2022 Zukaritasu

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Properties.hpp"

#include <iostream>
#include <fstream>

#include <cstdlib>
#include <cstring>

Properties::~Properties()
{
	for (auto elem : data)  {
		if (elem != nullptr) {
			elem->~basic_string();
		}
	}
}

bool Properties::Contains(const std::string& key)
{
	return IndexOf(key) != UINT32_MAX;
}

size_t Properties::IndexOf(const std::string& key)
{
	for (size_t i = 0; i < this->data.size(); i += 2) {
		if (data[i]->compare(key) == 0) {
			return i;
		}
	}
	return UINT32_MAX;
}

bool Properties::IsEmpty(const std::string& key)
{
	for (auto c : key) {
		if (!isblank(c)) {
			return false;
		}
	}
	return true;
}

std::string* Properties::Convert(const std::string &value)
{
	std::string* str = new std::string;
	for (size_t i = 0; i < value.size(); i++) {
		if (value[i] == '\\') {
			if (i + 1 == value.size())
				break;
			switch (value[++i]) {
			case 't': *str += '\t'; break;
			case 'f': *str += '\f'; break;
			case 'r': *str += '\r'; break;
			case 'n': *str += '\n'; break;
			}
			continue;
		}

		*str += value[i];
	}
	return str;
}

void Properties::ResolveString(std::ofstream& out, 
							   const std::string& value)
{
	for (size_t i = 0; i < value.size(); i++) {
		switch (value[i]) {
		case '\t': out << "\\t"; break;
		case '\f': out << "\\f"; break;
		case '\r': out << "\\r"; break;
		case '\n': out << "\\n"; break;
		default:
			out << value[i];
			break;
		}
	}
}

void Properties::AddValue(const std::string &value, bool is_value)
{
	if (is_value) {
		data.push_back(value.size() == 0 ? nullptr : Convert(value));
	} else {
		data.push_back(new std::string(value));
	}
}

void Properties::AddProperty(const std::string &line)
{
	bool space = true;
	bool cp_key = true;

	std::string temp;

	for (size_t i = 0; i < line.size(); i++) {
		char c = line[i];
		if (cp_key && (c == '=' || c == ':')) {
		_value:
			if (temp.size() == 0)
				break;
			AddValue(temp, false);
			temp.clear();
			cp_key = false;
			continue;
		}

		if (space && isblank((unsigned char)c)) {
			if (cp_key && i + 1 < line.size()) {
				c = line[i + 1];
				if (!isblank((unsigned char)c) && c != '=' && c != ':') {
					goto _value;
				}
			}
			continue;
		}

		if (!cp_key && space) {
			space = false;
		}

		temp += c;

		if (!cp_key && i + 1 == line.size()) {
			std::string aux = temp;
			for (int i = (int)(aux.size() - 1); i >= 0; i--) {
				if (!isblank((unsigned char)aux[i]))
					break;
				temp = aux.substr(0, i);
			}
		}
	}

	if (cp_key) {
		if (temp.size() != 0) {
			AddValue(temp, false);
			AddValue("", true);
		}
	} else {
		AddValue(temp, true);
	}
}

void Properties::RemoveRange(uint32_t begin, uint32_t end)
{
	for (uint32_t i = begin; i < end; i++) {
		if (data[i] != nullptr) {
			data[i]->~basic_string();
		}
	}
	data.erase(data.begin() + begin, data.begin() + end);
}

bool Properties::ReadLine(std::string& line, std::ifstream& in)
{
	int c;
	bool begin = true;
	bool comment = false;
	line.clear();

	while (true) {
		if ((c = in.get()) == EOF || c == '\n' || c == '\r') {
			if (comment && c != EOF) {
				begin = true;
				comment = false;
				continue;
			}
			break;
		}

		if (begin) {
			if (begin = isblank(c) != 0) {
				continue;
			} else if (c == '#' || c == ';') {
				comment = true;
			}
		}

		if (!comment) {
			line += (char)c;
		}
	}

	return c != EOF;
}

bool Properties::Remove(const std::string &key)
{
	size_t index = this->IndexOf(key);
	if (index != UINT32_MAX) {
		RemoveRange(index, index + 2);
		return true;
	}
	return false;
}

bool Properties::Replace(const std::string &key, const std::string &value)
{
	size_t index = IndexOf(key);
	if (index++ != UINT32_MAX) {
		RemoveRange(index, index + 1);
		data.insert(data.begin() + index, value.size() == 0 ? nullptr : 
			new std::string(value));
	}
	return false;
}

bool Properties::AddValueForType(const std::string &key, 
								 const std::string &value)
{
	if (IndexOf(key) == UINT32_MAX && !IsEmpty(key)) {
		data.push_back(new std::string(key));
		data.push_back(new std::string(value));
	}
	return false;
}

size_t Properties::Size() const
{
	return data.size() == 0 ? 0 : data.size() / 2;
}

bool Properties::Load(const std::string& filename)
{
	std::ifstream in(filename);
	if (in.is_open()) {
		std::string line;
		while (ReadLine(line, in) || line.size() > 0) {
			AddProperty(line);
		}
		bool error = in.fail();
		in.close();
		return !error;
	}
	return false;
}

bool Properties::Save(const std::string& filename)
{
	std::ofstream out(filename);
	if (out.is_open()) {
		for (size_t i = 1; !out.fail() && i < data.size(); i += 2) {
			out << *(data[i - 1]) << '=';
			if (data[i] != nullptr) {
				ResolveString(out, *(data[i]));
			}
			if (i + 2 < data.size()) {
				out << std::endl;
			}
		}
		bool error = out.fail();
		out.close();
		return !error;
	}
	return false;
}

void Properties::Print() const
{
	for (size_t i = 1; i < data.size(); i += 2) {
		const char* value = data[i] == nullptr ? "" : data[i]->c_str();
		std::cout << *(data[i - 1]) << '=' << value << '\n';
	}
}

const char* Properties::GetValue(const std::string &key)
{
	size_t i = IndexOf(key);
	return (i != UINT32_MAX && 
		data[i + 1] != nullptr) ? 
		data[i + 1]->c_str(): nullptr;
}
