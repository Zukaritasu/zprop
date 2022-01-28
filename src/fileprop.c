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

#include "..\include\fileprop.h"

#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys\stat.h>

// ************************************************************************

typedef struct
{
	size_t bytes;
	size_t count;
	char  *data;
} STRING;

#define STR_REALLOC_BYTES 512

#define OPEN_FILE(flag) \
	(unicode ? _wfopen((const wchar_t *)filename, L##flag) : \
	fopen((const char *)filename, flag))
	
#define NUMBER_TO_STR(type) \
	char buf[100]; \
	sprintf(buf, type, value); \
	return zp_add(prop, key, buf); 
	
#define STR_TO_NUMBER(func) \
	cstr_t value = zp_value(prop, key); \
	if (value != NULL) \
		return func(value, NULL, 10); \
	return 0;

#define STR_TO_NUMBERF(func) \
	cstr_t value = zp_value(prop, key); \
	if (value != NULL) \
		return func(value, NULL); \
	return 0;

// ************************************************************************

static size_t _zp_indexof(pzprop prop, cstr_t key);
static bool   _zp_write(pzprop prop, const void *filename, bool unicode);
static bool   _zp_read(pzprop *prop, const void *filename, bool unicode);
static bool   _zp_realloc(pzprop prop, int count_e);
static bool   _zp_is_empty(cstr_t value);
static bool   _zp_read_line(STRING *line, FILE *file);
static bool   _zp_append(STRING *str, char c);
static void   _zp_app_property(pzprop prop, cstr_t kval, size_t cKval);
static bool   _zp_add_value(pzprop prop, cstr_t value, bool is_value);
static char*  _zp_convert(char *out, cstr_t in);
static bool   _zp_resolve_str(char **out, const char *in);
static bool   _zp_replace(pzprop prop, cstr_t key, cstr_t value);
static size_t _zp_str_length_control(const char *in);
static bool   _zp_substr_token(char* out, size_t begin, size_t end);
static bool   _zp_add_geometry(pzprop prop, cstr_t key, const long* g, size_t c);
static bool   _zp_value_geometry(pzprop prop, cstr_t key, long* g, size_t c);

// ************************************************************************

ZPEXPORT void zp_free(pzprop* prop)
{
	if (prop != NULL && (*prop) != NULL) {
		for (size_t i = 0; i < (*prop)->count; i++)
			free((*prop)->data[i]);
		free((*prop)->data);
		free(*prop);
		(*prop) = NULL;
	}
}

ZPEXPORT void zp_print(pzprop prop)
{
	if (prop != NULL) {
		for (size_t i = 1; i < prop->count; i += 2) {
			printf("%s=%s\n", prop->data[i - 1], 
			prop->data[i] == NULL ? "" : prop->data[i]);
		}
	}
}

ZPEXPORT bool zp_contains(pzprop prop, cstr_t key)
{
	return _zp_indexof(prop, key) != UINT32_MAX;
}

ZPEXPORT pzprop zp_create(void)
{
	return calloc(sizeof(zprop), 1);
}

ZPEXPORT bool zp_remove(pzprop prop, cstr_t key)
{
	size_t i = _zp_indexof(prop, key);
	if (i != UINT32_MAX)
	{
		free(prop->data[i++]);
		free(prop->data[i++]);
		while (i < prop->count)
			prop->data[i - 2] = prop->data[i++];
		return _zp_realloc(prop, -2);
	}
	return false;
}

ZPEXPORT void zp_removeall(pzprop prop)
{
	if (prop != NULL) {
		for (size_t i = 0; i < prop->count; i++)
			free(prop->data[i]);
		prop->count = 0;
	}
}

ZPEXPORT cstr_t zp_value(pzprop prop, cstr_t key)
{
	size_t i = _zp_indexof(prop, key);
	return (i != UINT32_MAX) ? prop->data[i + 1] : NULL;
}

ZPEXPORT bool zp_valueb(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value)
		return strcmp(value, "true") == 0 || value[0] == '1';
	return 0;
}

ZPEXPORT int zp_valuei(pzprop prop, cstr_t key)
{ STR_TO_NUMBER(strtol); }

ZPEXPORT int64_t zp_valuei64(pzprop prop, cstr_t key)
{ STR_TO_NUMBER(strtoll); }

ZPEXPORT ldouble_t zp_valueld(pzprop prop, cstr_t key)
{ STR_TO_NUMBERF(strtold); }

