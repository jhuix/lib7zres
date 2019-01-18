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
#include <stdio.h>
#include <string.h>

#include "lzma/C/CpuArch.h"
#include "lzma/C/7z.h"
#include "lzma/C/7zAlloc.h"
#include "lzma/C/7zBuf.h"
#include "lzma/C/7zCrc.h"
#include "lzma/C/7zFile.h"
#include "lzma/C/7zVersion.h"
#include "7zres.h"
#include "VERSION"

namespace SEVENZRES {

  #define kInputBufSize ((size_t)1 << 18)

  static const ISzAlloc g_Alloc = { SzAlloc, SzFree };

  static bool File_IsOpened(CSzFile *p)
  {
#ifdef USE_WINDOWS_FILE
    if (p->handle != INVALID_HANDLE_VALUE)
    {
      return true;
    }
#else
    if (p->file != NULL)
    {
      return true;
    }
#endif
    return false;
  }

  static int Buf_EnsureSize(CBuf *dest, size_t size)
  {
    if (dest->size >= size)
      return 1;
    Buf_Free(dest, &g_Alloc);
    return Buf_Create(dest, size, &g_Alloc);
  }

#ifndef _WIN32
#define _USE_UTF8
#endif

  /* #define _USE_UTF8 */

#ifdef _USE_UTF8

#define _UTF8_START(n) (0x100 - (1 << (7 - (n))))

#define _UTF8_RANGE(n) (((UInt32)1) << ((n) * 5 + 6))

#define _UTF8_HEAD(n, val) ((Byte)(_UTF8_START(n) + (val >> (6 * (n)))))
#define _UTF8_CHAR(n, val) ((Byte)(0x80 + (((val) >> (6 * (n))) & 0x3F)))

  static size_t Utf16_To_Utf8_Calc(const UInt16 *src, const UInt16 *srcLim)
  {
    size_t size = 0;
    for (;;)
    {
      UInt32 val;
      if (src == srcLim)
        return size;

      size++;
      val = *src++;

      if (val < 0x80)
        continue;

      if (val < _UTF8_RANGE(1))
      {
        size++;
        continue;
      }

      if (val >= 0xD800 && val < 0xDC00 && src != srcLim)
      {
        UInt32 c2 = *src;
        if (c2 >= 0xDC00 && c2 < 0xE000)
        {
          src++;
          size += 3;
          continue;
        }
      }

      size += 2;
    }
  }

  static Byte *Utf16_To_Utf8(Byte *dest, const UInt16 *src, const UInt16 *srcLim)
  {
    for (;;)
    {
      UInt32 val;
      if (src == srcLim)
        return dest;

      val = *src++;

      if (val < 0x80)
      {
        *dest++ = (char)val;
        continue;
      }

      if (val < _UTF8_RANGE(1))
      {
        dest[0] = _UTF8_HEAD(1, val);
        dest[1] = _UTF8_CHAR(0, val);
        dest += 2;
        continue;
      }

      if (val >= 0xD800 && val < 0xDC00 && src != srcLim)
      {
        UInt32 c2 = *src;
        if (c2 >= 0xDC00 && c2 < 0xE000)
        {
          src++;
          val = (((val - 0xD800) << 10) | (c2 - 0xDC00)) + 0x10000;
          dest[0] = _UTF8_HEAD(3, val);
          dest[1] = _UTF8_CHAR(2, val);
          dest[2] = _UTF8_CHAR(1, val);
          dest[3] = _UTF8_CHAR(0, val);
          dest += 4;
          continue;
        }
      }

      dest[0] = _UTF8_HEAD(2, val);
      dest[1] = _UTF8_CHAR(1, val);
      dest[2] = _UTF8_CHAR(0, val);
      dest += 3;
    }
  }

  static SRes Utf16_To_Utf8Buf(CBuf *dest, const UInt16 *src, size_t srcLen)
  {
    size_t destLen = Utf16_To_Utf8_Calc(src, src + srcLen);
    destLen += 1;
    if (!Buf_EnsureSize(dest, destLen))
      return SZ_ERROR_MEM;
    *Utf16_To_Utf8(dest->data, src, src + srcLen) = 0;
    return SZ_OK;
  }

#endif

