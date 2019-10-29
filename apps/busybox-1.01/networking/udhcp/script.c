/* script.c
 *
 * Functions to call the DHCP client notification scripts
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include "script.h"
#include "options.h"
#include "dhcpd.h"
#include "dhcpc.h"
#include "common.h"
#include "libmsglog.h"

#include "signalpipe.h"

#if MODULE_SMART_DMZ/*  by huangwenzhong, 27Aug12 */
#include "static_leases.h"
#endif
#include "msgq.h"

/* get a rough idea of how long an option will be (rounding up...) */
static const int max_option_length[] = {
	[OPTION_IP] =		sizeof("255.255.255.255 "),
	[OPTION_IP_PAIR] =	sizeof("255.255.255.255 ") * 2,
	[OPTION_STRING] =	1,
	[OPTION_BOOLEAN] =	sizeof("yes "),
	[OPTION_U8] =		sizeof("255 "),
	[OPTION_U16] =		sizeof("65535 "),
	[OPTION_S16] =		sizeof("-32768 "),
	[OPTION_U32] =		sizeof("4294967295 "),
	[OPTION_S32] =		sizeof("-2147483684 "),
};


static inline int upper_length(int length, int opt_index)
{
	return max_option_length[opt_index] *
		(length / option_lengths[opt_index]);
}


static int sprintip(char *dest, char *pre, uint8_t *ip)
{
	return sprintf(dest, "%s%d.%d.%d.%d", pre, ip[0], ip[1], ip[2], ip[3]);
}


/* really simple implementation, just count the bits */
static int mton(struct in_addr *mask)
{
	int i;
	unsigned long bits = ntohl(mask->s_addr);
	/* too bad one can't check the carry bit, etc in c bit
	 * shifting */
	for (i = 0; i < 32 && !((bits >> i) & 1); i++);
	return 32 - i;
}


/* Fill dest with the text of option 'option'. */
static void fill_options(char *dest, uint8_t *option, struct dhcp_option *type_p)
{
	int type, optlen;
	uint16_t val_u16;
	int16_t val_s16;
	uint32_t val_u32;
	int32_t val_s32;
	int len = option[OPT_LEN - 2];

	dest += sprintf(dest, "%s=", type_p->name);

	type = type_p->flags & TYPE_MASK;
	optlen = option_lengths[type];
	for(;;) {
		switch (type) {
		case OPTION_IP_PAIR:
			dest += sprintip(dest, "", option);
			*(dest++) = '/';
			option += 4;
			optlen = 4;
		case OPTION_IP:	/* Works regardless of host byte order. */
			dest += sprintip(dest, "", option);
 			break;
		case OPTION_BOOLEAN:
			dest += sprintf(dest, *option ? "yes" : "no");
			break;
		case OPTION_U8:
			dest += sprintf(dest, "%u", *option);
			break;
		case OPTION_U16:
			memcpy(&val_u16, option, 2);
			dest += sprintf(dest, "%u", ntohs(val_u16));
			break;
		case OPTION_S16:
			memcpy(&val_s16, option, 2);
			dest += sprintf(dest, "%d", ntohs(val_s16));
			break;
		case OPTION_U32:
			memcpy(&val_u32, option, 4);
			dest += sprintf(dest, "%lu", (unsigned long) ntohl(val_u32));
			break;
		case OPTION_S32:
			memcpy(&val_s32, option, 4);
			dest += sprintf(dest, "%ld", (long) ntohl(val_s32));
			break;
		case OPTION_STRING:
			memcpy(dest, option, len);
			dest[len] = '\0';
			return;	 /* Short circuit this case */
		}
		option += optlen;
		len -= optlen;
		if (len <= 0) break;
		dest += sprintf(dest, " ");
	}
}


