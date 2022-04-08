/*
 * Copyright (C) 2021-2022 Zukaritasu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "../include/fileprop.h"
#include "../include/strutil.h"
#include "../include/fileio.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>

// ************************************************************************

#define OPEN_FILE(flag) \
	(unicode ? \
	_wfopen((const wchar_t*)filename, L##flag) : \
	  fopen((const char*)filename, flag))
	
#define ADD_VALUE_NUMBER(type)          \
	char buf[100];                      \
	sprintf(buf, type, value);          \
	return zp_add(prop, key, buf); 
	
#define RETURN_VALUE_NUMBER(func)       \
	cstr_t value = zp_value(prop, key); \
	if (value != NULL)                  \
		return func(value, NULL, 10);   \
	return 0;

#define RETURN_VALUE_NUMBERF(func)      \
	cstr_t value = zp_value(prop, key); \
	if (value != NULL)                  \
		return func(value, NULL);       \
	return 0;

// ************************************************************************

static int _zp_indexof(cpzprop prop, cstr_t key);
static bool _zp_write(cpzprop prop, const void* filename, bool unicode);
static bool _zp_read(pzprop* prop, const void* filename, bool unicode);
bool _zp_app_property(pzprop prop, char* line);
static bool _zp_add_value(pzprop prop, cstr_t value, bool is_value);
static char* _zp_convert(char* out, cstr_t in);
static bool _zp_replace(pzprop prop, cstr_t key, cstr_t value);
static bool _zp_add_geometry(pzprop prop, cstr_t key, const long* g, unsigned c);
static bool _zp_value_geometry(cpzprop prop, cstr_t key, long* g, unsigned c);

// ************************************************************************

ZPEXPORT void zp_free(pzprop prop)
{
	if (prop)
	{
		for (int i = 0; i < prop->count; i++)
			free(prop->data[i]);
		free(prop->data);
		free(prop);
	}
}

ZPEXPORT void zp_print(pzprop prop)
{
	if (prop)
	{
		for (int i = 1; i < prop->count; i += 2) {
			printf("%s=%s\n", prop->data[i - 1], 
			prop->data[i] == NULL ? "" : prop->data[i]);
		}
	}
}

ZPEXPORT bool zp_contains(cpzprop prop, cstr_t key)
{
	return _zp_indexof(prop, key) != INT32_MAX;
}

ZPEXPORT pzprop zp_create(void)
{
	pzprop prop = (pzprop)calloc(1, sizeof(zprop));
	if (prop)
	{
		prop->auto_dpoint   = true;
		prop->count_decimal = 4;
	}
	return prop;
}

#define PROP_REALLOC(c) \
		_zp_realloc(&prop->data, &prop->count, c, sizeof(void*))

ZPEXPORT bool zp_remove(pzprop prop, cstr_t key)
{
	int i = _zp_indexof(prop, key);
	if (i != -1)
	{
		free(prop->data[i++]);
		free(prop->data[i++]);
		while (i < prop->count)
			prop->data[i - 2] = prop->data[i++];
		return PROP_REALLOC(-2);
	}
	return false;
}

ZPEXPORT void zp_removeall(pzprop prop)
{
	if (prop)
	{
		for (int i = 0; i < prop->count; i++)
			free(prop->data[i]);
		free(prop->data);
		prop->count = 0;
		prop->data  = NULL;
	}
}

static char* _zp_getvalue(cpzprop prop, cstr_t key)
{
	const int i = _zp_indexof(prop, key);
	return (i != -1) ? prop->data[i + 1] : NULL;
}

ZPEXPORT cstr_t zp_get(cpzprop prop, cstr_t key)
{
	return _zp_getvalue(prop, key);
}

ZPEXPORT bool zp_getb(cpzprop prop, cstr_t key)
{
	cstr_t _vb = zp_get(prop, key);
	return _vb ? (_zp_equals(_vb, "true") || _vb[0] == '1') : 0;
}

ZPEXPORT int zp_geti(cpzprop prop, cstr_t key)
{ RETURN_VALUE_NUMBER (strtol);
}
ZPEXPORT int64_t zp_geti64(cpzprop prop, cstr_t key)
{ RETURN_VALUE_NUMBER (strtoll);
}
ZPEXPORT uint32_t zp_getul(cpzprop prop, cstr_t key)
{ RETURN_VALUE_NUMBER (strtoul);
}
ZPEXPORT uint64_t zp_getull(cpzprop prop, cstr_t key)
{ RETURN_VALUE_NUMBER (strtoull);
}

ZPEXPORT double zp_getd(cpzprop prop, cstr_t key)
{
	char* _vd = _zp_getvalue(prop, key);
	if (_vd)
	{
		if (!prop->auto_dpoint) {
			return strtod(_vd, NULL);
		} else {
			char point = '\0';
			int i = 0;
			for (; _vd[i]; i++) {
				if (_vd[i] == '.' || _vd[i] == ',') {
					if (localeconv()->decimal_point[0] != _vd[i]) {
						point = _vd[i];
						_vd[i] = localeconv()->decimal_point[0];
					}
					break;
				}
			}

			double val = strtod(_vd, NULL);
			if (point != '\0') {
				_vd[i] = point;
			}
			return val;
		}
	}
	return 0;
}

ZPEXPORT float zp_getf(cpzprop prop, cstr_t key)
{
	return (float)zp_getd(prop, key);
}
ZPEXPORT ldouble_t zp_getld(cpzprop prop, cstr_t key)
{ RETURN_VALUE_NUMBERF(strtold);
}

ZPEXPORT bool zp_getr(cpzprop prop, cstr_t key, zprect* r)
{ return r ?  _zp_value_geometry(prop, key, (long*)r,  4)
	: false; }

ZPEXPORT bool zp_getp(cpzprop prop, cstr_t key, zppoint* p)
{ return p ?  _zp_value_geometry(prop, key, (long*)p,  2)
	: false; }

ZPEXPORT bool zp_getsz(cpzprop prop, cstr_t key, zpsize* sz)
{ return sz ? _zp_value_geometry(prop, key, (long*)sz, 2)
	: false; }

static bool _zp_value_geometry(cpzprop prop, cstr_t key, long* g,
							   unsigned c)
{
	memset(g, 0, sizeof(long) * c);
	char* value = _zp_strdup(zp_get(prop, key));
	if (value != NULL)
	{
		const int length = _zp_strlen(value);
		if (length > 0) {
			for (int i = 0, pos = 0; i < 4; i++) {
				if (!_zp_next_token(value, ',', &pos, length))
					break;
				g[i] = strtol(value, NULL, 10);
			}
		}
		free(value);
		return true;
	}
	return false;
}

ZPEXPORT char zp_getc(cpzprop prop, cstr_t key)
{
	cstr_t str = zp_get(prop, key);
	return str ? str[0] : '\0';
}

static bool _zp_replace(pzprop prop, cstr_t key, cstr_t value)
{
	const int i = _zp_indexof(prop, key) + 1;
	if (i > 0)
	{
		char* text = _zp_strdup(value);
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
	if (prop && !_zp_is_empty(key))
	{
		if (zp_contains(prop, key))
			return _zp_replace(prop, key, value);
		if (_zp_add_value(prop, key, false) &&
			_zp_add_value(prop, value, true))
			return true;
	}
	return false;
}

ZPEXPORT bool zp_addb(pzprop prop, cstr_t key, bool value)
{
	return zp_add(prop, key, value ? "1" : "0");
}

ZPEXPORT bool zp_addi(pzprop prop, cstr_t key, int value)
{ ADD_VALUE_NUMBER("%d");   }
ZPEXPORT bool zp_addf(pzprop prop, cstr_t key, float value)
{ ADD_VALUE_NUMBER("%f");   }
ZPEXPORT bool zp_addd(pzprop prop, cstr_t key, double value)
{ ADD_VALUE_NUMBER("%f");   }
ZPEXPORT bool zp_addul(pzprop prop, cstr_t key, uint32_t value)
{ ADD_VALUE_NUMBER("%u");   }
ZPEXPORT bool zp_addull(pzprop prop, cstr_t key, uint64_t value)
#ifdef MSVC
{ ADD_VALUE_NUMBER("%llu"); }
#else
{ ADD_VALUE_NUMBER("%I64u"); }
#endif /* MSVC */
ZPEXPORT bool zp_addi64(pzprop prop, cstr_t key, int64_t  value)
#ifdef MSVC
{ ADD_VALUE_NUMBER("%lld"); }
#else
{ ADD_VALUE_NUMBER("%I64d"); }
#endif /* MSVC */
ZPEXPORT bool zp_addld(pzprop prop, cstr_t key, ldouble_t value)
{ ADD_VALUE_NUMBER("%Lf");  }

