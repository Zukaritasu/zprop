//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
// - Autor Zukaritasu
// - Copyright (c) 2021
// - Nombre de archivo zprop.c
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

#include "zprop.h"

#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <string.h>
#include <sys\stat.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

typedef struct
{
	int count;
	char** props;
} _zProp_;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

#define SIZE_BUFFER 256
#define SZ_CPP      sizeof(char**)
#define DECLARE_LV(props) _zProp_* _props_ = (_zProp_*)props;

#define ZPROP_READ(rdf) \
		int file; \
		errno_t openError; \
		zPropError error = zProp_OK; \
		if ((openError = rdf(&file, filename, _O_RDONLY, _SH_DENYWR, 0)) == 0) { \
			_zPropRead(props, &error, file); \
		} \
		if (openError != 0) { \
			if (openError == ENOENT) { \
				error = zProp_FILENOTFOUND; \
			} else { \
				error = zProp_UNKNOWN; \
			} \
		} \
		return error;

#define ZPROP_WRITE(wtf) \
		int file; \
		errno_t openError; \
		zPropError error = zProp_OK; \
		if ((openError = wtf(&file, filename, _O_WRONLY | _O_TRUNC | _O_CREAT, _SH_DENYNO, _S_IWRITE)) == 0) { \
			_zPropWrite(props, &error, file); \
		} \
		if (openError != 0) { \
			error = zProp_UNKNOWN; \
		} \
		return error;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void _zPropRead(zProp* props, zPropError* error, int file);
void _zPropWrite(zProp props, zPropError* error, int file);
bool zAddKeyOrValue(_zProp_* props, zText str);

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

/**
*
*
*/
_zReturn_(zProp) zCreateProp()
{
	_zProp_* props = (_zProp_*)malloc(sizeof(_zProp_));
	if (props != NULL)
		/* se rellena el puntero a la estructura con ceros */
		memset(props, 0, sizeof(_zProp_));
	return (zProp)props;
}

/**
*
*
*/
_zReturn_(bool) zContainsKey(zProp props, zText key)
{
	return zGetValue(props, key) != NULL;
}

/**
*
*
*/
_zReturn_(bool) zRemoveKey(zProp props, zText key)
{
	DECLARE_LV(props);
	if (props != NULL && key != NULL && zContainsKey(props, key)) {
		for (int i = 1; i < _props_->count; i += 2) {
			if (strcmp(_props_->props[i - 1], key) == 0) {
				/* se libera la memoria usada por la clave y el valor */
				free(_props_->props[i - 1]); /* key */
				free(_props_->props[i]);     /* value */

				/* se mueven las claves y valores en su nueva posicion */
				for (++i; i < _props_->count; i++)
					_props_->props[i - 2] = _props_->props[i];
				_props_->props = (char**)realloc(
					_props_->props, (_props_->count -= 2) * SZ_CPP);
				break;
			}
		}
		return true;
	}
	return false;
}

/**
*
*
*/
_zReturn_(void) zFreeProp(zProp props)
{
	DECLARE_LV(props);
	for (int i = 0; i < _props_->count; i++) {
		/* se libera la memoria usada por las claves y valores */
		free(_props_->props[i]);
		if ((i + 1) == _props_->count) {
			/* se libera la memoria usada por el array de string */
			free(_props_->props);
			break;
		}
	}
	/* se libera la memoria usada por la estructura _zProp_ */
	free(_props_);
}

/**
*
*
*/
_zReturn_(zText) zGetValue(zProp props, zText key)
{
	DECLARE_LV(props);
	if (props != NULL && key != NULL) {
		for (int i = 1; i < _props_->count; i += 2) {
			if (strcmp(_props_->props[i - 1], key) == 0) {
				return _props_->props[i]; /* se retorna el valor */
			}
		}
	}
	return NULL; /* no se encontro la clave */
}

/**
*
*
*/
_zReturn_(zPropError) zAddProp(zProp props, zText key, zText value)
{
	DECLARE_LV(props);
	if (props != NULL && key != NULL) {
		if (!zAddKeyOrValue((_zProp_*)props, key))     // key
			return zProp_OUTOFMEMORY;
		if (!zAddKeyOrValue((_zProp_*)props, value)) { // value
			/* se debe eliminar la clave ingresada si ocurrio un error */
			free(_props_->props[--_props_->count]);
			_props_->props = (char**)realloc(
				 _props_->props, _props_->count * SZ_CPP);
			return zProp_OUTOFMEMORY;
		}
	}
	return zProp_OK;
}

/**
*
*
*/
_zReturn_(int) zGetCountProp(zProp props)
{
	DECLARE_LV(props);
	return (props == NULL || _props_->count == 0) ? 0 : _props_->count / 2;
}

/**
*
*
*/
_zReturn_(zPropError) zPropReadA(zProp* props, zText filename)
{
	ZPROP_READ(_sopen_s);
}

/**
*
*
*/
_zReturn_(zPropError) zPropReadW(zProp* props, const wchar_t* filename)
{
	ZPROP_READ(_wsopen_s);
}

/**
*
*
*/
bool zAddKeyOrValue(_zProp_* props, zText str)
{
	/* se solicita mas memoria para el array de string */
	void* mem = realloc(props->props, (props->count + 1) * SZ_CPP);
	if (mem != NULL) {
		props->props = (char**)mem;
		/* se agrega la clave o el valor en la ultima posicion */
		if ((props->props[props->count++] = _strdup(str)) != NULL)
			return true;
		props->props = (char**)realloc(props->props, (--props->count) * SZ_CPP);
	}
	return false;
}

