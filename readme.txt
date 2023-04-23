## Introduction

Simple markdown hoster.

Upload, displays as html

It has some cli helper

## compile cli

- dependencies:

+ libarchive
+ libuuid

```
gcc src/cli.c -o cli -larchive -luuid
```

## compile server

- dependencies:

+ libarchive
+ json-c (soon)
+ microhttpd

## Uploading

ls -d -1 sample_md/* | ./cli