Lib7zres
=============

lib7zres 工程是一读取由7z压缩文件做为资源包的库。

## 7z-LZMA SDK 

7z文件分析和解压缩采用 [7Zip](https://www.7-zip.org) 的解压缩[LZMA SDK](https://www.7-zip.org/sdk.html)开源版本源码。

## Example

#### 1. 获取一个test.7z资源包里的名为"skin\\ui\\index.html"的内容至缓存中:

```
JRES::IRes* jres = SEVENZRES::Create7zRes(true);
if (jres) {
    if (jres->Open("test.7z", '\\')) {
        char* buf = nullptr;
        size_t bufsize = 0;
        void* res = jres->LoadResource("skin\\ui\\index.html", &buf, bufsize);
        if (res) {
            //TO DO USE data of "buf"
            rarres->FreeResource(res);
        }
    }
    jres->Release();
}
```

#### 2. 在windows平台下，获取一个test.7z"skin/img/caotion.png"的内容至IStream中:

```
JRES::IRes* jres = SEVENZRES::Create7zRes(true);
if (jres) {
    if (jres->Open("test.7z", 0)) {
        IStream* res = jres->LoadResource("skin/img/caotion.png");
        if (res) {
            //TO DO USE IStream interface methods of "res";
            //For examples: <Seek>|<Read>|<Write> methods etc.
            res->Release();
        }
    }
    jres->Release();
}
```

## License

[MIT](http://opensource.org/licenses/MIT)

Copyright (c) 2018-present, [Jhuix](mailto:jhuix0117@gmail.com) (Hui Jin)