ZPEXPORT bool zp_addr(pzprop prop, cstr_t key, const zprect* r)
{ return r ?  _zp_add_geometry(prop, key, (const long*)r,  4)
	: false; }

ZPEXPORT bool zp_addp(pzprop prop, cstr_t key,  const zppoint* p)
{ return p ?  _zp_add_geometry(prop, key, (const long*)p,  2)
	: false; }

ZPEXPORT bool zp_addsz(pzprop prop, cstr_t key, const zpsize* sz)
{ return sz ? _zp_add_geometry(prop, key, (const long*)sz, 2)
	: false; }

/* (((10 length INT32_MAX + 1 '-') x 4 long) + 3 ',' + 1 '\0') */
#define G_SIZE_BUF_VALUE (((10 + 1) * 4) + 3 + 1)

bool _zp_add_geometry(pzprop prop, cstr_t key, const long* g,
							 unsigned c)
{
	char value[G_SIZE_BUF_VALUE] = { 0 };
	if (c == 4)
		sprintf(value, "%d,%d,%d,%d", g[0], g[1], g[2], g[3]);
	else
		sprintf(value, "%d,%d",       g[0], g[1]);
	return zp_add(prop, key, value);
}

ZPEXPORT bool zp_addc(pzprop prop, cstr_t key, char value)
{
	if (value == '\0')
		return zp_add(prop, key, NULL);
	else
	{
		char str[] = { value, '\0' };
		return zp_add(prop, key, str);
	}
	return false;
}