  static SRes Utf16_To_Char(CBuf *buf, const UInt16 *s
#ifndef _USE_UTF8
    , UINT codePage
#endif
  )
  {
    unsigned len = 0;
    for (len = 0; s[len] != 0; len++);

#ifndef _USE_UTF8
    {
      unsigned size = len * 3 + 100;
      if (!Buf_EnsureSize(buf, size))
        return SZ_ERROR_MEM;
      {
        buf->data[0] = 0;
        if (len != 0)
        {
          char defaultChar = '_';
          BOOL defUsed;
          unsigned numChars = 0;
          numChars = WideCharToMultiByte(codePage, 0, (LPCWCH)s, len, (char *)buf->data, size, &defaultChar, &defUsed);
          if (numChars == 0 || numChars >= size)
            return SZ_ERROR_FAIL;
          buf->data[numChars] = 0;
        }
        return SZ_OK;
      }
    }
#else
    return Utf16_To_Utf8Buf(buf, s, len);
#endif
  }

#ifndef _USE_UTF8
static UINT g_FileCodePage = CP_ACP;
#define MY_FILE_CODE_PAGE_PARAM ,g_FileCodePage
#else
#define MY_FILE_CODE_PAGE_PARAM
#endif

  /////////////////////////////////////////////////////////////////
  C7zRes::C7zRes(bool ignorecase)
    : total_packsize_(0)
    , total_unpsize_(0)
    , ignorecase_(ignorecase)
    , errcode_(SZ_OK){
#ifndef _USE_UTF8
    g_FileCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
#endif
    memset(&lookStream_, 0, sizeof(lookStream_));
    memset(&db_, 0, sizeof(db_));
    FileInStream_CreateVTable(&archiveStream_);
    File_Construct(&archiveStream_.file);
    LookToRead2_CreateVTable(&lookStream_, False);
    lookStream_.buf = NULL;
  }

  C7zRes::~C7zRes() {
    Close();
  }

  void C7zRes::Close() {
    SzArEx_Free(&db_, &g_Alloc);
    if (lookStream_.buf) {
      ISzAlloc_Free(&g_Alloc, lookStream_.buf);
      lookStream_.buf = NULL;
    }
    File_Close(&archiveStream_.file);
    for (auto it : fileheadersW_) {
      if (it.second->data) {
        free(it.second->data);
        it.second->data = nullptr;
      }
      delete it.second;
    }
    fileheadersW_.clear();
    fileheadersA_.clear();
    total_packsize_ = 0;
    total_unpsize_ = 0;
  }

  void C7zRes::Release() {
    delete this;
  }

  bool C7zRes::Open(const char* filename, char path_sep) {
    Close();
    if (SZ_OK != InFile_Open(&archiveStream_.file, filename))
      return false;

    wchar_t sep = 0;
    ((char*)&sep)[0] = path_sep;
    return ListFiles(sep);
  }

  bool C7zRes::Open(const wchar_t* filename, wchar_t path_sep) {
    Close();
    if (SZ_OK != InFile_OpenW(&archiveStream_.file, filename))
      return false;
    return ListFiles(path_sep);
  }
 
