/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Windows shim: AF_UNIX sockets do not exist on older Windows releases.
 */
#ifndef __WINGSUP_SYS_UN_H
#define __WINGSUP_SYS_UN_H

#include <sys/socket.h>

struct sockaddr_un {
	unsigned short sun_family;
	char sun_path[108];
};

#endif /* __WINGSUP_SYS_UN_H */
