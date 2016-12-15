/*
 * Copyright (c) 2013 Eric Faurot <eric@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>
#include <grp.h>

#include "smtpd-defines.h"
#include "smtpd-api.h"
#include "log.h"

#define GREY_OK 0
#define GREY_HOLD 1
#define GREY_ERROR 2
#define GREY_DENY 3


char *greylistd_socket_path = "/var/run/greylistd/socket";


static int check_greylist(const char *ip_addr) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    addr.sun_family = AF_UNIX;
    snprintf((char *)&addr.sun_path, 108, "%s", greylistd_socket_path);

    if (connect(sock, (const struct sockaddr *)&addr,
                sizeof(struct sockaddr_un)) == -1) {
        log_warn("filter_greylistd: can't connect to server %s uid:%i gid:%i errno:%i", addr.sun_path, geteuid(), getegid(), errno);
        return -1;
    }


    char buf[7 + INET6_ADDRSTRLEN]; /* max length of "update [ipv6 addr]" */
    memset(&buf, 0, 7 + INET6_ADDRSTRLEN);
    snprintf((char *)&buf, 7 + INET6_ADDRSTRLEN, "update %s", ip_addr);

    if (write(sock, &buf, strlen((const char *)&buf)) == -1) {
        log_warn("filter_greylistd: write error");
        close(sock);
        return GREY_ERROR;
    }

    char reply[32];
    memset(&reply, 0, 32);
    if (read(sock, &reply, 31) == -1) {
        log_warn("filter_greylistd: read error");
        close(sock);
        return GREY_ERROR;
    }

    close(sock);

    if (strncmp((const char *)&reply, "grey", 4) == 0) return GREY_HOLD;
    if (strncmp((const char *)&reply, "black", 5) == 0) return GREY_DENY;
    if (strncmp((const char *)&reply, "white", 5) == 0) return GREY_OK;

    log_warn("unknown greylisting status: %s", (char *)&reply);
    return GREY_ERROR;
}


static int on_connect(uint64_t id, struct filter_connect *conn) {
    log_debug("debug: on_connect");

    char ip_str[INET6_ADDRSTRLEN];
    memset(&ip_str, 0, INET6_ADDRSTRLEN);

    /* get either an ipv4 or ipv6 address */
    if (conn->remote.ss_family == AF_INET6) {
        struct sockaddr_in6 *ip6 = (struct sockaddr_in6 *)&conn->remote;
        if (inet_ntop(AF_INET6, (const void *)&ip6->sin6_addr.s6_addr,
                      (char *)&ip_str,
                      INET6_ADDRSTRLEN) == NULL) {
            log_warn("can't convert assumed ipv6 address to string %i", errno);
            return filter_api_reject_code(id, FILTER_CLOSE, 521,
                                          "unparsable ipv6 address");
        }
    }
    else if (conn->remote.ss_family == AF_INET) {
        struct sockaddr_in *ip4 = (struct sockaddr_in *)&conn->remote;
        if (inet_ntop(AF_INET, (const void *)&ip4->sin_addr.s_addr,
                      (char *)&ip_str,
                      INET_ADDRSTRLEN) == NULL) {
            log_warn("can't convert assumed ipv4 address to string %i", errno);
            return filter_api_reject_code(id, FILTER_CLOSE, 521,
                                          "unparsable ipv4 address");
        }
    }
    else {
        log_warn("filter called with unknown protocol family %i",
                 conn->remote.ss_family);
        return filter_api_reject_code(id, FILTER_CLOSE, 521,
                                      "unknown address family");
    }

    /* now we should have a string representation of the remote IP in
     * ip_str
     */
    int action = check_greylist((const char *)&ip_str);

    switch (action) {
        case GREY_OK:
            return filter_api_accept(id);
        break;
        case GREY_HOLD:
            return filter_api_reject_code(id, FILTER_CLOSE, 450,
                                          "%s greylisted. Try again later.");
        break;
        case GREY_DENY:
            return filter_api_reject_code(id, FILTER_CLOSE, 550,
                                          "%s blacklisted. Transmission denied.");
        break;
    }

    /* if action == GREY_ERROR we land here, do a temporary reject for
     * a redelivery
     */
    log_warn("greylisting returned an error (GREY_ERROR), the server might be "
             "unavailable. We temporarily reject email and hope for better "
             "times.");
    return filter_api_reject_code(
                id, FILTER_CLOSE, 451,
                "there seems to be a technical problem on our end. "
                "Please try again."
           );
}


int main(int argc, char **argv) {
    int ch, d = 0, v = 0;

    log_init(1);

    while ((ch = getopt(argc, argv, "dvs:")) != -1) {
        switch (ch) {
            case 'd':
                d = 1;
                break;
            case 'v':
                v |= TRACE_DEBUG;
                break;
            case 's':
                greylistd_socket_path = optarg;
                break;
            default:
                log_warnx("warn: bad option");
                return 1;
                /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    log_init(d);
    log_verbose(v);

    log_debug("debug: starting...");

    filter_api_on_connect(on_connect);
    filter_api_no_chroot();
    filter_api_loop();
    log_debug("debug: exiting");

    return 1;
}
