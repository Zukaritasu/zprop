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

#ifndef _FILEPROP_H_
#define _FILEPROP_H_

#include <stdint.h>

#include "zpbase.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Retorna true si la clave existe en la lista de propiedades */
ZPEXPORT bool     zp_contains(cpzprop prop, cstr_t key);
/* Remueve una propiedad de la lista */
ZPEXPORT bool     zp_remove(pzprop prop, cstr_t key);
/* Crea una lista de propiedades */
ZPEXPORT pzprop   zp_create(void);
/* Remueve todas las propiedades de la lista */
ZPEXPORT void     zp_removeall(pzprop prop);
/* Libera la lista de propiedades */
ZPEXPORT void     zp_free(pzprop prop);
/* Imprime toda la lista de propiedades en consola */
ZPEXPORT void     zp_print(cpzprop prop);
/* Retorna el tamano de la lista de propiedades */
ZPEXPORT unsigned zp_size(cpzprop prop);

/* Las siguientes funciones que comienzan con `zp_add` agregan las
 * las propiedades a la lista. Si la funcion retorna false indica
 * que ocurrio un error, en ese caso debe consultar `errno`
 *
 * En caso de que la clave existe en la lista de propiedades entonces
 * el valor anterior es reemplazado por el nuevo
 */

ZPEXPORT bool     zp_add    (pzprop prop, cstr_t key, cstr_t value);
ZPEXPORT bool     zp_addc   (pzprop prop, cstr_t key, char value);
ZPEXPORT bool     zp_addb   (pzprop prop, cstr_t key, bool value);
ZPEXPORT bool     zp_addi   (pzprop prop, cstr_t key, int value);
ZPEXPORT bool     zp_addul  (pzprop prop, cstr_t key, uint32_t value);
ZPEXPORT bool     zp_addull (pzprop prop, cstr_t key, uint64_t value);
ZPEXPORT bool     zp_addi64 (pzprop prop, cstr_t key, int64_t value);

ZPEXPORT bool     zp_addf   (pzprop prop, cstr_t key, float value);
ZPEXPORT bool     zp_addd   (pzprop prop, cstr_t key, double value);
ZPEXPORT bool     zp_addld  (pzprop prop, cstr_t key, ldouble_t value);

/* Las funciones para cargar las propiedades de un archivo reciben
 * como primer paramtro un puntero a `pzprop`, esa variable debe
 * estar inicializada por la funcion `zp_create` o por `NULL`. Si
 * no lo esta causara problemas
 */
ZPEXPORT bool     zp_loada  (pzprop* prop, cstr_t fl_name);
ZPEXPORT bool     zp_loadw  (pzprop* prop, cwstr_t fl_name);
ZPEXPORT bool     zp_savea  (cpzprop prop, cstr_t fl_name);
ZPEXPORT bool     zp_savew  (cpzprop prop, cwstr_t fl_name);

/* Compatibilidad con ASCII y UNICODE */
#ifdef UNICODE /* UNICODE */
#	define zp_load zp_loadw
#	define zp_save zp_savew
#else /* ASCII */
#	define zp_load zp_loada
#	define zp_save zp_loadw
#endif /* UNICODE */

/* Las siguientes funciones que comienzan con `zp_get` en caso de
 * no existir la clave o el valor es NULL, las funciones retornaran
 * 0 como valor por defecto, para char '\0' y bool 'false'
 */
ZPEXPORT cstr_t    zp_get   (cpzprop prop, cstr_t key);
ZPEXPORT char      zp_getc  (cpzprop prop, cstr_t key);
ZPEXPORT int       zp_geti  (cpzprop prop, cstr_t key);
ZPEXPORT int64_t   zp_geti64(cpzprop prop, cstr_t key);
ZPEXPORT bool      zp_getb  (cpzprop prop, cstr_t key);
ZPEXPORT uint32_t  zp_getul (cpzprop prop, cstr_t key);
ZPEXPORT uint64_t  zp_getull(cpzprop prop, cstr_t key);
ZPEXPORT double    zp_getd  (cpzprop prop, cstr_t key);
ZPEXPORT float     zp_getf  (cpzprop prop, cstr_t key);
ZPEXPORT ldouble_t zp_getld (cpzprop prop, cstr_t key);

/* Asigna los valores definidos por `template_` en la lista de
 * argumentos. En caso de error la funcion retorna false. La
 * plantilla puede contener las mismas banderas que en la funcion
 * `scanf` porque desde el codigo fuente se llama la funcion
 * `vsscanf` para la lista de argumentos
 */
ZPEXPORT bool      zp_getv  (cpzprop prop, cstr_t key,
							 cstr_t template_, ...);

/* Recuerda que para agregar una estructura sus campos deben estar
 * inicializados para no guardar datos basura.
 * Puede usar `memset` o asignarle los valores correspondientes
 *
 * Si esta usando `Winapi` y ademas usa las estructuras RECT,
 * SIZE y POINT entonces puede hacer cast con las estructuras de
 * esta libreria porque contiene los mismos campos
 */
#if USE_FUNCTIONS_GEOMETRY
ZPEXPORT bool      zp_addr  (pzprop prop, cstr_t key, cpzprect r);
ZPEXPORT bool      zp_addp  (pzprop prop, cstr_t key, cpzppoint p);
ZPEXPORT bool      zp_addsz (pzprop prop, cstr_t key, cpzpsize sz);

ZPEXPORT bool      zp_getr  (cpzprop prop, cstr_t key, pzprect r);
ZPEXPORT bool      zp_getp  (cpzprop prop, cstr_t key, pzppoint p);
ZPEXPORT bool      zp_getsz (cpzprop prop, cstr_t key, pzpsize sz);
#endif /* USE_FUNCTIONS_GEOMETRY */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _FILEPROP_H_ */
