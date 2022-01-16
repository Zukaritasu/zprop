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

ZPEXPORT void ZPCALL zp_free(pzprop prop)
{
	if (prop != NULL) {
		/* Si 'prop->count' no esta acorde con 'prop->data' puede causar
		   que el programa se detenga */
		for (size_t i = 0; i < prop->count; i++) {
			/* se libera la memoria asignada por strdup */
			free(prop->data[i]); 
		}
		free(prop->data);
		free(prop);
	}
}

/* Se imprime la clave y el valor. Es posible que el valor sea NULL
   y al imprimir se puede mostrar como <null> en Windows, para ello
   se imprime una cadena vacia "" */
ZPEXPORT void ZPCALL zp_print(pzprop prop)
{
	if (prop != NULL) {
		for (size_t i = 1; i < prop->count; i += 2) {
			printf("%s=%s\n", prop->data[i - 1], 
				prop->data[i] == NULL ? "" : prop->data[i]);
		}
	}
}

ZPEXPORT bool ZPCALL zp_contains(pzprop prop, cstr_t key)
{
	/* Si la funcion '_zp_indexof' retorna 'UINT_MAX' significa
	   que la propiedad no existe */
	return _zp_indexof(prop, key) != UINT_MAX;
}

/* Se tiene en cuenta que si esta definido el preprocesador
   '__ZPROP_LOCAL__', en ese caso 'pzprop' es un puntero a la
   estructura 'zprop', por lo cual sizeof(zprop) siempre obtiene
   el tamano de la estructura y no el puntero 'void*'.
   
   Al usar malloc ese bloque de memoria es rellenado con ceros para
   "inicializar los campos de la estructura" */
ZPEXPORT pzprop ZPCALL zp_create(void)
{
	pzprop prop = (pzprop)malloc(sizeof(zprop));
	if (prop != NULL)
		memset(prop, 0, sizeof(zprop));
	return prop;
}

/* Remueve una propiedad del conjunto de propiedades. Es liberada la
   memoria asignada por 'strdup' para la clave y el valor. El contenido
   de 'prop->data' es movido -2 espacios para cubrir el espacio de la
   clave y el valor eliminados */
ZPEXPORT bool ZPCALL zp_remove(pzprop prop, cstr_t key)
{
	size_t i = _zp_indexof(prop, key);
	if (i != UINT_MAX) {
		free(prop->data[i++]); /* key */
		free(prop->data[i++]); /* value */
		/* el contenido es movido 2 espacios hacia atras para
		   descartar la propiedad */
		while (i < prop->count)
			prop->data[i - 2] = prop->data[i++];
		return _zp_realloc(prop, -2);
	}
	return false;
}

/* Retorna el valor de la clave solicitada. Si la clave no existe la
   funcion retorna NULL. Al usar esta funcion cabe mencionar que el
   puntero char* o el string puede ser eliminado por lo cual hacer una
   asignacion de esta manera 'const char* str = zp_value()' puede no
   ser seguro, es mejor usar 'strdup' para copiar la cadena */
ZPEXPORT cstr_t ZPCALL zp_value(pzprop prop, cstr_t key)
{
	size_t i = _zp_indexof(prop, key);
	return (i != UINT_MAX) ? prop->data[i + 1] : NULL;
}

/* Esta funcion cambia el valor de la clave por un nuevo valor. Este
   valor puede ser NULL. Para que la funcion tenga exito la clave debe
   existir, por otra parte la funcion puede fallar si 'strdup' retorna
   NULL indicando un error al clonar o copiar el valor en otra cadena */
ZPEXPORT bool ZPCALL _zp_replace(pzprop prop, cstr_t key, cstr_t value)
{
	size_t i = _zp_indexof(prop, key);
	if (i++ != UINT_MAX) {
		/* Es posible que el valor sea NULL por lo cual no se usada
		   'strdup' si no que se asigna el valor NULL directamente */
		char* text = value != NULL ? _strdup(value) : NULL;
		if (text != NULL || value == NULL) {
			/* See libera la memoria asignada por 'strdup' */
			free(prop->data[i]); 
			prop->data[i] = _zp_convert(text, text);
			return true;
		}
	}
	return false;
}

/* Agrega una nueva propiedad en el conjunto de propiedades.
   La clave se verifica con la funcion '_zp_is_empty' para saber si la 
   clave no es una cadena vacia, luego se hace un llamado a la funcion
   'zp_contains' para verificar de que la clave no exista en el
   conjunto de propiedades 
   
   La clave y el valor son agregadas a 'prop->data' mediante el uso de
   la funcion '_zp_add_value'. Si al agregar el valor la funcion falla
   se debe eliminar la clave agregada anteriormente */
