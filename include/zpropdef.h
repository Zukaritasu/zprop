//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
// - Autor Zukaritasu
// - Copyright (c) 2021
// - Nombre de archivo zpropdef.h
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

#pragma once
#ifndef ZPROPDEF_H
#define ZPROPDEF_H

#ifndef DLLEXPORT
#ifdef _WINDLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif // _WINDLL
#endif // !DLLEXPORT

#ifndef _zOut_
#define _zOut_
#endif // !_zOut_
#ifndef _zIn_
#define _zIn_
#endif // !_zIn_
#ifndef _zInout_
#define _zInout_
#endif // !_zInout_
#ifndef _zReturn_
#define _zReturn_(type) type DLLEXPORT __stdcall
#endif // !_zReturn_
#ifndef _zSuccess_
#define _zSuccess_(exp)
#endif // !_zSuccess_

typedef void* zProp;

#endif // !ZPROPDEF_H