/* put all the parameters into an environment */
static char **fill_envp(struct dhcpMessage *packet)
{
	int num_options = 0;
	int i, j;
	char **envp;
	uint8_t *temp;
	struct in_addr subnet;
	char over = 0;

	if (packet == NULL)
		num_options = 0;
	else {
		for (i = 0; dhcp_options[i].code; i++)
			if (get_option(packet, dhcp_options[i].code)) {
				num_options++;
				if (dhcp_options[i].code == DHCP_SUBNET)
					num_options++; /* for mton */
			}
		if (packet->siaddr) num_options++;
		if ((temp = get_option(packet, DHCP_OPTION_OVER)))
			over = *temp;
		if (!(over & FILE_FIELD) && packet->file[0]) num_options++;
		if (!(over & SNAME_FIELD) && packet->sname[0]) num_options++;
	}

	envp = xcalloc(sizeof(char *), num_options + 5);
	j = 0;
	asprintf(&envp[j++], "interface=%s", client_config.interface);
	asprintf(&envp[j++], "%s=%s", "PATH",
		getenv("PATH") ? : "/bin:/usr/bin:/sbin:/usr/sbin");
	asprintf(&envp[j++], "%s=%s", "HOME", getenv("HOME") ? : "/");

	if (packet == NULL) return envp;

	envp[j] = xmalloc(sizeof("ip=255.255.255.255"));
	sprintip(envp[j++], "ip=", (uint8_t *) &packet->yiaddr);


	for (i = 0; dhcp_options[i].code; i++) {
		if (!(temp = get_option(packet, dhcp_options[i].code)))
			continue;
		envp[j] = xmalloc(upper_length(temp[OPT_LEN - 2],
			dhcp_options[i].flags & TYPE_MASK) + strlen(dhcp_options[i].name) + 2);
		fill_options(envp[j++], temp, &dhcp_options[i]);

		/* Fill in a subnet bits option for things like /24 */
		if (dhcp_options[i].code == DHCP_SUBNET) {
			memcpy(&subnet, temp, 4);
			asprintf(&envp[j++], "mask=%d", mton(&subnet));
		}
	}
	if (packet->siaddr) {
		envp[j] = xmalloc(sizeof("siaddr=255.255.255.255"));
		sprintip(envp[j++], "siaddr=", (uint8_t *) &packet->siaddr);
	}
	if (!(over & FILE_FIELD) && packet->file[0]) {
		/* watch out for invalid packets */
		packet->file[sizeof(packet->file) - 1] = '\0';
		asprintf(&envp[j++], "boot_file=%s", packet->file);
	}
	if (!(over & SNAME_FIELD) && packet->sname[0]) {
		/* watch out for invalid packets */
		packet->sname[sizeof(packet->sname) - 1] = '\0';
		asprintf(&envp[j++], "sname=%s", packet->sname);
	}
	return envp;
}

/************************************************************************/
#define SUCCESS		0
#define PROCESS		1
#define SUSPEND		2

#define DHCPC_INFO_FILE		"/etc/udhcpc.info"
#define SHARED_MEM_SIZE		64	/* should be larger than size of struct dhcpcInfo */
#define SHARED_MEM_KEY_ID	0x1F

#define UINT32_BITS             32
#define IFNAMSIZ                16
#define DHCPC_MAX_STATIC_ROUTER 32

#if 0
#define IPC_SERVER_PATH			"/tmp/ipc"
#define IPC_DHCPC_PATH			"/tmp/client_dhcpc"
#define UINT32					unsigned int
#define UINT8  					unsigned char
#define IPC_REGISTER			1
#define IPC_UNREGISTER			0
#define ERROR					-1
#define	OK						1

static int sockfd;
#endif

typedef struct
{
	//BOOL	bStatus;		/* Entry esist ? */
	UINT32	ulIp;
	UINT32	ulMask;
	UINT32	ulGateway;
	BOOL	bEnable;	/* Enable or Disable */
    UINT8   sIfName[IFNAMSIZ];
}STATIC_ROUTE_ENTRY;

#ifdef MODULE_TR069/*  by huangwenzhong, 22Nov12 */
/* add tr069 parameters which should get from option 43 */
#define TR069_ACS_URL_MAX_LEN					(256)
#define TR069_DEV_INFO_PROVISION_CODE_MAX_LEN 	(64)

#define DHCP_ACS_URL_CODE						(1)
#define DHCP_PROVISION_CODE						(2)

#define DHCP_ACS_URL_BIT						(0)
#define DHCP_PRO_CODE_BIT						(1)
typedef struct
{
	unsigned int	flag;
	unsigned char	acsUrl[TR069_ACS_URL_MAX_LEN];
	unsigned char	provisionCode[TR069_DEV_INFO_PROVISION_CODE_MAX_LEN];
}TR069_CFG;
#endif

typedef struct
{
	uint8_t  status;
	uint32_t ip;
	uint32_t mask;
	uint32_t gw;
	uint32_t dns[2];	/* 2 dns is enough */
#ifdef MODULE_TR069/*  by huangwenzhong, 26Nov12 */
	TR069_CFG tr069Cfg;
#endif
    uint32_t sr_num;/* sr_num and srouter field must be the last two of this struct */
    STATIC_ROUTE_ENTRY srouter[0];
}dhcpcInfo;

