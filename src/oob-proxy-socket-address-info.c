
#include "oob-proxy-socket-address-info.h"

char * shm_path()
{
	char * path = getenv("OOBPSAI_SHM_PATH");
	if(path == NULL) return DFLT_SHM_PATH;
	if(strlen(path) > SHM_PATH_MAX_LEN)
	{
		warnx("OOBPSAI_SHM_PATH length > %u", SHM_PATH_MAX_LEN);
		abort();
	}
	return path;
}

bool_t
extract_ip_port(struct sockaddr * socket_addr, char * out_ipaddr, uint16_t * out_port)
{
	in_port_t * port_p;
	void * addr_p;
	
	if(socket_addr->sa_family == AF_INET)
	{
		port_p = &((struct sockaddr_in*)socket_addr)->sin_port;
		addr_p = &((struct sockaddr_in*)socket_addr)->sin_addr;
	}
	else if(socket_addr->sa_family == AF_INET6)
	{
		port_p = &((struct sockaddr_in6*)socket_addr)->sin6_port;
		addr_p = &((struct sockaddr_in6*)socket_addr)->sin6_addr;
	}
	else return FALSE;
	
	if(out_port) *out_port = ntohs(*port_p);
	if(!out_ipaddr) return TRUE;
	if(inet_ntop(socket_addr->sa_family, addr_p, out_ipaddr, IPADDR_MAX_LEN) == NULL) return FALSE;
	return TRUE;
}

bool_t
save_socket_address(struct sockaddr * socket_addr, char * dest_str)
{
	uint16_t port;
	char ipaddr[IPADDR_MAX_LEN+1];
	
	if(! extract_ip_port(socket_addr, (char*)&ipaddr, &port)) return FALSE;
	snprintf(dest_str, SOCKET_ADDR_MAX_LEN+1, "%s,%u", ipaddr, port);
	return TRUE;
}

bool_t
set_proxy_address_info(struct sockaddr * socket_addr, char * target_str)
{
	char sock_addr_str[SOCKET_ADDR_MAX_LEN+1];
	char linkname_buf[PATH_MAX_LEN+1];
	int symlink_ret;
	
	if(! save_socket_address(socket_addr, sock_addr_str)) return FALSE;
	snprintf(linkname_buf, PATH_MAX_LEN+1, "%s/%s", shm_path(), sock_addr_str);
	
	unlink(linkname_buf);
	symlink_ret = symlink(target_str, linkname_buf);
	if(symlink_ret == -1) warn("symlink: %s", linkname_buf);
	
	return symlink_ret == 0 ? TRUE : FALSE;
}

void
unset_proxy_address_info(char * ip_port)
{
	char linkname_buf[PATH_MAX_LEN+1];
	snprintf(linkname_buf, PATH_MAX_LEN+1, "%s/%s", shm_path(), ip_port);
	unlink(linkname_buf);
}
