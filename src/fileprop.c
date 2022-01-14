/*
** Copyright (C) 2021-2022 Zukaritasu
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "fileprop.h"

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
	char*  data;
} STRING;

#define STR_REALLOC_BYTES 512

#define OPEN_FILE(flag) \
		(unicode ? _wfopen((const wchar_t*)filename, L##flag) : \
			fopen((const char*)filename, flag))

// ************************************************************************

size_t _zp_indexof     (pzprop prop, cstr_t key);
bool   _zp_write       (pzprop prop, const void* filename, bool unicode);
bool   _zp_read        (pzprop* prop, const void* filename, bool unicode);
bool   _zp_realloc     (pzprop prop, int count_e);
bool   _zp_is_empty    (cstr_t value);
bool   _zp_read_line   (STRING* line, FILE* file);
bool   _zp_append      (STRING* str, char c);
void   _zp_app_property(pzprop prop, cstr_t kval, size_t cKval);
bool   _zp_add_value   (pzprop prop, cstr_t value, bool is_value);
char*  _zp_convert     (char* out, cstr_t in);
bool   _zp_resolve_str (char** out, const char* in);

// ************************************************************************

// Agrega un caracter a la cadena 'str->data'. Cuando 'str->count' es
// igual a la cantidad de memoria 'str->bytes' asignados a 'str->data' se
// debe reasignar mas memoria para el proximo caractery si ocurre un error
// al reasignar memoria la funcion retorna false
bool _zp_append(STRING* str, char c)
{
	if (str->count == str->bytes)
	{
		void* mem = realloc(str->data, str->bytes +
			STR_REALLOC_BYTES + (str->data == NULL ? 1 : 0));
		if (mem == NULL) 
		{
			return false; // out of memory error
		}
		str->data = (char*)mem;
		str->bytes += STR_REALLOC_BYTES;
	}

	str->data[str->count++] = c; // new char
	str->data[str->count  ] = '\0';
	return true;
}

 // Convierte un caracter interpretado a un caracter de control los cuales
 // son ('\t', '\f', '\r', '\n')
 //
 // Se puede decir que un caracter interpretado se entiende como por ejemplo
 // escribir un salto de linea '\n' en un archivo con un editor de texto de
 // esta manera "\n" identicamente pero como tal no es un caracter sino que
 // se interpreta como un caracter de salto de linea sin ni siquiera serlo
 //
 // La funcion recibe como primer parametro un array de caracteres que sera
 // la salida o el resultado de la funcion y el segundo parametro el string.
 // La funcion retorna el primer parametro
char* _zp_convert(char* out, cstr_t in)
{
	size_t length;
	if (out != NULL && in != NULL && (length = strlen(in)) > 0) {
		length++; // Se incrementa 1 para copiar el caracter nulo '\0'
		for (size_t i = 0,  j = 0; i < length; i++) {
			char c = in[i];
			if (c == '\\') {
				if ((c = in[++i]) == 't') {
					c = '\t';
				} else if (c == 'f') {
					c = '\f';
				} else if (c == 'r') {
					c = '\r';
				} else if (c == 'n') {
					c = '\n';
				}
			}

			out[j++] = c;
		}
	}
	return out;
}


bool _zp_read_line(STRING* line, FILE* file)
{
	int c;                // donde se guardara el caracter leido
	bool begin   = true;  // para eliminar los espacios iniciales de la linea
	bool comment = false; // para saltarse los comentarios
	line->count  = 0;     
	
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
			// se saltan los espacios en blanco y los comentarios
			if (begin = (bool)isblank(c)) {
				continue;
			} else if (c == '#' || c == ';') {
				comment = true;
			}
		}
		
		if (comment) {
			continue;
		}

		if (!_zp_append(line, (char)c)) {
			line->count = 0;
			c = EOF; // out of memory error
			break;
		}
	}

	return c != EOF;
}


void _zp_app_property(pzprop prop, cstr_t kval, size_t cKval)
{
	bool espace = true; // sirve para evitar los espacios en blanco
	bool cp_key = true; // indica si se esta copiando la clave o el valor
	char c;             // caracter de 'kval'
	
	STRING str; // aqui se guarda temporalmente la clave o el valor
	memset(&str, 0, sizeof(str));
	for (size_t i = 0; i < cKval; i++) {
		c = kval[i];
		if (cp_key && (c == '=' || c == ':')) {
		_value:
			if (str.count == 0 || !_zp_add_value(prop, str.data, false))
				break;
			str.count = 0;
			cp_key = false;
			continue;
		}

		// Se ignoran los espacios en blanco que se encuentran entre
		// la clave y el valor. Ejemplo "clave    =  valor"
		if (isblank(c) && espace) {
			if (cp_key && i + 1 < cKval) {
				if (!isblank(c = kval[i + 1]) && c != '=' && c != ':') {
					goto _value;
				}
			}
			continue;
		}

		if (!cp_key && espace)
			espace = false;
		if (!_zp_append(&str, c)) {
			break;
		}

		// Se eliminan los espacios en blanco inecesarios al final
		// de la cadena 
		if (!cp_key && i + 1 < cKval) {
			for (int j = (int)(str.count - 1); j >= 0; j--) {
				if (!isblank(str.data[j]))
					break;
				str.data[--str.count] = '\0';
			}
		}
	}

	if (errno == 0) {
		if (cp_key) {
			// Es posible que solo se encuentre la clave sin su valor
			// por lo cual se agrega la clave y el valor por defecto
			// asignado es NULL
			if (str.count != 0) {
				_zp_add_value(prop, /* key */ str.data, false);
				_zp_add_value(prop, /* value */  NULL, true);
			}
		} else {
			// La clave es agregada en el for. Cuando el ciclo del
			// for termina se agrega el valor
			_zp_add_value(prop, /* value */ 
				str.count == 0 ? NULL : str.data, true);
		}
	}

	free(str.data);
}