#if MODULE_DHCP_REQUEST_SAME_IP/*  by huangwenzhong, 22Apr13 */
unsigned long last_ip = 0;
#endif

void dhcpc_ipc_fork(void)
{
	int dhcpc_qid = -1;
	int cmd = -1;
	UINT8 rcvBuf[MSG_MAX_LEN + 4];	/* 4 bytes long: msg type */
	int len = 0;
	tp_ipc_msg *pMsg = NULL;
	int pid = fork();
	
	if (pid == 0)
	{
		dhcpc_qid = msgget(MSG_DHCPC_KEY, IPC_CREAT);
		if (dhcpc_qid < 0)
		{
			exit(0);
		}
		
		for (;;)
		{
			len = 0;
			pMsg = NULL;
			len = msgq_ipc_rcv(dhcpc_qid, DHCPC, rcvBuf, MSG_MAX_LEN + 4);

			if (len >= 0)
			{
				pMsg = (tp_ipc_msg*)(rcvBuf + 4);
				if (pMsg)
				{
					cmd = (int)(pMsg->payload[0]);
					if (cmd >= 0)
					{
						udhcp_sp_ipc_inform(cmd);
					}
				}
			}
			sleep(1);
		}
		exit(0);
	}
}

void dhcpc_ipc_send(int dstMid, int msgType, UINT8* ptr, UINT32 nbytes)
{
	static int dhcpc_qid = -1;

	dhcpc_qid = msgget(MSG_DHCPC_KEY, IPC_CREAT);
	if (dhcpc_qid < 0)
	{
		return;
	}
	msgq_ipc_send(dhcpc_qid, DHCPC, dstMid, msgType, ptr, nbytes);
	return;
}

#if MODULE_SMART_DMZ
/*

#include <stdarg.h>

void PRINTF_ECHO(char* cmd, ...)
{
	char tmpbuf[256];
	char cmdline[512];
    va_list vaList;
    va_start (vaList, cmd);
    vsprintf (tmpbuf, cmd, vaList);
    va_end (vaList);
	
    sprintf(cmdline, "echo \"====>>>>%s\" > /dev/ttyS0 \r\n", tmpbuf);
	system(cmdline);
}
*/
void dhcps_ipc_fork(void)
{
	int dhcps_qid = -1;
	UINT8 rcvBuf[MSG_MAX_LEN + 4];	/* 4 bytes long: msg type */
	int len = 0;
	tp_ipc_msg *pMsg = NULL;
	tp_static_lease *tp_lease = NULL;
	int pid = fork();
	
	if (pid == 0)
	{
		dhcps_qid = msgget(MSG_DHCPS_KEY, IPC_CREAT);
		if (dhcps_qid < 0)
		{
			exit(0);
		}
		
		for (;;)
		{
			len = 0;
			pMsg = NULL;
			len = msgq_ipc_rcv(dhcps_qid, DHCPS, rcvBuf, MSG_MAX_LEN + 4);

			if (len >= 0)
			{
				pMsg = (tp_ipc_msg*)(rcvBuf + 4);
				if (pMsg)
				{
					tp_lease = (tp_static_lease*)(pMsg->payload);

					if (tp_lease != NULL)
					{		
						udhcps_sp_ipc_inform((unsigned char*)tp_lease, sizeof(tp_static_lease));
					}
				}
			}
			usleep(500 * 1000);
		}
		exit(0);
	}
}

#endif

static inline int parseClasslessStaticRoute (uint8_t *index, int len, STATIC_ROUTE_ENTRY *srouter)
{
    int     mask_num = 0, ip_bytes = 0;
    
    if (NULL == srouter || NULL == index || *index > UINT32_BITS)
    {
        return -1;
    }

    mask_num = *index;
    ip_bytes = (mask_num - 1) / 8 + 1;
    
    if (len < (ip_bytes + 1 + 4))
    {
        return -1;
    }

    memcpy (&srouter->ulIp, index + 1, ip_bytes);
    srouter->ulIp = ntohl(srouter->ulIp);
    
    srouter->ulMask = (~0u) << (UINT32_BITS - mask_num);
    
    memcpy (&srouter->ulGateway, index + 1 + ip_bytes, 4);
    srouter->ulGateway = ntohl(srouter->ulGateway);
    srouter->bEnable = TRUE;
    
    return ip_bytes + 1 + 4;
}