  bool C7zRes::ListFiles(wchar_t path_sep) {
    if (!File_IsOpened(&archiveStream_.file)) {
      SetErrorCode(SZ_ERROR_FAIL);
      return false;
    }

    SRes res = SZ_OK;
    UInt16 *temp = NULL;
    do {
      if (!lookStream_.buf)
      {
        lookStream_.buf = (Byte*)ISzAlloc_Alloc(&g_Alloc, kInputBufSize);
        if (!lookStream_.buf) {
          res = SZ_ERROR_MEM;
          break;
        }

        lookStream_.bufSize = kInputBufSize;
        lookStream_.realStream = &archiveStream_.vt;
        LookToRead2_Init(&lookStream_);
        CrcGenerateTable();
      }

      SzArEx_Init(&db_);
      res = SzArEx_Open(&db_, &lookStream_.vt, &g_Alloc, &g_Alloc);
      if (res != SZ_OK)
        break;

      UInt32 i = 0;
      size_t tempSize = 0;
      CBuf buf;
      Buf_Init(&buf);

      for (i = 0; i < db_.NumFiles; i++)
      {
        // const CSzFileItem *f = db.Files + i;
        size_t len;
        bool isDir = SzArEx_IsDir(&db_, i);
        if (isDir)
          continue;

        len = SzArEx_GetFileNameUtf16(&db_, i, NULL);

        if (len > tempSize)
        {
          SzFree(NULL, temp);
          tempSize = len;
          temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
          if (!temp)
          {
            res = SZ_ERROR_MEM;
            break;
          }
        }

        SzArEx_GetFileNameUtf16(&db_, i, temp);
        /*
        if (SzArEx_GetFullNameUtf16_Back(&db_, i, temp + len) != temp)
        {
        res = SZ_ERROR_FAIL;
        break;
        }
        */

        UInt64 fileSize = SzArEx_GetFileSize(&db_, i);
        total_unpsize_ += fileSize;
        SZRES_FILEHEADER* rhd = new SZRES_FILEHEADER;
        rhd->index = i;
        rhd->filesize = fileSize;
        rhd->filename = (wchar_t*)temp;
        rhd->attrib = 0;
        rhd->mtime.dwHighDateTime = rhd->mtime.dwLowDateTime = 0;
        rhd->ctime.dwHighDateTime = rhd->ctime.dwLowDateTime = 0;
        rhd->data = nullptr;
#ifdef USE_WINDOWS_FILE
        if (SzBitWithVals_Check(&db_.MTime, i))
        {
          const CNtfsFileTime *t = &db_.MTime.Vals[i];
          rhd->mtime.dwLowDateTime = (DWORD)(t->Low);
          rhd->mtime.dwHighDateTime = (DWORD)(t->High);
        }
        if (SzBitWithVals_Check(&db_.CTime, i))
        {
          const CNtfsFileTime *t = &db_.CTime.Vals[i];
          rhd->ctime.dwLowDateTime = (DWORD)(t->Low);
          rhd->ctime.dwHighDateTime = (DWORD)(t->High);
        }
        if (SzBitWithVals_Check(&db_.Attribs, i))
        {
          rhd->attrib = db_.Attribs.Vals[i];
        }
#endif
        //Path sep default value is L'/'
        if (path_sep && path_sep != L'/') {
          wchar_t* ch = (wchar_t*)temp;
          while (*ch) {
            if (L'/' == *ch) *ch = path_sep;
            ++ch;
          }
        }
        if (ignorecase_) {
          wchar_t* ch = (wchar_t*)temp;
          while (*ch) {
            if ((*ch) >= L'A' && (*ch) <= L'Z')
              *ch = (*ch) + 32;
            ++ch;
          }
        }
        fileheadersW_[(wchar_t*)temp] = rhd;
        Utf16_To_Char(&buf, temp MY_FILE_CODE_PAGE_PARAM);
        fileheadersA_[(char*)buf.data] = rhd;
        Buf_Free(&buf, &g_Alloc);
      }
    } while (false);

    if (temp) {
      SzFree(NULL, temp);
      temp = NULL;
    }
    SetErrorCode(res);
    if (res != SZ_OK) {
      File_Close(&archiveStream_.file);
      return false;
    }

    return true;
  }

  void* C7zRes::LoadResource(const char* id, char** buf, size_t& bufsize) {
    if (!File_IsOpened(&archiveStream_.file)) {
      SetErrorCode(SZ_ERROR_FAIL);
      return NULL;
    }
    std::string fileid = id;
    if (ignorecase_) {
      char* ch = (char*)fileid.c_str();
      while (*ch) {
        if ((*ch) >= 'A' && (*ch) <= 'Z')
          *ch = (*ch) + 32;
        ++ch;
      }
    }
    std::map<std::string, SZRES_FILEHEADER* >::iterator it;
    it = fileheadersA_.find(fileid);
    if (it == fileheadersA_.end() || !it->second) {
      SetErrorCode(SZ_ERROR_FAIL);
      return NULL;
    }
    return Extract(it->second, buf, bufsize);
  }