ZPEXPORT uint32_t zp_valueul(pzprop prop, cstr_t key)
{ STR_TO_NUMBER(strtoul); }

ZPEXPORT uint64_t zp_valueull(pzprop prop, cstr_t key)
{ STR_TO_NUMBER(strtoull); }

ZPEXPORT double zp_valued(pzprop prop, cstr_t key)
{ STR_TO_NUMBERF(strtod); }

ZPEXPORT float zp_valuef(pzprop prop, cstr_t key)
{ STR_TO_NUMBERF(strtof); }

ZPEXPORT bool zp_valuer(pzprop prop, cstr_t key, zprect* r)
{
	return r ? _zp_value_geometry(prop, key, (long*)r, 4)
		: false;
}

ZPEXPORT bool zp_valuep(pzprop prop, cstr_t key, zppoint* p)
{
	return p ? _zp_value_geometry(prop, key, (long*)p, 2)
		: false;
}

ZPEXPORT bool zp_valuesz(pzprop prop, cstr_t key, zpsize* sz)
{
	return sz ? _zp_value_geometry(prop, key, (long*)sz, 2)
		: false;
}

static bool _zp_value_geometry(pzprop prop, cstr_t key, long* g, size_t c)
{
	memset(g, 0, sizeof(long) * c);
	char* value = _strdup(zp_value(prop, key));
	if (value != NULL) {
		if (strlen(value) > 0)
		{
			for (size_t i = 0; i < c; i++) {
				char* number = strtok(_strdup(value), ",");
				if (!number)
				{
					free(value);
					return false;
				}
				size_t len = strlen(number);
				g[i] = strtol(number, NULL, 10);
				free(number);

				if (!_zp_substr_token(value, len + 1,
					strlen(value))) {
					break;
				}
			}
		}
		free(value);
		return true;
	}
	return false;
}

ZPEXPORT char zp_valuec(pzprop prop, cstr_t key)
{
	cstr_t str = zp_value(prop, key);
	if (str != NULL)
		return str[0];
	return '\0';
}

static bool _zp_replace(pzprop prop, cstr_t key, cstr_t value)
{
	size_t i = _zp_indexof(prop, key);
	if (i++ != UINT32_MAX) {
		char *text = value != NULL ? _strdup(value) : NULL;
		if (text != NULL || value == NULL) {
			free(prop->data[i]);
			prop->data[i] = _zp_convert(text, text);
			return true;
		}
	}
	return false;
}

ZPEXPORT bool zp_add(pzprop prop, cstr_t key, cstr_t value)
{
	if (prop != NULL && !_zp_is_empty(key)) {
		if (zp_contains(prop, key))
			return _zp_replace(prop, key, value);
		if (!_zp_add_value(prop, key, false))
			return false;
		if (!_zp_add_value(prop, value, true)) {
			free(prop->data[prop->count - 1]);
			_zp_realloc(prop, -1);
			return false;
		}
		return true;
	}
	return false;
}

ZPEXPORT bool zp_addb(pzprop prop, cstr_t key, bool value)
{
	return zp_add(prop, key, value ? "1" : "0");
}

ZPEXPORT bool zp_addi(pzprop prop, cstr_t key, int value)
{ NUMBER_TO_STR("%d"); }

ZPEXPORT bool zp_addf(pzprop prop, cstr_t key, float value)
{ NUMBER_TO_STR("%f"); }

ZPEXPORT bool zp_addd(pzprop prop, cstr_t key, double value)
{ NUMBER_TO_STR("%f"); }

ZPEXPORT bool zp_addul(pzprop prop, cstr_t key, uint32_t value)
{ NUMBER_TO_STR("%u"); }

ZPEXPORT bool zp_addull(pzprop prop, cstr_t key, uint64_t value)
{ NUMBER_TO_STR("%llu"); }

ZPEXPORT bool zp_addi64(pzprop prop, cstr_t key, int64_t value)
{ NUMBER_TO_STR("%lld"); }

ZPEXPORT bool zp_addld(pzprop prop, cstr_t key, ldouble_t value)
{ NUMBER_TO_STR("%Lf"); }

ZPEXPORT bool zp_addr(pzprop prop, cstr_t key, const zprect* r)
{
	return r ? _zp_add_geometry(prop, key, (const long*)r, 4)
		: false;
}

ZPEXPORT bool zp_addp(pzprop prop, cstr_t key, const zppoint* p)
{
	return p ? _zp_add_geometry(prop, key, (const long*)p, 2)
		: false;
}