ZPEXPORT bool ZPCALL zp_add(pzprop prop, cstr_t key, cstr_t value)
{
	if (prop != NULL && !_zp_is_empty(key) && !zp_contains(prop, key)) {
		/* se agrega la clave */
		if (!_zp_add_value(prop, key, false))
			return false;
		/* se agrega el valor */
		if (!_zp_add_value(prop, value, true)) {
			/* si ocurre un error al agregar el valor, la clave se debe
			   eliminar y se libera la memoria asignada por strdup */
			free(prop->data[prop->count - 1]);
			_zp_realloc(prop, -1);
			return false;
		}
		return true;
	}
	return false;
}

/* Retorna la cantidad de propiedades que existen en el conjunto de
   propiedades. 'prop->count' indica la cantidad de elementos que
   tiene 'prop->data', como 'prop->data' contiene las claves y valores
   se debe dividir 'prop->count' para obtener el resultado */
ZPEXPORT size_t ZPCALL zp_size(pzprop prop)
{
	return (prop == NULL || prop->count == 0) ? 0 : prop->count / 2;
}

/***********************************************************************
 *
 * CONVERSIONES
 *
 **********************************************************************/

/* Convierte el valor de la clave en un valor entero 'int' usando la
   funcion 'atoi'. Si el valor no se pudo convertir, el valor por defecto
   que retorna esta funcion es 0 */
ZPEXPORT int ZPCALL zp_int(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return atoi(value);
	return 0;
}

/* Convierte el valor de la clave en un valor entero largo 'int64_t'
   usando la funcion 'atoll'. Si el valor no se pudo convertir, el valor
   por defecto que retorna esta funcion es 0 */
ZPEXPORT int64_t ZPCALL zp_int64(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return atoll(value);
	return 0;
}

/* Convierte el valor de la clave en un valor de tipo 'bool' usando
   como condicion si el valor es "true" o "1". Si el valor es diferente
   a lo mencionado la funcion retorna false */
ZPEXPORT bool ZPCALL zp_bool(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return strcmp(value, "true") == 0 || value[0] == '1';
	return false;
}

/* Convierte el valor de la clave en un valor de tipo 'double' usando la
   funcion 'atof'. Si el valor no se pudo convertir, el valor por
   defecto que retorna esta funcion es 0 */
ZPEXPORT double ZPCALL zp_double(pzprop prop, cstr_t key)
{
	cstr_t value = zp_value(prop, key);
	if (value != NULL)
		return atof(value);
	return 0;
}

/**********************************************************************/

/* Version ASCII */
ZPEXPORT bool ZPCALL zp_write_a(pzprop prop, cstr_t filename)
{ return _zp_write(prop, filename, false); }

/* Version UNICODE */
ZPEXPORT bool ZPCALL zp_write_w(pzprop prop, cwstr_t filename)
{ return _zp_write(prop, filename, true); }

/* Cambia los caracteres de control '\t', '\f', '\r' y '\n' agregando una
   barra diagonal '\' y luego el caracter que representa ese caracter para
   que desde el archivo se pueda presenciar los caracteres de control. */
