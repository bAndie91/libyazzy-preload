
#include <dlfcn.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>


#ifdef DEBUG
#undef DEBUG
#define DEBUG(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#else
#define DEBUG(...) 0
#endif

typedef int bool_t;
#define TRUE 1
#define FALSE 0


#define DFLT_SHM_PATH "/run/shm/oobpsai"
#define SHM_PATH_MAX_LEN 64
#define IPADDR_MAX_LEN INET6_ADDRSTRLEN
#define PORTNUM_MAX_LEN 5
#define SOCKET_ADDR_MAX_LEN (IPADDR_MAX_LEN + 1 + PORTNUM_MAX_LEN)
#define PATH_MAX_LEN (SHM_PATH_MAX_LEN + 1 + SOCKET_ADDR_MAX_LEN)


char * shm_path();

bool_t
set_proxy_address_info(struct sockaddr * socket_addr, char * target_str);

void
unset_proxy_address_info(char * ip_port);

bool_t
save_socket_address(struct sockaddr * socket_addr, char * dest_str);

bool_t
extract_ip_port(struct sockaddr * socket_addr, char * out_ipaddr, uint16_t * out_port);

