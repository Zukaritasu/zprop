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
	/* indica la cantidad de memoria asignada a 'data' */
	size_t bytes;
	/* la cantidad de caracteres de la cadena */
	size_t count;
	/* el string */
	char *data;
} STRING;

#define STR_REALLOC_BYTES 512

#define OPEN_FILE(flag) \
	(unicode ? _wfopen((const wchar_t *)filename, L##flag) : \
	fopen((const char *)filename, flag))

// ************************************************************************

size_t _zp_indexof(pzprop prop, cstr_t key);
bool _zp_write(pzprop prop, const void *filename, bool unicode);
bool _zp_read(pzprop *prop, const void *filename, bool unicode);
bool _zp_realloc(pzprop prop, int count_e);
bool _zp_is_empty(cstr_t value);
bool _zp_read_line(STRING *line, FILE *file);
bool _zp_append(STRING *str, char c);
void _zp_app_property(pzprop prop, cstr_t kval, size_t cKval);
bool _zp_add_value(pzprop prop, cstr_t value, bool is_value);
char *_zp_convert(char *out, cstr_t in);
bool _zp_resolve_str(char **out, const char *in);

// ************************************************************************

ZPEXPORT void zp_free(pzprop prop)
{
	if (prop != NULL) {
		for (size_t i = 0; i < prop->count; i++)
			free(prop->data[i]);
		free(prop->data);
		free(prop);
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

ZPEXPORT cstr_t zp_value(pzprop prop, cstr_t key)
{
	size_t i = _zp_indexof(prop, key);
	return (i != UINT32_MAX) ? prop->data[i + 1] : NULL;
}

ZPEXPORT bool _zp_replace(pzprop prop, cstr_t key, cstr_t value)
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
	if (prop != NULL && !_zp_is_empty(key) && !zp_contains(prop, key)) {
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

ZPEXPORT size_t zp_size(pzprop prop)
{
	return (prop == NULL || prop->count == 0) ? 0 : prop->count / 2;
}

/***********************************************************************
 *
 * CONVERSIONES
 *
 **********************************************************************/

ZPEXPORT int zp_int(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return atoi(value);
	return 0; 
}

ZPEXPORT int64_t zp_int64(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return atoll(value);
	return 0; 
}

ZPEXPORT double zp_double(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return atof(value);
	return 0; 
}

ZPEXPORT long double zp_longdouble(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return strtold(value, NULL);
	return 0;
}

ZPEXPORT unsigned long zp_ulong(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return strtoul(value, NULL, 10);
	return 0;
}

ZPEXPORT uint64_t zp_uint64(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return strtoull(value, NULL, 10);
	return 0;
}

ZPEXPORT bool zp_bool(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return strcmp(value, "true") == 0 || value[0] == '1';
	return false;
}

/**********************************************************************/

ZPEXPORT bool zp_write_a(pzprop prop, cstr_t filename)
{ return _zp_write(prop, filename, false); }

ZPEXPORT bool zp_write_w(pzprop prop, cwstr_t filename)
{ return _zp_write(prop, filename, true); }

ZPEXPORT bool zp_read_a(pzprop *prop, cstr_t filename)
{ return _zp_read(prop, filename, false); }

ZPEXPORT bool zp_read_w(pzprop *prop, cwstr_t filename)
{ return _zp_read(prop, filename, true); }

bool _zp_resolve_str(char **out, const char *in)
{
	if (in != NULL && out != NULL) {
		size_t len = strlen(in);
		if ((*out = (char *)calloc(len * 2 + 1, 1)) != NULL) {
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

bool _zp_write(pzprop prop, const void *filename, bool unicode)
{
	FILE *file;
	if (prop != NULL && filename != NULL && (file = OPEN_FILE("w")) != NULL) {
		for (size_t i = 1; i < prop->count; i += 2) {
			char *val_c = NULL;
			if (prop->data[i] != NULL && 
				!_zp_resolve_str(&val_c, prop->data[i]))
				break;
			if (fprintf(file, "%s=%s\n", prop->data[i - 1], val_c) < 0) {
				free(val_c);
				break;
			}
			free(val_c);
		}

		fclose(file);
		return errno == 0;
	}
	return false;
}

bool _zp_read(pzprop *prop, const void *filename, bool unicode)
{
	FILE *file;
	if (prop != NULL && filename != NULL && (file = OPEN_FILE("r")) != NULL) {
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

void _zp_app_property(pzprop prop, cstr_t line, size_t len)
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
		if (!cp_key && i + 1 < len) {
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

bool _zp_append(STRING *str, char c)
{
	if (str->count == str->bytes) {
		void *mem = realloc(str->data, str->bytes + 
			STR_REALLOC_BYTES + (str->data == NULL ? 1 : 0));
		if (mem == NULL)
			return false;
		str->bytes += STR_REALLOC_BYTES;
		str->data = (char*)mem;
	}

	str->data[str->count++] = c;
	str->data[str->count  ] = '\0';
	return true;
}

size_t _zp_indexof(pzprop prop, cstr_t key)
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

char *_zp_convert(char *out, cstr_t in)
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

bool _zp_read_line(STRING *line, FILE *file)
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

bool _zp_add_value(pzprop prop, cstr_t value, bool is_value)
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

bool _zp_is_empty(cstr_t value)
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

bool _zp_realloc(pzprop prop, int count_e)
{
	void *mem = realloc(prop->data,
					   (prop->count + count_e) * sizeof(void *));
	if (mem != NULL || (prop->count + count_e) == 0) {
		prop->data = (char **)mem;
		prop->count += count_e;
		return true;
	}
	return false;
}