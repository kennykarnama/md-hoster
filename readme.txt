## Introduction

Simple markdown hoster.

Upload, displays as html

This is still WIP.

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

## Server

```
./bin/server
```

It will serve HTTP on PORT 8080

To terminate, simply type any character then press enter.