#!/bin/bash
gcc \
    -I../opensmtpd-extras-5.7.1/api \
    -I../opensmtpd-extras-5.7.1/openbsd-compat/ \
    -I../opensmtpd-extras-5.7.1/ \
    -L../opensmtpd-extras-5.7.1/openbsd-compat/ \
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
    ../opensmtpd-extras-5.7.1/api/util.o \
    ../opensmtpd-extras-5.7.1/api/tree.o \
    ../opensmtpd-extras-5.7.1/api/iobuf.o \
    ../opensmtpd-extras-5.7.1/api/ioev.o \
    ../opensmtpd-extras-5.7.1/api/mproc.o \
    ../opensmtpd-extras-5.7.1/api/filter_api.o \
    ../opensmtpd-extras-5.7.1/api/log.o \
    ../opensmtpd-extras-5.7.1/api/dict.o \
    -lopenbsd-compat \
    -levent \
    -lssl \
    -lcrypto \
    filter_greylistd.c

