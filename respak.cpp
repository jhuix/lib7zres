// Copyright (c) 2018-present, Jhuix (Hui Jin) <jhuix0117@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or 
// without modification, are permitted provided that the 
// following conditions are met.
//
// Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above 
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials 
// provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifdef _WIN32
#ifndef STRICT
#define STRICT 1
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <wctype.h>
#include <tchar.h>
#endif
#include "lzma/C/7z.h"
#include "lzma/C/7zFile.h"
#include "7zres.h"
#include "librespak.h"


#ifdef RESPAK

static WRes OutFile_OpenUtf16(CSzFile *p, const UInt16 *name)
{
#ifdef USE_WINDOWS_FILE
  return OutFile_OpenW(p, (const WCHAR*)name);
#else
  CBuf buf;
  WRes res;
  Buf_Init(&buf);
  RINOK(Utf16_To_Char(&buf, name MY_FILE_CODE_PAGE_PARAM));
  res = OutFile_Open(p, (const char *)buf.data);
  Buf_Free(&buf, &g_Alloc);
  return res;
#endif
}

namespace SEVENZRES {
  bool PASCAL ExtractFileA(const char* srcfile, const char* filename, const char* destfile) {
    JRES::IRes* szres = JRES::Create7zRes(true);
#ifdef _WIN32
    if (!szres || !szres->Open(srcfile, L'\\'))
#else
    if (!szres || !szres->Open(srcfile, '/'))
#endif
      return false;

    bool result = false;
    char* buf = nullptr;
    size_t size = 0;
    void* res = szres->LoadResource(filename, &buf, size);
    if (buf) {
      CSzFile outFile;
      if (OutFile_Open(&outFile, destfile)) {
        if (File_Write(&outFile, buf, &size)) {
          result = true;
        }
#ifdef USE_WINDOWS_FILE
        SZRES_FILEHEADER* szhd = (SZRES_FILEHEADER*)res;
        if (szhd->mtime.dwHighDateTime || szhd->mtime.dwLowDateTime
          || szhd->ctime.dwHighDateTime || szhd->ctime.dwLowDateTime)
          SetFileTime(outFile.handle, &szhd->ctime, NULL, &szhd->mtime);
        File_Close(&outFile);
        if (szhd->attrib)
        {
          UInt32 attrib = szhd->attrib;
          /* p7zip stores posix attributes in high 16 bits and adds 0x8000 as marker.
          We remove posix bits, if we detect posix mode field */
          if ((attrib & 0xF0000000) != 0)
            attrib &= 0x7FFF;
          SetFileAttributesA(destfile, attrib);
        }
#else
        File_Close(&outFile);
#endif
      }
    }
    szres->FreeResource(res);
    szres->Release();
    return result;
  }

  bool PASCAL ExtractFileW(const wchar_t* srcfile, const wchar_t* filename, const wchar_t* destfile) {
    JRES::IRes* szres = JRES::Create7zRes(true);
#ifdef _WIN32
    if (!szres || !szres->Open(srcfile, L'\\'))
#else
    if (!szres || !szres->Open(srcfile, '/'))
#endif
      return false;

    bool result = false;
    char* buf = nullptr;
    size_t size = 0;
    void* res = szres->LoadResource(filename, &buf, size);
    if (buf) {
      CSzFile outFile;
      if (OutFile_OpenUtf16(&outFile, (const UInt16 *)destfile)) {
        if (File_Write(&outFile, buf, &size)) {
          result = true;
        }
#ifdef USE_WINDOWS_FILE
        SZRES_FILEHEADER* szhd = (SZRES_FILEHEADER*)res;
        if (szhd->mtime.dwHighDateTime || szhd->mtime.dwLowDateTime
          || szhd->ctime.dwHighDateTime || szhd->ctime.dwLowDateTime)
          SetFileTime(outFile.handle, &szhd->ctime, NULL, &szhd->mtime);
        File_Close(&outFile);
        if (szhd->attrib)
        {
          UInt32 attrib = szhd->attrib;
          /* p7zip stores posix attributes in high 16 bits and adds 0x8000 as marker.
          We remove posix bits, if we detect posix mode field */
          if ((attrib & 0xF0000000) != 0)
            attrib &= 0x7FFF;
          SetFileAttributesW(destfile, attrib);
        }
#else
        File_Close(&outFile);
#endif
      }
    }
    szres->FreeResource(res);
    szres->Release();
    return result;
  }

};
#endif