ZPEXPORT void ZPCALL zp_free(pzprop prop)
{
	if (prop != NULL) {
		for (size_t i = 0; i < prop->count; i++)
			free(prop->data[i]); // se libera la memoria asignada por strdup
		free(prop->data);
		free(prop);
	}
}


bool _zp_realloc(pzprop prop, int count_e)
{
	void* void_p = realloc(prop->data, (prop->count + count_e) * sizeof(void*));
	if (void_p  != NULL || (prop->count + count_e) == 0) {
		prop->data = (char**)void_p; // void_p puede ser NULL o no
		prop->count += count_e;
		return true;
	}
	return false;
}

ZPEXPORT void ZPCALL zp_print(pzprop prop)
{
	if (prop != NULL) {
		for (size_t i = 1; i < prop->count; i += 2) {
			printf("%s=%s\n", prop->data[i - 1], prop->data[i]);
		}
	}
}

bool _zp_is_empty(cstr_t value)
{
	size_t length; // longitud de la cadena
	if ((length = strlen(value)) > 0) {
		for (size_t i = 0; i < length; i++) {
			if (!isblank(value[i])) {
				return false; // la cadena no esta vacia
			}
		}
	}
	return true;
}

ZPEXPORT bool ZPCALL zp_contains(pzprop prop, cstr_t key)
{
	return zp_value(prop, key) != NULL;
}

ZPEXPORT pzprop ZPCALL zp_create(void)
{
	pzprop prop = (pzprop)malloc(sizeof(zprop));
	if (prop != NULL)
		memset(prop, 0, sizeof(zprop));
	return prop;
}

size_t _zp_indexof(pzprop prop, cstr_t key)
{
	if (prop != NULL && key != NULL) {
		for (size_t i = 0; i < prop->count; i += 2) {
			if (strcmp(prop->data[i], key) == 0) {
				return i; // posicion de la clave
			}
		}
	}
	return UINT_MAX;
}

ZPEXPORT bool ZPCALL zp_remove(pzprop prop, cstr_t key)
{
	size_t i = _zp_indexof(prop, key);
	if (i != UINT_MAX) {
		free(prop->data[i++]); // key
		free(prop->data[i++]); // value
		// el contenido es movido 2 espacios hacia atras
		while (i < prop->count)
			prop->data[i - 2] = prop->data[i++];
		return _zp_realloc(prop, -2);
	}
	return false;
}

