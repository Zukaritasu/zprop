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

#ifdef __cplusplus
extern "C" {
#else
#	include <stdbool.h>
#endif

	typedef const char*    cstr_t;
	typedef const wchar_t* cwstr_t;
	typedef long double    ldouble_t;

#ifdef __ZPROP_LOCAL__
	/* struct reservada para el developer */
	typedef struct zprop
	{
		size_t count;
		char **data;
	} zprop, *pzprop;
#else
	/* handle del conjunto de propiedades */
	typedef void *pzprop;
#endif /* __ZPROP_LOCAL__ */

	ZPEXPORT bool      zp_contains (pzprop prop, cstr_t key);
	ZPEXPORT bool      zp_remove   (pzprop prop, cstr_t key);
	ZPEXPORT void      zp_removeall(pzprop prop);
	ZPEXPORT pzprop    zp_create   (void);
	ZPEXPORT void      zp_free     (pzprop* prop);
	ZPEXPORT void      zp_print    (pzprop prop);

	ZPEXPORT bool      zp_add      (pzprop prop, cstr_t key, cstr_t value);
	ZPEXPORT bool      zp_addc     (pzprop prop, cstr_t key, char value);
	ZPEXPORT bool      zp_addb     (pzprop prop, cstr_t key, bool value);
	ZPEXPORT bool      zp_addi     (pzprop prop, cstr_t key, int value);
	ZPEXPORT bool      zp_addf     (pzprop prop, cstr_t key, float value);
	ZPEXPORT bool      zp_addd     (pzprop prop, cstr_t key, double value);
	ZPEXPORT bool      zp_addul    (pzprop prop, cstr_t key, uint32_t value);
	ZPEXPORT bool      zp_addull   (pzprop prop, cstr_t key, uint64_t value);
	ZPEXPORT bool      zp_addi64   (pzprop prop, cstr_t key, int64_t value);
	ZPEXPORT bool      zp_addld    (pzprop prop, cstr_t key, ldouble_t value);

	ZPEXPORT size_t    zp_size     (pzprop prop);

	ZPEXPORT bool      zp_read_a   (pzprop *prop, cstr_t filename);
	ZPEXPORT bool      zp_read_w   (pzprop *prop, cwstr_t filename);
	ZPEXPORT bool      zp_write_a  (pzprop prop, cstr_t filename);
	ZPEXPORT bool      zp_write_w  (pzprop prop, cwstr_t filename);

	ZPEXPORT cstr_t    zp_value    (pzprop prop, cstr_t key);
	ZPEXPORT char      zp_valuec   (pzprop prop, cstr_t key);
	ZPEXPORT int       zp_valuei   (pzprop prop, cstr_t key);
	ZPEXPORT int64_t   zp_valuei64 (pzprop prop, cstr_t key);
	ZPEXPORT bool      zp_valueb   (pzprop prop, cstr_t key);
	ZPEXPORT ldouble_t zp_valueld  (pzprop prop, cstr_t key);
	ZPEXPORT uint32_t  zp_valueul  (pzprop prop, cstr_t key);
	ZPEXPORT uint64_t  zp_valueull (pzprop prop, cstr_t key);
	ZPEXPORT double    zp_valued   (pzprop prop, cstr_t key);
	ZPEXPORT float     zp_valuef   (pzprop prop, cstr_t key);

/* Compatibilidad con ASCII y UNICODE */
#ifdef UNICODE /* UNICODE */
#	define zp_read     zp_read_w
#	define zp_write    zp_write_w
#else /* ASCII */
#	define zp_read     zp_read_a
#	define zp_write    zp_write_a
#endif /* UNICODE */

#ifdef __cplusplus
}
#endif
#endif /* _FILEPROP_H_ */