ZPEXPORT bool zp_addsz(pzprop prop, cstr_t key, const zpsize* sz)
{
	return sz ? _zp_add_geometry(prop, key, (const long*)sz, 2)
		: false;
}

static bool _zp_add_geometry(pzprop prop, cstr_t key, const long* g, 
					  size_t c)
{
	char buf[260] = { 0 };
	char num[100];
	for (size_t i = 0; i < c; i++) {
		sprintf(num, "%d", g[i]);
		strcat(buf, num);
		if (i + 1 < c) {
			strcat(buf, ",");
		}
	}
	return zp_add(prop, key, buf);
}

ZPEXPORT bool zp_addc(pzprop prop, cstr_t key, char value)
{
	if (value == '\0') {
		return zp_add(prop, key, NULL);
	} else {
		char str[] = { value, '\0' };
		return zp_add(prop, key, str);
	}
	return false;
}

ZPEXPORT size_t zp_size(pzprop prop)
{
	return (prop == NULL || prop->count == 0) ? 0 : prop->count / 2;
}

ZPEXPORT bool zp_write_a(pzprop prop, cstr_t filename)
{ return _zp_write(prop, filename, false); }

ZPEXPORT bool zp_write_w(pzprop prop, cwstr_t filename)
{ return _zp_write(prop, filename, true); }

ZPEXPORT bool zp_read_a(pzprop *prop, cstr_t filename)
{ return _zp_read(prop, filename, false); }

ZPEXPORT bool zp_read_w(pzprop *prop, cwstr_t filename)
{ return _zp_read(prop, filename, true); }

bool _zp_substr_token(char* out, size_t begin, size_t end)
{
	if (begin == end || begin > end)
		return false;

	size_t i = begin, j = 0;
	for (; i < end; i++)
	{
		if (out[i] == ',' && i == begin++)
			continue;
		out[j++] = out[i];
	}
	out[j] = '\0';
	return j != 0;
}

static size_t _zp_str_length_control(const char *in)
{
	size_t length = strlen(in);
	for (size_t i = 0; in[i] != '\0'; i++) {
		if (in[i] == '\t' || in[i] == '\f' || 
			in[i] == '\r' || in[i] == '\n') {
			length++;
		}
	}
	return length;
}

static bool _zp_resolve_str(char **out, const char *in)
{
	if (in != NULL && out != NULL) {
		size_t len = strlen(in);
		if ((*out = (char *)calloc(
			_zp_str_length_control(in) + 1, 1)) != NULL) {
			for (size_t i = 0, j = 0; i < len; i++) {
				switch (in[i]) {
				case '\t':
					(*out)[j++] = '\\';
					(*out)[j++] = 't';
					break;
				case '\f':
					(*out)[j++] = '\\';
					(*out)[j++] = 'f';
					break;
				case '\r':
					(*out)[j++] = '\\';
					(*out)[j++] = 'r';
					break;
				case '\n':
					(*out)[j++] = '\\';
					(*out)[j++] = 'n';
					break;
				default:
					(*out)[j++] = in[i];
					break;
				}
			}
			return true;
		}
	}
	return false;
}

static bool _zp_write_line(FILE *file, cstr_t key, cstr_t value)
{
	if (value == NULL) {
		fprintf(file, "%s=\n", key);
	} else {
		char *aux = NULL;
		if (!_zp_resolve_str(&aux, value))
			return false;
		fprintf(file, "%s=%s\n", key, aux);
		free(aux);
	}
	return true;
}

static bool _zp_write(pzprop prop, const void *filename, bool unicode)
{
	FILE *file;
	if (prop != NULL && filename != NULL && 
		(file = OPEN_FILE("w")) != NULL) {
		for (size_t i = 1; i < prop->count; i += 2) {
			if (!_zp_write_line(file, prop->data[i - 1], 
				prop->data[i])) {
				break;
			}
		}
		fclose(file);
		return errno == 0;
	}
	return false;
}

static bool _zp_read(pzprop *prop, const void *filename, bool unicode)
{
	FILE *file;
	if (prop != NULL && filename != NULL && 
		(file = OPEN_FILE("r")) != NULL) {
		if (*prop == NULL && (*prop = zp_create()) == NULL)
			goto _closefl_;

		STRING line;
		memset(&line, 0, sizeof(STRING));
		while (_zp_read_line(&line, file) || line.count != 0) {
			if (line.count > 0) {
				_zp_app_property(*prop, line.data, line.count);
				if (errno != 0) {
					break;
				}
			}
		}

		free(line.data);

	_closefl_:
		fclose(file);
		return errno == 0;
	}
	return false;
}

