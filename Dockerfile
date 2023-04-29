FROM docker.io/gcc:12.2.0-bullseye

WORKDIR /app

RUN apt-get update && apt-get install ca-certificates cmake -y

COPY . .

RUN wget https://github.com/libarchive/libarchive/releases/download/v3.6.2/libarchive-3.6.2.tar.gz && tar xzvf libarchive-3.6.2.tar.gz

RUN cd ./libarchive-3.6.2 && ./configure && make && make install

RUN wget https://onboardcloud.dl.sourceforge.net/project/libuuid/libuuid-1.0.3.tar.gz && tar xzvf libuuid-1.0.3.tar.gz

RUN cd ./libuuid-1.0.3 && ./configure && make && make install

RUN wget https://mirror.freedif.org/GNU/libmicrohttpd/libmicrohttpd-0.9.76.tar.gz && tar xzvf libmicrohttpd-0.9.76.tar.gz

RUN cd ./libmicrohttpd-0.9.76 && ./configure && make && make install

RUN wget https://github.com/mity/md4c/archive/refs/tags/release-0.4.8.tar.gz && tar xzvf release-0.4.8.tar.gz

RUN cd ./md4c-release-0.4.8 && mkdir build && cd build && cmake .. && make && make install

RUN mkdir bin && gcc cmd/server.c -o bin/server -larchive -luuid -lmicrohttpd -lmd4c-html

ENV LD_LIBRARY_PATH=/usr/local/lib64

RUN ldconfig

CMD [ "./bin/server" ]
