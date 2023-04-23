## Introduction

Simple markdown hoster.

Upload, displays as html

It has some cli helper

## compile cli

- dependencies:

+ libarchive
+ libuuid

```
gcc cmd/cli.c -o bin/cli -larchive -luuid
```

## compile server

- dependencies:

+ libarchive
+ libuuid
+ json-c (soon)
+ microhttpd

```
gcc cmd/server.c -o bin/server -larchive -luuid -lmicrohttpd
```

## Uploading

ls -d -1 sample_md/* | ./cli