static void _zp_app_property(pzprop prop, cstr_t line, size_t len)
{
	bool espace = true;
	bool cp_key = true;
	char c;

	STRING str;
	memset(&str, 0, sizeof(str));
	for (size_t i = 0; i < len; i++) {
		c = line[i];
		if (cp_key && (c == '=' || c == ':')) {
		_value:
			if (str.count == 0 || !_zp_add_value(prop, str.data, false))
				break;
			str.count = 0;
			cp_key = false;
			continue;
		}

		if (isblank(c) && espace) {
			if (cp_key && i + 1 < len && 
				!isblank(c = line[i + 1]) && c != '=' && c != ':') {
				goto _value;
			}
			continue;
		}

		if (!cp_key && espace)
			espace = false;
		if (!_zp_append(&str, c))
			break;
		if (!cp_key && i + 1 == len) {
			for (int j = (int)(str.count - 1); j >= 0; j--) {
				if (!isblank(str.data[j]))
					break;
				str.data[--str.count] = '\0';
			}
		}
	}

	if (errno == 0) {
		if (cp_key) {
			if (str.count != 0) {
				_zp_add_value(prop, str.data, false);
				_zp_add_value(prop, NULL, true);
			}
		} else {
			_zp_add_value(prop, str.count == 0 ? NULL : str.data, true);
		}
	}

	free(str.data);
}

static bool _zp_append(STRING *str, char c)
{
	if (str->count == str->bytes) {
		void *mem = realloc(str->data, str->bytes + STR_REALLOC_BYTES + 1);
		if (mem == NULL)
			return false;
		str->bytes += STR_REALLOC_BYTES;
		str->data = (char*)mem;
	}

	str->data[str->count++] = c;
	str->data[str->count  ] = '\0';
	return true;
}

static size_t _zp_indexof(pzprop prop, cstr_t key)
{
	if (prop != NULL && key != NULL) {
		for (size_t i = 0; i < prop->count; i += 2) {
			if (strcmp(prop->data[i], key) == 0) {
				return i;
			}
		}
	}
	return UINT32_MAX;
}

static char *_zp_convert(char *out, cstr_t in)
{
	size_t length;
	if (out != NULL && in != NULL && (length = strlen(in)) > 0) {
		length++;
		for (size_t i = 0, j = 0; i < length; i++) {
			char c = in[i];
			if (c == '\\') {
				switch (c = in[++i]) {
				case 't': c = '\t'; break;
				case 'f': c = '\f'; break;
				case 'r': c = '\r'; break;
				case 'n': c = '\n'; break;
				}
			}

			out[j++] = c;
		}
	}
	return out;
}

static bool _zp_read_line(STRING *line, FILE *file)
{
	int c;
	bool begin = true;
	bool comment = false;
	line->count = 0;

	while (true) {
		if ((c = fgetc(file)) == EOF || c == '\n' || c == '\r') {
			if (comment && c != EOF) {
				begin = true;
				comment = false;
				continue;
			}
			break;
		}

		if (begin) {
			if (begin = (bool)isblank(c)) {
				continue;
			} else if (c == '#' || c == ';') {
				comment = true;
			}
		}

		if (!comment && !_zp_append(line, (char)c)) {
			line->count = 0;
			c = EOF;
			break;
		}
	}

	return c != EOF;
}

static bool _zp_add_value(pzprop prop, cstr_t value, bool is_value)
{
	if (_zp_realloc(prop, 1)) {
		if (value == NULL) {
			prop->data[prop->count - 1] = NULL;
			return true;
		} else {
			char *value_c = _strdup(value);
			if (value_c != NULL) {
				prop->data[prop->count - 1] = value_c;
				if (is_value)
					_zp_convert(value_c, value_c);
				return true;
			}
		}
		_zp_realloc(prop, -1);
	}
	return false;
}

static bool _zp_is_empty(cstr_t value)
{
	size_t length;
	if ((length = strlen(value)) > 0) {
		for (size_t i = 0; i < length; i++) {
			if (!isblank(value[i])) {
				return false;
			}
		}
	}
	return true;
}

static bool _zp_realloc(pzprop prop, int count_e)
{
	void *mem = realloc(prop->data,
					   (prop->count + count_e) * sizeof(void*));
	if (mem != NULL || (prop->count + count_e) == 0) {
		prop->data = (char**)mem;
		prop->count += count_e;
		return true;
	}
	return false;
}