// Retorna el valor de la clave. Nungun parametro puede ser NULL y ademas
// la clave debe existir sino el valor retornado por la funcion es NULL
// aunque puede retornar NULL si el valor de la clave es NULL
ZPEXPORT cstr_t ZPCALL zp_value(pzprop prop, cstr_t key)
{
	size_t i = _zp_indexof(prop, key);
	return (i != UINT_MAX) ? prop->data[i + 1] : NULL;
}

// Reemplaza el valor de una clave por otra. Este valor puede ser NULL
// o una cadena valida sin importar si esta vacia. La memoria asignada
// por strdup es liberada y luego es sustituido ese espacio del array
// por el nuevo valor. Si la funcion tuvo exito retornara true
ZPEXPORT bool ZPCALL _zp_replace(pzprop prop, cstr_t key, cstr_t value)
{
	size_t i = _zp_indexof(prop, key); // se obtiene el indice de la clave
	if (i++ != UINT_MAX) {
		char* text = value != NULL ? _strdup(value) : NULL;
		if (text != NULL || value == NULL) {
			free(prop->data[i]);
			prop->data[i] = _zp_convert(text, text);
			return true;
		}
	}
	return false;
}

// Agrega un nuevo elemento al array 'prop->data' lo cual puede ser una
// clave o el valor de la clave. Se llama la funcion '_zp_realloc' para
// tener un espacio disponible para el nuevo elemento en el array
// 'prop->data' y si ocurre algun error ese espacio es eliminado por la
// misma funcion '_zp_realloc'
//
// Esta funcion retorna true si la operacion al agregar un nuevo elemento
// tuvo exito, de lo contrario retorna false indicando un error
bool _zp_add_value(pzprop prop, cstr_t value, bool is_value)
{
	if (_zp_realloc(prop, 1)) {
		if (value == NULL) {
			prop->data[prop->count - 1] = NULL;
			return true;
		} else {
			char* value_c = _strdup(value);
			if (value_c != NULL) {
				prop->data[prop->count - 1] = value_c;
				if (is_value) {
					_zp_convert(value_c, value_c);
				}
				return true;
			}
		}
		_zp_realloc(prop, -1);
	}
	return false;
}

// Agrega una nueva clave y su valor en el array 'prop->data'. Los
// parametros 'prop' y 'key' no pueden ser NULL y ademas 'key' debe ser
// un valor valido
// 
// La clave no puede ser una cadena vacia o con espacios
ZPEXPORT bool ZPCALL zp_add(pzprop prop, cstr_t key, cstr_t value)
{
	if (prop != NULL && !_zp_is_empty(key) && !zp_contains(prop, key)) {
		if (!_zp_add_value(prop, key, false))    // se asigna la clave
			return false;
		if (!_zp_add_value(prop, value, true)) { // se asigna el valor
			free(prop->data[prop->count - 1]);
			_zp_realloc(prop, -1);
			return false;
		}
		return true;
	}
	return false;
}

// Retorna la cantidad de pares de claves y su valor. Como todas las
// claves y valores se encuentran en un solo array 'prop->data' se debe
// dividir 'prop->count' entre 2 para tener la cantidad. Si el parametros
// es NULL o 'prop->count' es 0 entonces la funcion retorna 0
ZPEXPORT size_t ZPCALL zp_size(pzprop prop)
{
	return (prop == NULL || prop->count == 0) ? 0 : prop->count / 2;
}

