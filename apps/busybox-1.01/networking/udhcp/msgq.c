/******************************************************************************
*
* Copyright (c) 2010 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* FILE NAME  :   msgq.c
* VERSION    :   1.0
* DESCRIPTION:   处理与webserver等其他进程之间进行通信的接口.
*
* AUTHOR     :   huangwenzhong <huangwenzhong@tp-link.net>
* CREATE DATE:   04/02/2013
*
* HISTORY    :
* 01   04/02/2013  huangwenzhong     Create.
*
******************************************************************************/
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "msgq.h"

int msgq_ipc_rcv(int qid, int mid, UINT8* rcvBuf, int maxSize)
{
	int nReads= 0;
	tp_ipc_msg* pMsg = NULL;
	UINT32* pType = NULL;

	if ((qid < 0) || (mid <= 0) || (rcvBuf == NULL) || (maxSize <= 0))
	{
		return -1;
	}

	memset(rcvBuf, 0, maxSize);
	nReads = msgrcv(qid, rcvBuf, MSG_MAX_LEN, mid, IPC_NOWAIT);

	if (nReads < 0)
	{
		return -1;
	}

	pMsg = (tp_ipc_msg*)(rcvBuf + 4);
	
	/* check the type */
	pType = (UINT32 *)rcvBuf;
	if (*pType != mid || pMsg->dstMid != mid)
	{
		return -1;		
	}

	if (pMsg->magic != TP_IPC_MAGIC)
	{
		return -3;
	}

	if (pMsg->version != TP_IPC_VERSION)
	{
		return -4;
	}

	return nReads;
}

/******************************************************************************
* FUNCTION      :	httpd_ipc_send()
* AUTHOR        :	zhouqiqiu <zhouqiqiu@tp-link.net>
* DESCRIPTION   :	send msg to process 
* INPUT         : 	int dstMid: the destination id of precess to send
					int msgType: msg type
					UINT8* ptr: the msg content to send
					UINT32 nbytes: the msg length to send
*
* OUTPUT        : 	N/A
* RETURN        : 	< 0 : fail, 0 : succeed
* OTHERS        :
******************************************************************************/
int msgq_ipc_send(int qid, int srcMid, int dstMid, int msgType, UINT8* ptr, UINT32 nbytes)
{
	UINT8 sendBuf[MSG_MAX_LEN + 4];		/* 4 bytes long: msg type */
	tp_ipc_msg* pMsg = (tp_ipc_msg*)(sendBuf + 4);
	int i = 0;

	if ((qid < 0) || (ptr == NULL))
	{
		return -1;
	}
	memset(sendBuf, 0, MSG_MAX_LEN + 4);
	*((UINT32*)sendBuf) = dstMid;		/* msg type:unix ipc method requested */
	pMsg->magic = TP_IPC_MAGIC;
	pMsg->version = TP_IPC_VERSION;
	pMsg->dstMid = dstMid;
	pMsg->srcMid = srcMid;
	pMsg->msgType = msgType;
	pMsg->bFrag = FALSE;				/* httpd_ipc_send not support fragment now */

	if (nbytes > MSG_MAX_LEN - sizeof(tp_ipc_msg))
	{
		return -1;
	}
	memcpy(pMsg->payload, ptr, nbytes);

	if (msgsnd(qid, sendBuf, nbytes + sizeof(tp_ipc_msg), IPC_NOWAIT) < 0)
	{
		return -2;
	}

	return 0;
}

