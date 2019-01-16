// rarres.cpp: �������̨Ӧ�ó������ڵ㡣
//
#ifdef _WIN32
#ifndef STRICT
#define STRICT 1
#endif
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <wchar.h>
#include <wctype.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "lib7zres.h"
#include "librespak.h"

void print_help()
{
//#ifdef RARRES_CRYPT
//  printf("\n");
//  printf("RarRes ʹ�ð�����\n");
//  printf("�ӿڣ�����RAR�ļ���ֻ֧����������ܵ�RAR�ļ���\n");
//  printf("-e srcfile destfile\n");
//  printf("srcfile��  �������ļ����ɰ���·����\n");
//  printf("destfile�� ���ܺ��ļ����ɰ���·����\n");
//  printf("\n");
//  printf("�ӿڣ�����RAR�ļ���ֻ֧�ָù��߼��ܵ�RAR�ļ���\n");
//  printf("-d srcfile destfile\n");
//  printf("srcfile��  �������ļ����ɰ���·����\n");
//  printf("destfile�� ���ܺ��ļ����ɰ���·����\n");
//#endif
  printf("\n");
  printf("�ӿڣ���ѹRAR�ļ��еĵ����ļ���֧�ָù��߼��ܺ�δ����RAR�ļ���\n");
  printf("-x srcfile file destfile\n");
  printf("srcfile��  ����ѹ�ļ����ɰ���·����\n");
  printf("file��     ����ѹ�ļ��е��ļ����������ļ����·����\n");
  printf("destfile�� ��ѹ���ļ����ɰ���·����\n");
  printf("\n");
}

int main(int argc, char *argv[]) {
  //Default print help
  if (argc > 1) {
    char* param = argv[1];
//#ifdef RARRES_CRYPT
//    if (_stricmp(param, "-e") == 0) {
//      if (argc > 3) {
//        if (RARRES::Encode(argv[2], argv[3])) {
//          printf("\nRarRes: �ɹ������ļ�\"");
//          printf(argv[2]);
//          printf("\"��Ŀ���ļ�\"");
//          printf(argv[3]);
//          printf("\"��\n");
//          return 0;
//        }
//      }
//    }
//    else if (_stricmp(param, "-d") == 0) {
//      if (argc > 3) {
//        if (RARRES::Decode(argv[2], argv[3])) {
//          printf("\nRarRes: �ɹ������ļ�\"");
//          printf(argv[2]);
//          printf("\"��Ŀ���ļ�\"");
//          printf(argv[3]);
//          printf("\"��\n");
//          return 0;
//        }
//      }
//    }
//#endif
    if (_stricmp(param, ("-x")) == 0) {
      if (argc > 4) {
        if (SEVENZRES::ExtractFileA(argv[2], argv[3], argv[4])) {
          printf(("\nRarRes: �ɹ���ѹ�ļ�\""));
          printf(argv[3]);
          printf(("\"��Ŀ���ļ�\""));
          printf(argv[4]);
          printf(("\"��\n"));
          return 0;
        }
      }
    }
  }

  print_help();
  return -1;
}