//
//
//
//
bool _zp_resolve_str(char** out, const char* in)
{
	if (in != NULL && out != NULL) {
		size_t len = strlen(in);
		if ((*out = (char*)calloc(len * 2 + 1, 1)) != NULL) {
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
	return false;;
}

// Escribe un archivo de propiedades. La funcion recibe como parametro el
// puntero a estructura 'zprop' que contiene las claves y valores, el
// nombre de archivo 'filename' y un valor booleando 'unicode' que indica
// si el nombre del archivo es unicode o no
//
// La funcion retorna true si la operacion tuvo exito. Retorna false si
// los parametro son NULL o que ocurrio un error en el proceso de
// escritura del archivo
bool _zp_write(pzprop prop, const void* filename, bool unicode)
{
	FILE* file;
	bool error = false;
	if (prop != NULL && filename != NULL && (file = OPEN_FILE("w")) != NULL) {
		for (size_t i = 0; i < prop->count; i += 2) {
			char* val_c = NULL;
			if (prop->data[i] != NULL && (error = !_zp_resolve_str(&val_c, 
				prop->data[i])))
				break;
			if (error = (fprintf(file, "%s=%s\n", 
				prop->data[i - 1], val_c) < 0)) {
				free(val_c);
				break;
			}
			free(val_c);
		}
		
		fclose(file);
		return !error;
	}
	return error;
}

/***********************************************************************
 *
 * CONVERSIONES
 *
 **********************************************************************/

// La funcion convierte el valor de la clave en un valor entero de 64
// bits usando a funcion 'atoi'. El valor no puede ser NULL y debe ser
// un valor valido para la conversion
ZPEXPORT int ZPCALL zp_int(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return atoi(value);
	return 0;
}

// La funcion convierte el valor de la clave en un valor entero de 64
// bits usando a funcion 'atoll'. El valor no puede ser NULL y debe ser
// un valor valido para la conversion
ZPEXPORT int64_t ZPCALL zp_int64(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return atoll(value);
	return 0;
}

// La funcion convierte el valor de la clave en un valor booleano. Para
// que la funcion retorne true el valor no puede ser NULL y ademas debe
// contener 'true' o un '1', de lo contrario retorna false
ZPEXPORT bool ZPCALL zp_bool(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return strcmp(value, "true") == 0 || value[0] == '1';
	return false;
}

// La funcion convierte el valor de la clave usando la funcion 'atof'
// el un valor de tipo double. El valor no puede ser NULL y debe ser
// un valor valido para la conversion
ZPEXPORT double ZPCALL zp_double(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return atof(value);
	return 0;
}

/**********************************************************************/

// Escribe un archivo de propiedades. La funcion recibe como parametro
// el puntero a estructura 'zprop' que contiene las claves y valores, el
// nombre de archivo 'filename' que es una cadena ANCII
//
// La funcion retorna false si uno de los parametros es NULL o invalidos,
// tambien si ocurre un error en la escritura del archivo
ZPEXPORT bool ZPCALL zp_write_a(pzprop prop, cstr_t filename)
{
	return _zp_write(prop, filename, false);
}

// Escribe un archivo de propiedades. La funcion recibe como parametro
// el puntero a estructura 'zprop' que contiene las claves y valores, el
// nombre de archivo 'filename' que es una cadena Unicode
//
// La funcion retorna false si uno de los parametros es NULL o invalidos,
// tambien si ocurre un error en la escritura del archivo
ZPEXPORT bool ZPCALL zp_write_w(pzprop prop, cwstr_t filename)
{
	return _zp_write(prop, filename, true);
}

// Lee un archivo de propiedades. La funcion recibe como parametro
// el puntero a estructura 'zprop' donde se guardaran as propiedades
// leidas y como segundo parametro el nombre del archivo version (ASCII)
//
// La funcion retorna false si los parametro son invalidos, ocurrio un
// error en la apertura / lectura del archivo o error al asignar memoria
ZPEXPORT bool ZPCALL zp_read_a(pzprop* prop, cstr_t filename)
{
	return _zp_read(prop, filename, false);
}

// Lee un archivo de propiedades. La funcion recibe como parametro
// el puntero a estructura 'zprop' donde se guardaran as propiedades
// leidas y como segundo parametro el nombre del archivo version (UNICODE)
//
// La funcion retorna false si los parametro son invalidos, ocurrio un
// error en la apertura / lectura del archivo o error al asignar memorias
ZPEXPORT bool ZPCALL zp_read_w(pzprop* prop, cwstr_t filename)
{
	return _zp_read(prop, filename, true);
}

//
bool _zp_read(pzprop* prop, const void* filename, bool unicode)
{
	FILE* file;
	if (prop != NULL && filename != NULL && (file = OPEN_FILE("r")) != NULL) {
		if (*prop == NULL && (*prop = zp_create()) == NULL) {
			goto _closefl_;
		}

		STRING line;
		memset(&line, 0, sizeof(STRING));
		// se leen las lineas del archivo y se llama la funcion
		// '_zp_app_property' para agregar la propiedad
		while (_zp_read_line(&line, file) || line.count != 0) {
			if (line.count > 0) {
				_zp_app_property(*prop, line.data, line.count);
				if (errno != 0)
				{
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