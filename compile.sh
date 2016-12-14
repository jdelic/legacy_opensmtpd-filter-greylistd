#!/bin/bash
gcc \
    -I../s/opensmtpd-extras/api \
    -I../s/opensmtpd-extras/openbsd-compat/ \
    -I../s/opensmtpd-extras/ \
    -L../s/opensmtpd-extras/openbsd-compat/ \
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
    ../s/opensmtpd-extras/api/util.o \
    ../s/opensmtpd-extras/api/tree.o \
    ../s/opensmtpd-extras/api/iobuf.o \
    ../s/opensmtpd-extras/api/ioev.o \
    ../s/opensmtpd-extras/api/mproc.o \
    ../s/opensmtpd-extras/api/filter_api.o \
    ../s/opensmtpd-extras/api/log.o \
    ../s/opensmtpd-extras/api/dict.o \
    ../s/opensmtpd-extras/api/rfc2822.o \
    -lopenbsd-compat \
    -levent \
    -lssl \
    -lcrypto \
    filter_greylistd.c

