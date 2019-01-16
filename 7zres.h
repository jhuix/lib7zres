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

#ifndef _RARRES_INCLUDE_
#define _RARRES_INCLUDE_

#include "lib7zres.h"
#include <map>

namespace SEVENZRES {

#if !defined(DISALLOW_COPY_AND_ASSIGN)

  // A macro to disallow the copy constructor and operator= functions
  // This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(T) \
  T(const T&);                      \
  void operator=(const T&)

#endif  // !DISALLOW_COPY_AND_ASSIGN
  
  struct SZRES_FILEHEADER {
    UInt32 index;
    UInt64 filesize;
#ifdef _WIN32
    UInt32 attrib;
    FILETIME mtime;
    FILETIME ctime;
#endif
    void* data;
    std::wstring filename;
  };

class C7zRes : public JRES::IRes {
  public:
    explicit C7zRes(bool ignorecase = true);
    ~C7zRes();

    virtual void Release();
    virtual bool Open(const char* filename, char path_sep);
    virtual bool Open(const wchar_t* filename, wchar_t path_sep);
    virtual void* LoadResource(const char* id, char** buf, size_t& bufsize);
    virtual void* LoadResource(const wchar_t* id, char** buf, size_t& bufsize);
    virtual void FreeResource(void* res);
    virtual int GetErrorCode();
    virtual void Close();
#ifdef _WIN32
    virtual IStream* LoadResource(const char* id);
    virtual IStream* LoadResource(const wchar_t* id);
#endif

  protected:
    void SetErrorCode(int errcode) {
      errcode_ = errcode;
    }
    bool ListFiles(wchar_t path_sep);
    void* Extract(SZRES_FILEHEADER* szhd, char** buf, size_t& bufsize);

    CFileInStream archiveStream_;
    CLookToRead2 lookStream_;
    CSzArEx db_;
    SRes errcode_;
    std::map<std::string,  SZRES_FILEHEADER* > fileheadersA_;
    std::map<std::wstring, SZRES_FILEHEADER* > fileheadersW_;

  private:
    long long total_packsize_, total_unpsize_;
    bool ignorecase_;
    DISALLOW_COPY_AND_ASSIGN(C7zRes);
  };

};

#endif  //_RARRES_INCLUDE_
