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

#pragma once
#ifndef _FILEPROP_H_
#define _FILEPROP_H_

#include <stdlib.h>
#include <stdint.h>

#ifndef ZPEXPORT
#	ifdef __ZPROP_LOCAL__
#		define ZPEXPORT __declspec(dllexport)
#	else
#		define ZPEXPORT __declspec(dllimport)
#	endif
#endif

#ifndef ZPCALL
#	define ZPCALL __stdcall
#endif

#ifdef __cplusplus
extern "C" {
#else
#	include <stdbool.h>
#endif

	/* string para version ASCII */
	typedef const char*    cstr_t;
	/* string para version UNICODE */
	typedef const wchar_t* cwstr_t;
	
#ifdef __ZPROP_LOCAL__
	/* struct reservada para el developer */
	typedef struct zprop
	{
		size_t count;
		char** data;
	} zprop, *pzprop;
#else

	/* handle del conjunto de propiedades */
	typedef void* pzprop;
#endif // __ZPROP_LOCAL__

	/**
	 * Retorna true si la clave existe en el conjunto de propiedades. Si los
	 * parámetros son inválidos la función retorna false o en el caso que no
	 * exista la clave
	 * 
	 * @param prop handle del conjunto de propiedades
	 * @param key  la clave a solicitar
	 * @return     existe la clave
	 */
	ZPEXPORT bool ZPCALL zp_contains(pzprop prop, cstr_t key);
	
	/**
	 * Remueve una clave y su valor del conjunto de propiedades. Si los
	 * parámetros son invalidos o la clave no existe la función retorna false.
	 * Tambien puede retornar false si ocurrió un error al reasignar memoria,
	 * en este caso debe consultar errno
	 * 
	 * @param prop handle del conjunto de propiedades
	 * @param key  la clave que sera removida
	 * @return     true si la funcion tuvo exito
	 */
	ZPEXPORT bool ZPCALL zp_remove(pzprop prop, cstr_t key);
	
	/**
	 * Reemplaza el valor de una clave por otro. El valor puede ser una cadena
	 * vacía o NULL. La función puede retornar false si los parámetro son
	 * inválidos, la clave no existe o un error al reasignar memoria, para lo
	 * ultimo mencionado debe consultar errno
	 * 
	 * @param prop  handle del conjunto de propiedades
	 * @param key   la clave del valor
	 * @param value el nuevo valor
	 * @return      true si la funcion tuvo exito
	 */
	ZPEXPORT bool ZPCALL zp_replace(pzprop prop, cstr_t key, cstr_t value);
	
	/**
	 * Agrega una nueva clave y su valor al conjunto de propiedades. Para que
	 * la función tenga éxito la clave no debe existir en el conjunto de
	 * propiedades, los parámetros deben ser validos además la clave no puede
	 * ser una cadena vacía y por último es posible que ocurra un error al
	 * reasignar memoria por lo cual debe consultar errno
	 * 
	 * @param prop  handle del conjunto de propiedades
	 * @param key   la clave del valor
	 * @param value el nuevo valor
	 * @return      true si la funcion tuvo exito
	 */
	ZPEXPORT bool ZPCALL zp_add(pzprop prop, cstr_t key, cstr_t value);
	
	/**
	 * Retorna el tamaño o la cantidad de propiedades existentes. La funcion
	 * puede retornar 0 si el parametro es invalido o no existen propiedades
	 * 
	 * @param prop handle del conjunto de propiedades
	 * @return     la cantidad de propiedades
	 */
	ZPEXPORT size_t ZPCALL zp_size(pzprop prop);
	
	/**
	 * Lee un archivo de propiedades .properties. La función lee todo el
	 * conjunto de propiedades en el archivo.
	 * 
	 * Una propiedad está constituida por su clave y su valor como un
	 * diccionario. Una propiedad puede o no tener un valor especificado, como
	 * por ejemplo “clave= <vacio>”. Si no se encuentra un separador como ‘=’
	 * o ‘:’ y solo se encuentra un espacio entre la clave y el valor, ese
	 * espacio es considerado un separador entre los dos, ejemplo
	 * 
	 * 		“clave <espacio> valor”.
	 * 
	 * El carácter '\' seguido de 't', 'f', 'r', u una 'n' es convertido a un
	 * carácter de control cuando es leído el valor. La función puede retornar
	 * false si los parámetros son inválidos, el archivo no existe, error de
	 * lectura o error al reasignar memoria. Debe consultar errno
	 * 
	 * Tenga en cuenta que la función también crea un espacio en memoria para
	 * el puntero pzprop como la función ‘zp_create’ si el puntero hace
	 * referencia a NULL, ejemplo:
	 * 
	 * 		pzprop prop = NULL;
	 * 		zp_read(&prop, “archive.properties”);
	 * 
	 * @param prop     puntero al handle del conjunto de propiedades
	 * @param filename nombre del archivo
	 * @return         true si la funcion tuvo exito
	 */
	ZPEXPORT bool ZPCALL zp_read_a(pzprop* prop, cstr_t filename);
	ZPEXPORT bool ZPCALL zp_read_w(pzprop* prop, cwstr_t filename);
	
	/**
	 * Escribe un archivo de propiedades. Si el valor contiene caracteres de
	 * control como ‘\t’, ‘\f’, ‘\r’ o ‘\n’ serán caracteres “visibles en el
	 * archivo” como por ejemplo:
	 * 
	 * 		“clave=lunes’\n’martes’\n’\t’’”
	 * 
	 * La función puede retornar false si los parámetros son inválidos o
	 * ocurrió un error en la apertura, escritura del archivo o al reasignar
	 * memoria. Debe consultar errno
	 * 
	 * @param prop     handle del conjunto de propiedades
	 * @param filename nombre del archivo
	 * @return         true si la funcion tuvo exito
	 */
	ZPEXPORT bool ZPCALL zp_write_a(pzprop prop, cstr_t filename);
	ZPEXPORT bool ZPCALL zp_write_w(pzprop prop, cwstr_t filename);

	/**
	 * Retorna el valor de la clave solicitada. La función puede retornar
	 * NULL si los parámetros son inválidos o la clave no existe en el
	 * conjunto de propiedades aunque en tal caso una clave puede tener un
	 * valor NULL, hay que tomar eso en cuenta
	 * 
	 * @param prop handle del conjunto de propiedades
	 * @param key  la clave del valor
	 * @return     el valor solicitado
	 */
	ZPEXPORT cstr_t ZPCALL zp_value(pzprop prop, cstr_t key);
	
	/**
	 * Las siguientes funciones convierte el valor en el tipo de dato que
	 * retorna cada función. Si el valor es inválido siendo NULL o un valor
	 * que no se puede convertir, la función retorna 0 como valor por defecto
	 * 
	 * @param prop handle del conjunto de propiedades
	 * @param key  la clave del valor
	 * @return     el valor solicitado
	 */
	ZPEXPORT int     ZPCALL zp_int   (pzprop prop, cstr_t key);
	ZPEXPORT int64_t ZPCALL zp_int64 (pzprop prop, cstr_t key);
	ZPEXPORT bool    ZPCALL zp_bool  (pzprop prop, cstr_t key);
	ZPEXPORT double  ZPCALL zp_double(pzprop prop, cstr_t key);

	/**
	 * Crea un espacio en memoria para el puntero 'pzprop'. La funcion
	 * puede retornar false si ocurre un error al solicitar memoria
	 * 
	 * @return handle del conjunto de propiedades
	 */
	ZPEXPORT pzprop ZPCALL zp_create(void);
	
	/**
	 * Libera la memoria usada por el puntero 'pzprop' y el conjunto de
	 * propiedades
	 * 
	 * @param prop
	 * @return 
	 */
	ZPEXPORT void ZPCALL zp_free(pzprop prop);

#if defined(__ZPROP_LOCAL__) || defined(_DEBUG)
	/**
	 * Imprime el conjunto de propiedades. Esta destinado para
	 * debugear aunque se puede modificar este header para poder usar
	 * esta funcion sin necesidad de estar debugeando
	 * 
	 * @param prop handle del conjunto de propiedades
	 * @return 
	 */
	ZPEXPORT void ZPCALL zp_print(pzprop prop);
#endif // defined(__ZPROP_LOCAL__) || defined(_DEBUG)

/* Compatibilidad con ASCII y UNICODE */
#ifdef UNICODE /* UNICODE */
#	define zp_read zp_read_w
#	define zp_write zp_write_w
#else          /* ASCII */
#	define zp_read zp_read_a
#	define zp_write zp_write_a
#endif // UNICODE

#ifdef __cplusplus
}
#endif
#endif // !_FILEPROP_H_