/**
*
*
*/
void _zPropRead(zProp* props, zPropError* error, int file)
{
	char buf[SIZE_BUFFER];   /* los datos leidos */
	char* copy_bt = NULL;    /* en este buffer se guarda temporalmente la clave o valor */
	int i = SIZE_BUFFER;     /* posicion en el buffer */
	int j = 0;               /* posicion en el buffer temporal */
	int nBytes = 0;          /* numero de bytes del buffer temporal */
	int count = 0;           /* cantidad de elementos leidos */

	bool _cp_val_  = false;  /* indica que se esta copiando el valor */
	bool _cp_key_  = false;  /* indica que se esta copiando la clave */
	bool _newLine_ = false;  /* indica una nueva linea en el archivo */
	bool _comment_ = false;  /* indica que la linea leida es un comentario */

	*props = zCreateProp();
	if (*props == NULL) // memoria insuficiente
	{
		*error = zProp_OUTOFMEMORY;
		goto finish;
	}

	for (;; i++) {
		bool _stop_ = (((i == count) && i != SIZE_BUFFER) || count == -1);
		if (_cp_val_ || _cp_key_ || _stop_)
		{
			if (_cp_key_ && _stop_) {
				// error de sintaxis
			}
			if (_cp_val_ && (_newLine_ || _stop_)) {
				_cp_val_ = false;
				// agregar el valor de la clave
				copy_bt[j] = '\0';
				if (!zAddKeyOrValue((_zProp_*)*props, copy_bt)) {
					*error = zProp_OUTOFMEMORY;
					zFreeProp(*props);
					break; // memoria insuficiente
				}
				j = 0;
			}

			if (_stop_)
			{
				break;
			}
		}

		if (i == SIZE_BUFFER)
		{
			i = 0;
			if ((count = _read(file, buf, SIZE_BUFFER)) == -1) { /* eof */
				continue;
			}
		}

		if (buf[i] == '\r' || buf[i] == '\n')
		{
			_comment_ = false;
			_newLine_ = true;
			if (_cp_key_) {
				// error de sintaxis
				*error = zProp_SYNTAX;
				zFreeProp(*props);
				break;
			}
			continue;
		}

		if (_newLine_)
		{
			if (buf[i] == ' ' || buf[i] == '\t') {
				continue;
			}
			_newLine_ = false;
			if (buf[i] == '#' || buf[i] == ';' || buf[i] == '!') {
				_comment_ = true;
			}
		}

		if (_comment_)
		{
			continue;
		}

		if (!_comment_ && !_cp_key_ && !_cp_val_) {
			_cp_key_ = true;
		}

		if (_cp_key_ && buf[i] != ' ')
		{
			if (buf[i] == '=' || buf[i] == ':') {
				if (j == 0)
				{
					/*
					Error de sintaxis.
					No existe la clave del valor. Ejemplo:
					<vacio> = <valor>
					*/
					*error = zProp_SYNTAX;
					zFreeProp(*props);
					break;
				}
				// agregar la clave copiada
				copy_bt[j] = '\0';
				if (!zAddKeyOrValue((_zProp_*)*props, copy_bt)) {
					*error = zProp_OUTOFMEMORY;
					zFreeProp(*props);
					break; // memoria insuficiente
				}
				j = 0;
				_cp_val_ = true;
				_cp_key_ = false;
				continue;
			}
		}

		if (_cp_key_ || _cp_val_)
		{
			if (copy_bt == NULL) {
				copy_bt = (char*)malloc((nBytes = 1024) + 1);
				if (copy_bt == NULL) {
					*error = zProp_OUTOFMEMORY;
					zFreeProp(*props);
					break; // memoria insuficiente
				}
			}
			else if (j == nBytes) {
				void* mem = (char*)realloc(copy_bt, (nBytes += 256) + 1);
				if (mem == NULL) {
					*error = zProp_OUTOFMEMORY;
					zFreeProp(*props);
					break; // memoria insuficiente
				}
				copy_bt = (char*)mem;
			}
			// copiar clave o valor en el buffer
			copy_bt[j++] = buf[i];
		}
	}

	if (copy_bt != NULL) {
		/* se libera la memoria usada por el buffer temporal */
		free(copy_bt);
	}
finish:
	_close(file);
}

/**
*
*
*/
_zReturn_(zPropError) zPropWriteA(zProp props, zText filename)
{
	ZPROP_WRITE(_sopen_s);
}

/**
*
*
*/
_zReturn_(zPropError) zPropWriteW(zProp props, const wchar_t* filename)
{
	ZPROP_WRITE(_wsopen_s);
}

/**
*
*
*/
void _zPropWrite(zProp props, zPropError* error, int file)
{
	DECLARE_LV(props);
	if (_props_ != NULL && _props_->count > 0) {
		char s = '=', n = '\n'; /* caracteres para la sintaxis */

		for (int i = 1; i < _props_->count; i += 2) {
			_write(file, _props_->props[i - 1], 
				(unsigned int)strlen(_props_->props[i - 1]));
			_write(file, &s, 1);
			_write(file, _props_->props[i], 
				(unsigned int)strlen(_props_->props[i]));

			/* se descarta el salto de linea si es la ultima
			propiedad por escribir */
			if ((i + 1) < _props_->count) {
				_write(file, &n, 1);
			}
		}
	}
	_close(file);
}

/**
*
*
*/
_zReturn_(void) zPrintProp(zProp props)
{
	DECLARE_LV(props);
	if (props != NULL)
	{
		for (int i = 1; i < _props_->count; i += 2) {
			printf_s("%s=%s\n", _props_->props[i - 1], _props_->props[i]);
		}
	}
}