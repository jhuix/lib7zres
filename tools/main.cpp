// 7zres�������̨Ӧ�ó������ڵ㡣

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
#ifdef SZRES_CRYPT
  printf("\n");
  printf("7zRes ʹ�ð�����\n");
  printf("�ӿڣ�����RAR�ļ���ֻ֧����������ܵ�7Z�ļ���\n");
  printf("-e srcfile destfile\n");
  printf("srcfile��  �������ļ����ɰ���·����\n");
  printf("destfile�� ���ܺ��ļ����ɰ���·����\n");
  printf("\n");
  printf("�ӿڣ�����7Z�ļ���ֻ֧�ָù��߼��ܵ�7Z�ļ���\n");
  printf("-d srcfile destfile\n");
  printf("srcfile��  �������ļ����ɰ���·����\n");
  printf("destfile�� ���ܺ��ļ����ɰ���·����\n");
#endif
  printf("\n");
  printf("�ӿڣ���ѹ7Z�ļ��еĵ����ļ���֧�ָù��߼��ܺ�δ����7Z�ļ���\n");
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
#ifdef SZRES_CRYPT
    if (_stricmp(param, "-e") == 0) {
      if (argc > 3) {
        if (SEVENZRES::Encode(argv[2], argv[3])) {
          printf("7zRes: �ɹ������ļ�\"");
          printf(argv[2]);
          printf("\"��Ŀ���ļ�\"");
          printf(argv[3]);
          printf("\"��\n");
          return 0;
        }
      }
    }
    else if (_stricmp(param, "-d") == 0) {
      if (argc > 3) {
        if (SEVENZRES::Decode(argv[2], argv[3])) {
          printf("7zRes: �ɹ������ļ�\"");
          printf(argv[2]);
          printf("\"��Ŀ���ļ�\"");
          printf(argv[3]);
          printf("\"��\n");
          return 0;
        }
      }
    }
#endif
    if (_stricmp(param, ("-x")) == 0) {
      if (argc > 4) {
        if (SEVENZRES::ExtractFileA(argv[2], argv[3], argv[4])) {
          printf(("7zRes: �ɹ���ѹ�ļ�\""));
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