static inline int getClasslessStaticRoute (uint8_t *index, int len, STATIC_ROUTE_ENTRY *srouter, int num)
{
    int     i, pos, ret; 
        
    if (NULL == index || NULL == srouter || len <= 0 || num <= 0)
    {
        return -1;
    }

    i = 0; 
    pos = 0;
    
    do
    {
        ret = parseClasslessStaticRoute (index + pos, len - pos, srouter + i);
        i++;
        pos += ret;
    }
    while (i < num && ret > 0 && len > pos);

    if (ret < 0)
    {
        return -1;
    }

    return i;
    
}

static inline int getStaticRoute (uint8_t *index, int len, STATIC_ROUTE_ENTRY *srouter, int num)
{
    int     i, j, pos;
    uint8_t *pch;

    if (NULL == index || NULL == srouter || len <= 0 || num <= 0 || len % 8 != 0)
    {
        return -1;
    }
    
    pos = 0;
    i = 0;
    do
    {
        memcpy(&(srouter[i].ulIp), index + pos, 4);
        srouter[i].ulIp = ntohl(srouter[i].ulIp);

        j = 0;
        srouter[i].ulMask = 0;
        pch = (uint8_t *)&srouter[i].ulMask;
        while(j < 4)
        {
            if (*(index + pos + 3 - j))
            {
                break;
            }

            pch[3-j] = 0xff;
            j++;
        }

        srouter[i].ulMask = ntohl(~srouter[i].ulMask);
        memcpy(&srouter[i].ulGateway, index + pos + 4, 4);
        srouter[i].ulGateway = ntohl(srouter[i].ulGateway);
        srouter[i].bEnable = TRUE;
        
        i++;
        pos += 8;
    }
    while (i < num && len > pos);

    return i;
}

#ifdef MODULE_TR069

/*  by huangwenzhong, 22Nov12 */
/*
get acs url and provision code from option 43.
*/
static inline int getTR069ParamFromOption43(uint8_t *option, int len, TR069_CFG *tr069Cfg)
{
	int ret =  -1;
	int pos = 0;
	uint8_t subCode = 0;
	uint8_t subOptionLen = 0;
	if ((NULL == option) || (len <= 0) || (NULL == tr069Cfg))
    {
        return -1;
    }

	do
	{
		subCode = option[pos++];
		subOptionLen = option[pos++];
		if ((DHCP_ACS_URL_CODE == subCode) && (TR069_ACS_URL_MAX_LEN > subOptionLen))
		{
			memset(tr069Cfg->acsUrl, 0, TR069_ACS_URL_MAX_LEN);
			memcpy(tr069Cfg->acsUrl, option + pos, subOptionLen);
			tr069Cfg->flag |= 1 << DHCP_ACS_URL_BIT;
			ret = 0;
		}
		else if ((DHCP_PROVISION_CODE == subCode) && 
			(TR069_DEV_INFO_PROVISION_CODE_MAX_LEN > subOptionLen))
		{
			memset(tr069Cfg->provisionCode, 0, TR069_DEV_INFO_PROVISION_CODE_MAX_LEN);
			memcpy(tr069Cfg->provisionCode, option + pos, subOptionLen);
			tr069Cfg->flag |= 1 << DHCP_PRO_CODE_BIT;
			ret = 0;
		}
		pos += subOptionLen;
	}while (pos < len);
	
	return ret;
}
#endif


