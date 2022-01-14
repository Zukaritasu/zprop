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
#include <time.h>

#ifndef ZPEXPORT
#	ifdef _WINDLL
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

	typedef const char*    cstr_t;
	typedef const wchar_t* cwstr_t;
	
#ifdef __ZPROP_LOCAL__
	typedef struct zprop
	{
		size_t count;
		char** data;
	} zprop, *pzprop;
#else
	typedef void* pzprop;
#endif // __ZPROP_LOCAL__

	ZPEXPORT bool    ZPCALL zp_contains (pzprop prop, cstr_t key);
	ZPEXPORT bool    ZPCALL zp_remove   (pzprop prop, cstr_t key);
	ZPEXPORT bool    ZPCALL zp_replace  (pzprop prop, cstr_t key, cstr_t value);
	ZPEXPORT bool    ZPCALL zp_add      (pzprop prop, cstr_t key, cstr_t value);
	ZPEXPORT size_t  ZPCALL zp_size     (pzprop prop);
	ZPEXPORT bool    ZPCALL zp_read_a   (pzprop* prop, cstr_t filename);
	ZPEXPORT bool    ZPCALL zp_read_w   (pzprop* prop, cwstr_t filename);
	ZPEXPORT bool    ZPCALL zp_write_a  (pzprop prop, cstr_t filename);
	ZPEXPORT bool    ZPCALL zp_write_w  (pzprop prop, cwstr_t filename);

	ZPEXPORT cstr_t  ZPCALL zp_value    (pzprop prop, cstr_t key);
	ZPEXPORT int     ZPCALL zp_int      (pzprop prop, cstr_t key);
	ZPEXPORT int64_t ZPCALL zp_int64    (pzprop prop, cstr_t key);
	ZPEXPORT bool    ZPCALL zp_bool     (pzprop prop, cstr_t key);
	ZPEXPORT double  ZPCALL zp_double   (pzprop prop, cstr_t key);

	ZPEXPORT pzprop  ZPCALL zp_create   (void);
	ZPEXPORT void    ZPCALL zp_free     (pzprop prop);

#if defined(__ZPROP_LOCAL__) || defined(_DEBUG)
	ZPEXPORT void    ZPCALL zp_print    (pzprop prop);
#endif // defined(__ZPROP_LOCAL__) || defined(_DEBUG)

#ifdef UNICODE
#	define zp_read zp_read_w
#	define zp_write zp_write_w
#else
#	define zp_read zp_read_a
#	define zp_write zp_write_a
#endif // UNICODE

#ifdef __cplusplus
}
#endif
#endif // !_FILEPROP_H_
