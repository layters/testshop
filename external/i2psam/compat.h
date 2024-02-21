/**
 * Copyright (c) 2009-2010 Satoshi Nakamoto
 * Copyright (c) 2009-2014 The Bitcoin developers
 * Copyright (c) 2013-2015 The Anoncoin Core developers
 *
 * Distributed under the MIT software license, see the accompanying
 * file COPYING or http://www.opensource.org/licenses/mit-license.php.
 *
 * See full documentation about SAM at http://www.i2p2.i2p/samv3.html
 */

#ifndef I2PSAM_COMPAT_H
#define I2PSAM_COMPAT_H

#ifdef WIN32

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif // _WIN32_WINNT

#define _WIN32_WINNT 0x0501
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif // WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#ifdef FD_SETSIZE
#undef FD_SETSIZE // prevent redefinition compiler warning
#endif // FD_SETSIZE

#define FD_SETSIZE 1024 // max number of fds in fd_set

#include <winsock2.h> // Must be included before mswsock.h and windows.h
#include <mswsock.h>
#include <windows.h>
#include <ws2tcpip.h>

#else // WIN32

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <limits.h>
#include <netdb.h>
#include <unistd.h>
#endif // WIN32

#ifdef WIN32
#define MSG_DONTWAIT 0
#else // WIN32
typedef u_int SOCKET;
#include "errno.h"
#define WSAGetLastError() errno
#define WSAEINVAL EINVAL
#define WSAEALREADY EALREADY
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEMSGSIZE EMSGSIZE
#define WSAEINTR EINTR
#define WSAEINPROGRESS EINPROGRESS
#define WSAEADDRINUSE EADDRINUSE
#define WSAENOTSOCK EBADF
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR -1
#endif // WIN32

#ifdef WIN32
#ifndef S_IRUSR
#define S_IRUSR 0400
#define S_IWUSR 0200
#endif // S_IRUSR
#else // WIN32
#define MAX_PATH 1024
#endif // WIN32

// As Solaris does not have the MSG_NOSIGNAL flag for send(2) syscall, it is defined as 0
#if !defined(HAVE_MSG_NOSIGNAL) && !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif

#ifndef WIN32

// PRIO_MAX is not defined on Solaris
#ifndef PRIO_MAX
#define PRIO_MAX 20
#endif // PRIO_MAX

#define THREAD_PRIORITY_LOWEST PRIO_MAX
#define THREAD_PRIORITY_BELOW_NORMAL 2
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_ABOVE_NORMAL (-2)
#endif // WIN32

size_t strnlen_int(const char *start, size_t max_len);

#endif // I2PSAM_COMPAT_H
