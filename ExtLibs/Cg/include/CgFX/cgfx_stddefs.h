/*********************************************************************NVMH3****

File:  cgfx_stddefs.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:




******************************************************************************/

#ifndef __CGFX_STDDEFS_H
#define __CGFX_STDDEFS_H

#ifdef _WIN32
#include <windows.h>
#else
typedef unsigned int UINT;
typedef char BYTE;
typedef long HRESULT;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef void* LPVOID;
typedef const void * LPCVOID;
typedef float FLOAT;
typedef unsigned long ULONG;
typedef unsigned int DWORD;
#define S_OK 0x0
#define S_FALSE 0x1
#define E_FAIL 0x1
#define E_OUTOFMEMORY 0x2
#define E_INVALIDARG 0x3
#define SUCCEEDED(x) ((x) == S_OK)
#define FAILED(x) ((x) != S_OK)
#endif // _WIN32

#endif