  void* C7zRes::LoadResource(const wchar_t* id, char** buf, size_t& bufsize) {
    if (!File_IsOpened(&archiveStream_.file)) {
      SetErrorCode(SZ_ERROR_FAIL);
      return NULL;
    }

    std::wstring fileid = id;
    if (ignorecase_) {
      wchar_t* ch = (wchar_t*)fileid.c_str();
      while (*ch) {
        if ((*ch) >= L'A' && (*ch) <= L'Z')
          *ch = (*ch) + 32;
        ++ch;
      }
    }
    std::map<std::wstring, SZRES_FILEHEADER* >::iterator it;
    it = fileheadersW_.find(fileid);
    if (it == fileheadersW_.end() || !it->second) {
      SetErrorCode(SZ_ERROR_FAIL);
      return NULL;
    }
    return Extract(it->second, buf, bufsize);
  }

  void* C7zRes::Extract(SZRES_FILEHEADER* szhd, char** buf, size_t& bufsize)
  {
    if (!buf || ((*buf) && szhd->filesize > (UInt64)bufsize)) {
      bufsize = (size_t)szhd->filesize;
      SetErrorCode(SZ_OK);
      return NULL;
    }

    /*
    if you need cache, use these 3 variables.
    if you use external function, you can make these variable as static.
    */
    UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
    Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
    size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */
    size_t offset = 0;
    size_t outSizeProcessed = 0;
    SRes res = SzArEx_Extract(&db_, &lookStream_.vt, szhd->index,
      &blockIndex, &outBuffer, &outBufferSize,
      &offset, &outSizeProcessed,
      &g_Alloc, &g_Alloc);
    if (res != SZ_OK) {
      SetErrorCode(res);;
      return NULL;
    }

    SetErrorCode(SZ_OK);
    void* result = nullptr;
    if (!(*buf)) {
      result = malloc(outSizeProcessed);
      if (!result) {
        SetErrorCode(SZ_ERROR_MEM);
        return NULL;
      }
      *buf = (char*)result;
    }
    memcpy(*buf, outBuffer + offset, outSizeProcessed);
    bufsize = outSizeProcessed;
    ISzAlloc_Free(&g_Alloc, outBuffer);
    szhd->data = result;
    return (void*)szhd;
  }

#ifdef _WIN32
  IStream* C7zRes::LoadResource(const char* id) {
    size_t res_size = 0;
    LoadResource(id, nullptr, res_size);
    if (!res_size)
      return nullptr;

    IStream* stream = nullptr;
    HGLOBAL buffer_handler = ::GlobalAlloc(GMEM_MOVEABLE, res_size);
    if (buffer_handler) {
      void* buffer = ::GlobalLock(buffer_handler);
      if (buffer) {
        void* res = LoadResource(id, (char**)&buffer, res_size);
        if (res) {
          ::CreateStreamOnHGlobal(buffer_handler, TRUE, &stream);
          FreeResource(res);
        }
        ::GlobalUnlock(buffer_handler);
      }
    }
    return stream;
  }

  IStream* C7zRes::LoadResource(const wchar_t* id) {
    size_t res_size = 0;
    LoadResource(id, nullptr, res_size);
    if (!res_size)
      return nullptr;

    IStream* stream = nullptr;
    HGLOBAL buffer_handler = ::GlobalAlloc(GMEM_MOVEABLE, res_size);
    if (buffer_handler) {
      void* buffer = ::GlobalLock(buffer_handler);
      if (buffer) {
        void* res = LoadResource(id, (char**)&buffer, res_size);
        if (res) {
          ::CreateStreamOnHGlobal(buffer_handler, TRUE, &stream);
          FreeResource(res);
        }
        ::GlobalUnlock(buffer_handler);
      }
    }
    return stream;
  }
#endif

  void C7zRes::FreeResource(void* res) {
    if (res) {
      SZRES_FILEHEADER* szhd = (SZRES_FILEHEADER*)res;
      if (szhd->data) {
        free(szhd->data);
        szhd->data = nullptr;
      }
    }
  }

  int C7zRes::GetErrorCode() {
    return errcode_;
  }

};

namespace JRES {

  const char* PASCAL GetVersion() {
    return PRODUCT_VERSION;
  }

  IRes* PASCAL Create7zRes(bool ignorecase) {
    return new SEVENZRES::C7zRes(ignorecase);
  }

};