ZPEXPORT bool zp_getv(cpzprop prop, cstr_t key,
					   cstr_t template_, ...)
{
	if (prop && key && template_)
	{
		cstr_t value = zp_get(prop, key);
		if (value)
		{
			va_list args;
			va_start(args, template_);
			int count = vsscanf(value, template_, args);
			va_end(args);
			return count >= 0;
		}
	}
	return false;
}

ZPEXPORT unsigned zp_size(cpzprop prop)
{
	return (!prop || prop->count == 0) ? 0 : prop->count / 2;
}

/* ASCII */
ZPEXPORT bool zp_savea(cpzprop prop, cstr_t filename)
{ return _zp_write(prop, filename, false); }
ZPEXPORT bool zp_loada(pzprop *prop, cstr_t filename)
{ return _zp_read(prop, filename, false); }

/* UNICODE */
ZPEXPORT bool zp_savew(cpzprop prop, cwstr_t filename)
{ return _zp_write(prop, filename, true); }
ZPEXPORT bool zp_loadw(pzprop *prop, cwstr_t filename)
{ return _zp_read(prop, filename, true); }

bool _zp_write(pzprop prop, const void* filename, bool unicode)
{
	FILE* file;
	if (prop && filename && (file = OPEN_FILE("w")))
	{
		for (int i = 1; i < prop->count; i += 2)
		{
			if (!_zp_write_line(file,
				prop->data[i - 1], prop->data[i]))
				break;
		}
		fclose(file);
		return errno == 0;
	}
	return false;
}

bool _zp_read(pzprop* prop, const void* filename, bool unicode)
{
	FILE* file;
	if (prop && filename && (file = OPEN_FILE("r")))
	{
		if (!(*prop) && !((*prop) = zp_create()))
			goto _closefl_; /* out of memory */

		STRING line;
		memset(&line, 0, sizeof(STRING));
		while (_zp_read_line(&line, file) || line.count != 0) {
			if (line.count > 0) {
				if (!_zp_app_property(*prop, line.data, line.count)) {
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

static char* _zp_next_elem(char* prop, int* pos, bool is_key)
{
	if (prop && pos)
	{
		int i, j = 0;
		for (i = (*pos); prop[i]; i++) {
			if (is_key && (prop[i] == '=' || prop[i] == ':'))
				break;
			prop[j++] = prop[i];
		}
		(*pos)  = i + 1;
		prop[j] = '\0';
	}
	return _zp_strtrim(prop);
}

bool _zp_app_property(pzprop prop, char* line)
{
	int pos = 0;
	if (_zp_add_value(prop, _zp_next_elem(line, &pos, true)) &&
		_zp_add_value(prop, _zp_next_elem(line, &pos, false)))
		return true;
	return false;
}

int _zp_indexof(cpzprop prop, cstr_t key)
{
	if (prop && key)
	{
		for (int i = 0; i < prop->count; i += 2) {
			if (_zp_equals(prop->data[i], key)) {
				return i;
			}
		}
	}
	return -1;
}

char* _zp_convert(char* out, cstr_t in)
{
	const int length = _zp_strlen(in) + 1; /* + 1 copy '\0' */
	if (out && length > 1)
	{
		for (int i = 0, j = 0; i < length; i++) {
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

bool _zp_add_value(pzprop prop, cstr_t value, bool is_value)
{
	if (PROP_REALLOC(1))
	{
		if (!value || !value[0]) {
			prop->data[prop->count - 1] = NULL;
			return true;
		} else {
			char* copy = _zp_strdup(value);
			if (copy) {
				prop->data[prop->count - 1] = copy;
				if (is_value)
					_zp_convert(copy, copy);
				return true;
			} else if (is_value) {
				free(prop->data[prop->count - 2]);
				PROP_REALLOC(-2);
			} else {
				PROP_REALLOC(-1);
			}
		}
	}
	return false; /* out of memory */
}
