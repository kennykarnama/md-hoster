## Introduction

Simple markdown hoster.

Upload tar.gz containing your markdown files and assets. Displays as html

This is only my learning projects to learn more about C :D

This is still WIP.

Feel free to use and modify.

Cheers :D

## compile

- dependencies

+ https://www.libarchive.org/
+ https://sourceforge.net/projects/libuuid/
+ https://www.gnu.org/software/libmicrohttpd/
+ https://github.com/mity/md4c

```
gcc cmd/cli.c -o bin/cli -larchive -luuid
```

## compile server

```
gcc cmd/server.c -o bin/server -larchive -luuid -lmicrohttpd -lmd4c-html
```

## Uploading

ls -d -1 sample_md/* | ./cli

## Server

```
./bin/server
```

It will serve HTTP on PORT 8080

To terminate, simply type any character then press enter.

Note:

If you have successfully compiled server.c but when running you encounter message like this:

```
./bin/server: error while loading shared libraries: libmd4c-html.so.0: cannot open shared object file: No such file or directory
```

The workaround I use is to specify LD_LIBRARY_PATH to /usr/local/lib64:

```
[kenny_void@(none) md-hoster]$ LD_LIBRARY_PATH=/usr/local/lib64
[kenny_void@(none) md-hoster]$ export LD_LIBRARY_PATH
[kenny_void@(none) md-hoster]$ sudo ldconfig
```

## Valgrind

Version: 3.20.0

```
valgrind --leak-check=yes --track-origins=yes ./bin/server
```

Latest report

```
==30205== 
==30205== HEAP SUMMARY:
==30205==     in use at exit: 0 bytes in 0 blocks
==30205==   total heap usage: 1,018 allocs, 1,018 frees, 844,804 bytes allocated
==30205== 
==30205== All heap blocks were freed -- no leaks are possible
==30205== 
==30205== For lists of detected and suppressed errors, rerun with: -s
==30205== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```