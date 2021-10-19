//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
// - Autor Zukaritasu
// - Copyright (c) 2021
// - Nombre de archivo zprop.h
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

#pragma once

#ifndef ZPROP_H
#define ZPROP_H

#include <stdbool.h>
#include <stdlib.h>
#include "zpropdef.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		zProp_OK = 0,           /* El procedimiento finalizo correctamente */
		zProp_FILENOTFOUND = 1, /* El archivo no existe */
		zProp_ACCESSDENIED = 2, /* Acceso denegado */
		zProp_UNKNOWN = 3,      /* Error desconocido */
		zProp_SYNTAX = 4,       /* Error de sintaxis */
		zProp_OUTOFMEMORY = 5   /* Memoria insuficiente */
	} zPropError;

	typedef const char* zText;

	_zSuccess_(zProp != NULL)
	_zReturn_(zProp) zCreateProp();

	_zSuccess_(return == true)
	_zReturn_(bool) zContainsKey(zProp props, zText key);

	_zSuccess_(return == true)
	_zReturn_(bool) zRemoveKey(zProp props, zText key);

	_zReturn_(void) zFreeProp(zProp props);

	_zSuccess_(return != NULL)
	_zReturn_(zText) zGetValue(zProp props, zText key);

	_zSuccess_(return == zProp_OK)
	_zReturn_(zPropError) zAddProp(zProp props, zText key, zText value);

	_zReturn_(int) zGetCountProp(zProp props);

	_zSuccess_(return == zProp_OK)
	_zReturn_(zPropError) zPropReadA(zProp* props, zText filename);
	_zSuccess_(return == zProp_OK)
	_zReturn_(zPropError) zPropReadW(zProp* props, const wchar_t* filename);
	_zSuccess_(return == zProp_OK)
	_zReturn_(zPropError) zPropWriteA(zProp props, zText filename);
	_zSuccess_(return == zProp_OK)
	_zReturn_(zPropError) zPropWriteW(zProp props, const wchar_t* filename);

	_zReturn_(void) zPrintProp(zProp props);

#ifdef UNICODE
#define zPropRead zPropReadW
#define zPropWrite zPropWriteW
#else
#define zPropRead zPropReadA
#define zPropWrite zPropWriteA
#endif // UNICODE

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !ZPROP_H