/* get the info we need (ip, mask, gw, dns, status) and write to file */
int update_info(struct dhcpMessage *packet, const char *name)
{
	//dhcpcInfo info;
	static uint8_t  pBuff[MSG_MAX_LEN];
    dhcpcInfo       *info;
	uint8_t         *mask, *gw, *dns, *csr, *sr;
#ifdef MODULE_TR069
	uint8_t			*option43 = NULL;
#endif
	uint32_t dns_num = 0, i = 0;
	int             len, ret;    
	
	if (name == NULL)
	{
		return -1;
	}

    memset(pBuff, 0, MSG_MAX_LEN);
    info = (dhcpcInfo *)pBuff;
    info->sr_num = 0;

	if (strcmp(name, "renew") == 0 || strcmp(name, "bound") == 0)
		info->status = SUCCESS;
	else if (strcmp(name, "deconfig") == 0 || strcmp(name, "nak") == 0)
		info->status = PROCESS;
	else /*if (strcmp(name, "release") == 0 ||
			strcmp(name, "leasefail") == 0 ||
			strcmp(name, "nak") == 0)*/		
		info->status = SUSPEND;

	if (info->status != SUCCESS)
	{
		info->ip = 0;
		info->mask = 0;
		info->gw = 0;
		info->dns[0]= 0;
		info->dns[1] = 0;
	}
	else if (info->status == SUCCESS && packet != NULL)	// GOT
	{
		info->ip = packet->yiaddr;
		
		if ((mask = get_option(packet, DHCP_SUBNET)) != NULL)
			memcpy(&(info->mask), mask, 4);
		
		if ((gw = get_option(packet, DHCP_ROUTER)) != NULL)
			memcpy(&(info->gw), gw, 4);
		
		if ((dns = get_option(packet, DHCP_DNS_SERVER)) != NULL)
		{
			dns_num = (*(dns - 1))/4;
			dns_num = dns_num > 2 ? 2 : dns_num;
			
			for (i = 0; i < dns_num; i++)
			{
				memcpy(info->dns + i, dns + 4 * i, 4);
			}
		}

        info->sr_num = 0;
        
        /* get classless static router option */
        if ((csr = get_option (packet, DHCP_CLASSLESS_STATIC_ROUTE)) != NULL)
        {
            len = (int) *(csr - 1);
            ret = getClasslessStaticRoute (csr, len, &info->srouter[info->sr_num], DHCPC_MAX_STATIC_ROUTER - info->sr_num);
            if (ret < 0)
            {
                msglogd (LOG_INFO, LOGTYPE_DHCP, "DHCPC: parse classless static route option error");
            }
            else
            {
                msglogd (LOG_INFO, LOGTYPE_DHCP, "DHCPC: get %d static route from classless static route option(121)", ret);
                info->sr_num += ret;
            }
        }

		/* get Microsoft Classless static route option (option code 249), the difference between option 121 and 249
		 * is only the option code. add by lqt, 2010.6.21 */
		if ((csr = get_option (packet, DHCP_MS_CLASSLESS_STATIC_ROUTE)) != NULL)
        {
            len = (int) *(csr - 1);
            ret = getClasslessStaticRoute (csr, len, &info->srouter[info->sr_num], DHCPC_MAX_STATIC_ROUTER - info->sr_num);
            if (ret < 0)
            {
                msglogd (LOG_INFO, LOGTYPE_DHCP, "DHCPC: parse Microsoft classless static route option error");
            }
            else
            {
                msglogd (LOG_INFO, LOGTYPE_DHCP, "DHCPC: get %d static route from Microsoft classless static route option(249)", ret);
                info->sr_num += ret;
            }
        }
		/* add end */

        if ((sr = get_option (packet, DHCP_STATIC_ROUTE)) != NULL)
        {
            len = (int) *(sr - 1);
            ret = getStaticRoute (sr, len, &info->srouter[info->sr_num], DHCPC_MAX_STATIC_ROUTER - info->sr_num);
            if (ret < 0)
            {
                msglogd (LOG_INFO, LOGTYPE_DHCP, "DHCPC: parse static route option error");
            }
            else
            {
                msglogd (LOG_INFO, LOGTYPE_DHCP, "DHCPC: get %d static route from static route option(33)", ret);
                info->sr_num += ret;
            }
        }
#ifdef MODULE_TR069
		info->tr069Cfg.flag = 0;
		if ((option43 = get_option(packet, DHCP_VENDOR_SPEC_INFO)) != NULL)
		{
			len = (int) *(option43 - 1);
			ret = getTR069ParamFromOption43(option43, len, &info->tr069Cfg);
			if (ret < 0)
			{
				msglogd (LOG_INFO, LOGTYPE_DHCP, "parse tr069 parameters from option(43) fail");
			}
			else
			{
				msglogd (LOG_INFO, LOGTYPE_DHCP, "get tr069 paramteters from  option(43) successfully");
			}
		}
#endif        
		msglogd(LOG_INFO, LOGTYPE_DHCP, "DHCPC:GET ip:%x mask:%x gateway:%x dns1:%x dns2:%x static route:%x", 
			info->ip, info->mask, info->gw, info->dns[0], info->dns[1], info->sr_num);

		//config_interface(&info);
#if MODULE_DHCP_REQUEST_SAME_IP/*  by huangwenzhong, 22Apr13 */
		last_ip = info->ip;
#endif
	}

	dhcpc_ipc_send(HTTPD, IPC_DATA_SET, (UINT8*)info, 
		sizeof(dhcpcInfo) + info->sr_num * sizeof(STATIC_ROUTE_ENTRY));
	return 0;
}
/***********************************************************************/

#if MODULE_DHCP_REQUEST_SAME_IP/*  by huangwenzhong, 22Apr13 */
unsigned long dhcpc_get_last_ip(void)
{
	return last_ip;
}
#endif

/* Call a script with a par file and env vars */
void run_script(struct dhcpMessage *packet, const char *name)
{
	update_info(packet, name);
}

