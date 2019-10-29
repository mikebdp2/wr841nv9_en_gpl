/*
 * wpa_supplicant/hostapd - Default include files
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This header file is included into all C files so that commonly used header
 * files can be selected with OS specific #ifdefs in one place instead of
 * having to have OS/C library specific selection in many files.
 */

#ifndef INCLUDES_H
#define INCLUDES_H

/* Include possible build time configuration before including anything else */
#include "build_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifndef _WIN32_WCE
#ifndef CONFIG_TI_COMPILER
#include <signal.h>
#include <sys/types.h>
#endif /* CONFIG_TI_COMPILER */
#include <errno.h>
#endif /* _WIN32_WCE */
#include <ctype.h>
#include <time.h>

#ifndef CONFIG_TI_COMPILER
#ifndef _MSC_VER
#include <unistd.h>
#endif /* _MSC_VER */
#endif /* CONFIG_TI_COMPILER */

#ifndef CONFIG_NATIVE_WINDOWS
#ifndef CONFIG_TI_COMPILER
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef __vxworks
#include <sys/uio.h>
#include <sys/time.h>
#endif /* __vxworks */
#endif /* CONFIG_TI_COMPILER */
#endif /* CONFIG_NATIVE_WINDOWS */

//////////////////////for wpa debug///////////////////////////////////
#define MAX_BUF_SIZE 4098

#ifndef __debug_location__
#define __s_debug1__(s)	#s
#define __s_debug2__(s)	__s_debug1__(s)
#define __s_debug3__		__s_debug2__(__LINE__)
#define __debug_location__		__FILE__ ":" __s_debug3__
#endif

#define WPA_DEBUG_FORMAT	"echo \"WPA_DEBUG(%s)====>%s\" > /dev/ttyS0 \r\n"

#define _WPA_DEBUG
#ifdef _WPA_DEBUG
#define debug_print(cmd,...)		WPA_DEBUG_PRINT(__debug_location__, cmd, ##__VA_ARGS__)
#else
#define debug_print(cmd,...)
#endif

//end of debug add.../////////////////////////////////////

#endif /* INCLUDES_H */
