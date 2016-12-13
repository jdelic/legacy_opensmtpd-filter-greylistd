
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define GREYLISTD_SOCKET "/var/run/greylistd/socket"

struct sockaddr_un *addr;

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("must provide socket path as argument\n");
        return 1;
    }

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("filter_greylistd: connection error");
        exit(EXIT_FAILURE);
    }

    addr = calloc(sizeof(struct sockaddr_un), 1);
    if (addr == NULL) {
        perror("filter_greylistd: memory allocation failed");
        exit(EXIT_FAILURE);
    }
    addr->sun_family = AF_UNIX;
    snprintf(addr->sun_path, 108, "%s", argv[1]);

    if (connect(sock, (const struct sockaddr *)addr,
                sizeof(struct sockaddr_un)) == -1) {
        fprintf(stderr, "filter_greylistd: can't connect to server\n");
        exit(EXIT_FAILURE);
    }

    char *cmd = "update mkmgmbh.com";
    if (write(sock, cmd, strlen(cmd)) == -1) {
        perror("filter_greylistd: write error");
        exit(EXIT_FAILURE);
    }

    char reply[32];
    memset(&reply, 0, 32);
    if (read(sock, &reply, 31) == -1) {
        perror("filter_greylistd: read error");
        exit(EXIT_FAILURE);
    }

    printf("response: %s\n", &reply);

    close(sock);
    free(addr);
}
