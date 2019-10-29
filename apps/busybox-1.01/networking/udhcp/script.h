#ifndef _SCRIPT_H
#define _SCRIPT_H
#include "msgq.h"
#include "packet.h"

void run_script(struct dhcpMessage *packet, const char *name);
void dhcpc_ipc_send(int dstMid, int msgType, UINT8* ptr, UINT32 nbytes);
void dhcpc_ipc_fork(void);
void dhcps_ipc_fork(void);

#if MODULE_DHCP_REQUEST_SAME_IP/*  by huangwenzhong, 22Apr13 */
unsigned long dhcpc_get_last_ip(void);
#endif

#endif
