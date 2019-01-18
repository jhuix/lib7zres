rem 非固实、LZMA2、1M字典大小、64单词大小快速压缩模式
7z.exe a -t7Z -mx=2 -ms=0m -m0=LZMA2:d=20:fb=64 -mmt %~1.7z %~2