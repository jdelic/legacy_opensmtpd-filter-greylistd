#!/bin/bash

if [ ! -d opensmtpd-extras ] || [ ! -x opensmtpd-extras/bootstrap ]; then
    git submodule update --init
fi

if [ ! -x opensmtpd-extras/configure ]; then
    cd opensmtpd-extras
    ./bootstrap
    cd ..
fi

if [ ! -f opensmtpd-extras/config.h ]; then
    cd opensmtpd-extras
    ./configure --with-filter-stub --with-table-stub --with-scheduler-stub \
        --with-queue-stub
    cd ..
fi

if [ ! -f opensmtpd-extras/api/filter_api.o ]; then
    cd opensmtpd-extras
    make
    cd ..
fi

gcc \
    -Iopensmtpd-extras/api \
    -Iopensmtpd-extras/openbsd-compat/ \
    -Iopensmtpd-extras/ \
    -Lopensmtpd-extras/openbsd-compat/ \
    -o filter-greylistd \
    -DHAVE_CONFIG_H \
    -D_FORTIFY_SOURCE=2 \
    -g \
    -O2 \
    -fstack-protector-strong \
    -Wformat \
    -Werror=format-security \
    -fPIC \
    -DPIC \
    -Wall \
    -Wpointer-arith \
    -Wuninitialized \
    -Wsign-compare \
    -Wformat-security \
    -Wsizeof-pointer-memaccess \
    -Wno-pointer-sign \
    -Wno-unused-result \
    -fno-strict-aliasing \
    -fno-builtin-memset \
    -DBUILD_FILTER \
    opensmtpd-extras/api/util.o \
    opensmtpd-extras/api/tree.o \
    opensmtpd-extras/api/iobuf.o \
    opensmtpd-extras/api/ioev.o \
    opensmtpd-extras/api/mproc.o \
    opensmtpd-extras/api/filter_api.o \
    opensmtpd-extras/api/log.o \
    opensmtpd-extras/api/dict.o \
    opensmtpd-extras/api/rfc2822.o \
    -lopenbsd-compat \
    -levent \
    -lssl \
    -lcrypto \
    filter_greylistd.c

