/***************************************************************
 *
 * Copyright(c) 2005-2008 Shenzhen TP-Link Technologies Co. Ltd.
 * All right reserved.
 *
 * Filename		:	msgq.h
 * Version		:	1.0
 * Abstract		:	
 * Author		:	LI SHAOZHANG (lishaozhang@tp-link.net)
 * Created Date	:	19Jul08
 *
 * Modified History:
 ***************************************************************/

#ifndef _MSG_Q_H_
#define _MSG_Q_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


typedef unsigned char UINT8;
typedef unsigned int UINT32;
typedef int BOOL;

#define TP_IPC_MAGIC		0xbabeface
#define TP_IPC_VERSION		0x10		/* 1.0 */
#define MSG_Q_KEY			0x3F
#define MSG_DHCPC_KEY		MSG_Q_KEY
#define MSG_DHCPV6C_KEY 	(MSG_Q_KEY + 1)
#define MSG_DHCPS_KEY		(0x4F)
#define MSG_MAX_LEN			(2*1024 + 32)  /*64条路由条目和信息头的长度*/
#define IPC_MAX			32

#define IDLE_Q_KEY		0xFF
#define IDLE_Q_ID		(-1)

typedef struct tp_ipc_msg_struct
{
	UINT32 magic;		/* Must be 0xbabeface */
	UINT32 version;		/* Header version */
	UINT32 dstMid;		/* ID of receiver module */
	UINT32 srcMid;		/* ID of sender module */
	UINT32 msgType;		/* data type of the IPC msg */
	BOOL   bFrag;		/* There is fragment data in next msg */
	UINT8  payload[0];	/* real data */
}tp_ipc_msg;

typedef enum 
{
	IPC_ANNOUNCEMENT,
	IPC_ECHO_REQUEST,
	IPC_ECHO_REPLY,
	IPC_DATA_SET,
	IPC_DATA_GET,
	IPC_TYPE_MAX
}tp_ipc_msg_type;
 

typedef enum
{
	IDLE = 0,
	HTTPD = 1,
	PPPD = 2,
	PPPOE = 3,
	L2TP = 4,
	PPTP = 5,
	DHCPC = 6,
	DHCPS = 7,
	ADVSEC = 8,
	NTP = 9,
	DNS_DETECT = 10,
	SYS_STATS = 11,
	/*added by by ZQQ, Add the Processes, 21Mar2011 */
	DHCP6C = 12,
	DHCP6S = 13,
	RADVD =	14,
	/*end added by ZQQ*/
	MODULE_ID_MAX = 0x100
}tp_ipc_module_id;

typedef int (*ipc_proc_t)(tp_ipc_msg*);

typedef struct IPC_MODULE
{
	tp_ipc_module_id 	id;
	key_t 			key;
	int				qid;
	ipc_proc_t          	fn;			
}ipc_modules;

#ifndef TRUE
#define TRUE 	(1)
#endif

#ifndef FALSE
#define FALSE	(0)
#endif

#define IPC_DEBUG
#ifdef  IPC_DEBUG
	#define ipc_msg(...) printf("%s %s %d : ",__FILE__,__FUNCTION__,__LINE__); \
		printf(__VA_ARGS__); \
		printf("\r\n")
#else
#define ipc_msg(...)
#endif

int msgq_ipc_send(int qid, int srcMid, int dstMid, int msgType, UINT8* ptr, UINT32 nbytes);
int msgq_ipc_rcv(int qid, int mid, UINT8* rcvBuf, int maxSize);

#endif