bool _zp_resolve_str(char** out, const char* in)
{
	if (in != NULL && out != NULL) {
		size_t len = strlen(in);
		/* se pide un bloque de memoria usando la longitud de la cadena
		   multiplicada por 2 para tener el espacio suficiente para el
		   cambio, ademas se le suma 1 para el caracter nulo */
		if ((*out = (char*)calloc(len * 2 + 1, 1)) != NULL) 
		{
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

/* Se abre el archivo de propiedades con la macro 'OPEN_FILE' y se escriben
   las propiedades. El valor de la clave de cada propiedad es pasada por
   parametro a la funcion '_zp_resolve_str' para que cambie los caracteres
   de control en caracteres que puedan ser "visibles" desde el archivo */
bool _zp_write(pzprop prop, const void* filename, bool unicode)
{
	FILE* file;
	if (prop != NULL && filename != NULL && (file = OPEN_FILE("w")) != NULL) 
	{
		for (size_t i = 0; i < prop->count; i += 2) {
			char* val_c = NULL;
			/* se cambian los caracteres de control por caracteres 'visibles' */
			if (prop->data[i] != NULL && !_zp_resolve_str(&val_c, prop->data[i]))
				break;
			/* se escribe la clave y su valor (el valor puede ser NULL) */
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

/* Version ASCII */
ZPEXPORT bool ZPCALL zp_read_a(pzprop* prop, cstr_t filename)
{ return _zp_read(prop, filename, false); }

/* Version UNICODE */
ZPEXPORT bool ZPCALL zp_read_w(pzprop* prop, cwstr_t filename)
{ return _zp_read(prop, filename, true); }

/* Se abre el archivo de propiedades con la macro 'OPEN_FILE' y
   se lee cada propiedad con la funcion '_zp_read_line' y la funcion
   '_zp_app_property' se encarga de procesar esa propiedad y  agregarla 
   al conjunto de propiedades */
bool _zp_read(pzprop* prop, const void* filename, bool unicode)
{
	FILE* file;
	if (prop != NULL && filename != NULL && (file = OPEN_FILE("r")) != NULL) 
	{
		/* se crea un handle para el conjunto de propiedades */
		if (*prop == NULL && (*prop = zp_create()) == NULL)
			goto _closefl_;

		STRING line;
		memset(&line, 0, sizeof(STRING));
		/* se leen las lineas del archivo y se llama la funcion
		 '_zp_app_property' para agregar la propiedad */
		while (_zp_read_line(&line, file) || line.count != 0) {
			if (line.count > 0) {
				_zp_app_property(*prop, line.data, line.count);
				if (errno != 0) /* posible error producido por desde la funcion */
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

/* Agrega una propiedad en el conjunto de propiedades. Esta funcion es
   llamada desde '_zp_read' para procesar la linea del archivo que
   contiene la propiedad. La clave y el valor estan separados por los
   caracteres '=' o ':' tambien es valido un espacio en blanco. La clave 
   puede o no contener un valor, en ese caso se asigna NULL como valor
   por defecto */
void _zp_app_property(pzprop prop, cstr_t kval, size_t cKval)
{
	bool espace = true; /* sirve para evitar los espacios en blanco */
	bool cp_key = true; /* indica si se esta copiando la clave o el valor */
	char c;             /* caracter de 'kval' */
	
	STRING str; /* aqui se guarda temporalmente la clave o el valor */
	memset(&str, 0, sizeof(str));
	for (size_t i = 0; i < cKval; i++) {
		c = kval[i];
		if (cp_key && (c == '=' || c == ':')) {
		_value:
			/* En un caso remoto pueden haber errores en la sintaxis
			   del archivo como por ejemplo no tener la clave pero
			   si el valor o sin valor. En todo caso se verifica que
			   str.count no sea igual a 0 y si lo es entonces no existen
			   una clave y se rompe el ciclo del for. Tambien se rompe
			   el ciclo si la funcion '_zp_add_value' retorna false */
			if (str.count == 0 || !_zp_add_value(prop, str.data, false))
				break;
			str.count = 0;
			cp_key = false;
			continue;
		}

		/* Se ignoran los espacios en blanco que se encuentran entre la
		   clave y el valor. Ejemplo "clave    =  valor" */
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

		/* Se eliminan los espacios en blanco inecesarios al final de
		   la cadena */
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
			/* Es posible que solo se encuentre la clave sin su valor
			   por lo cual se agrega la clave y el valor por defecto
			   asignado es NULL */
			if (str.count != 0) {
				_zp_add_value(prop, /* key */ str.data, false);
				_zp_add_value(prop, /* value */  NULL, true);
			}
		} else {
			/* La clave es agregada en el for. Cuando el ciclo del
			   for termina se agrega el valor */
			_zp_add_value(prop, /* value */ 
				str.count == 0 ? NULL : str.data, true);
		}
	}

	free(str.data);
}

/* Agrega un caracter al final de la cadena. Cuando 'str->count' sea
   igual a la cantidad de buyes asignados 'str->bytes' se debe llamar
   a la funcion 'realloc' para reasignar mas memoria a 'str->data'.
   Cada llamado a la funcion 'realloc' exitosa se le suma la cantidad
   definida por 'STR_REALLOC_BYTES' al campo 'str->bytes'.
   
   La funcion retorna false si 'realloc' retorna un puntero nulo (NULL),
   en tal caso se sale de la funcion retornando false y se mantiene
   'str->bytes' sin ningun incremento.
   
   Cada vez que se agrega un nuevo caracter a la cadena se asigna el
   caracter '\0' en todo los caso.*/
bool _zp_append(STRING* str, char c)
{
	if (str->count == str->bytes) {
		void* mem = realloc(str->data, str->bytes +
			STR_REALLOC_BYTES + (str->data == NULL ? 1 : 0));
		if (mem == NULL)  {
			return false; /* memoria insuficiente */
		}
		str->data   = (char*)mem;
		str->bytes += STR_REALLOC_BYTES;
	}

	str->data[str->count++] = c; /* el nuevo caracter */
	str->data[str->count  ] = '\0';
	return true;
}

/* La funcion retorna la posicion de donde se encuentra la clave 'key'
   En el caso que los parametros sean NULL o que no existe la clave
   en el conjunto de propiedades la funcion retorna 'UINT_MAX' */
size_t _zp_indexof(pzprop prop, cstr_t key)
{
	if (prop != NULL && key != NULL) {
		for (size_t i = 0; i < prop->count; i += 2) {
			if (strcmp(prop->data[i], key) == 0) {
				return i; /* posicion de la clave */
			}
		}
	}
	return UINT_MAX;
}

/* Convierte un caracter interpretado a un caracter de control los
   cuales son '\t', '\f', '\r' y '\n'
   
   Se puede decir que un caracter interpretado se entiende como por
   ejemplo escribir un salto de linea '\n' en un archivo con un editor
   de texto de esta manera "\n" identicamente pero como tal no es un
   caracter sino que se interpreta como un caracter de salto de linea
   sin ni siquiera serlo, es solo para que sea <visible> de que existe
   tal caracter de control en el valor de la clave
   
   La funcion recibe como primer parametro un array de caracteres
   que sera la salida o el resultado de la funcion y el segundo
   parametro el string. La funcion retorna el primer parametro 'out' */
char* _zp_convert(char* out, cstr_t in)
{
	size_t length;
	if (out != NULL && in != NULL && (length = strlen(in)) > 0) 
	{
		/* se incrementa 1 para copiar el caracter nulo '\0' */
		length++; 
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

/* Lee una linea de un archivo de propiedades. Los espacios "si
   existen en el inicio de la nueva linea" y los comentarios son
   ignorados. La funcion puede retornar false si se llego al final
   del archivo o ocurrio un error al reasignar memoria */
bool _zp_read_line(STRING* line, FILE* file)
{
	int c;                /* donde se guardara el caracter leido */
	bool begin   = true;  /* para eliminar los espacios iniciales de la linea */
	bool comment = false; /* para saltarse los comentarios */
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
			/* se saltan los espacios en blanco y los comentarios */
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
			c = EOF; 
			break; /* memoria insuficiente */
		}
	}

	return c != EOF;
}

/* Agrega un valor al conjunto de propiedades. Este valor puede ser
   una clave o el valor de la clave, el parametro 'is_value' lo indica.
   En el caso que sea un valor se debe llamar a la funcion '_zp_convert'
   para que cambie los caracteres de control interpretados por los
   no interpretados */
bool _zp_add_value(pzprop prop, cstr_t value, bool is_value)
{
	if (_zp_realloc(prop, 1)) {
		if (value == NULL) { /* el valor puede ser NULL */
			prop->data[prop->count - 1] = NULL;
			return true;
		} else {
			/* se crea una copia del valor y se asigina a 'prop->data'
			   En el caso que sea un valor de clave debe pasar por
			   la funcion '_zp_convert' */
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

/* Verifica que una cadena no este vacia "tomando en cuenta los
   espacios en blanco" y si lo esta retorna true, de lo contrario
   false. Esta funcion es usada para verificar que la clave no sea
   una cadena vacia */
bool _zp_is_empty(cstr_t value)
{
	size_t length; /* longitud de la cadena */
	if ((length = strlen(value)) > 0) {
		for (size_t i = 0; i < length; i++) {
			if (!isblank(value[i])) {
				return false; /* la cadena no esta vacia */
			}
		}
	}
	return true;
}

/* Se reasigna memoria para eliminar o sumar un espacio que puede
   ser para la clave o el valor. Para un nuevo espacio 'count_e'
   debe ser positivo y para eliminar ese espacio 'count_e' debe ser
   negativo, si es negativo y 'count_e' (sin signo) es mayor a
   'prop->count' el valor resultado es negativo y no seria un valor
   valido */
bool _zp_realloc(pzprop prop, int count_e)
{
	void* mem = realloc(prop->data, 
				(prop->count + count_e) * sizeof(void*));
	if (mem != NULL || (prop->count + count_e) == 0) 
	{
		prop->data = (char**)mem; /* mem puede ser NULL o no */
		prop->count += count_e;
		return true;
	}
	return false;
}