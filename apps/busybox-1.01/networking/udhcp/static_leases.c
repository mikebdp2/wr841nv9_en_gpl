/*
 * static_leases.c -- Couple of functions to assist with storing and
 * retrieving data for static leases
 *
 * Wade Berrier <wberrier@myrealbox.com> September 2004
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "static_leases.h"
#include "dhcpd.h"

#if MODULE_SMART_DMZ
/* Takes the address of the pointer to the static_leases linked list,
 *   Address to a 6 byte mac address
 *   Address to a 4 byte ip address */

int addStaticLease(struct static_lease **lease_struct, uint8_t *mac, uint32_t *ip, uint32_t lease)
{

	struct static_lease *cur = NULL;
	struct static_lease *new_static_lease = NULL;
	struct static_option *option = NULL;
	struct static_option *tmp_option = NULL;

	/* Build new node */
	if (lease_struct)
	{
		cur = *lease_struct;
		while (cur)
		{
			/* if its static_lease is already exist, replace it*/
			if (memcmp(cur->mac, mac, 6) == 0)
			{
				free(cur->ip);
				cur->ip = ip;
				cur->lease = lease;
				option = cur->option;/* free its option */				
				cur->option = NULL;
				while (option)
				{
					tmp_option = option->next;
					if (option->data)
					{
						free(option->data);
					}
					free(option);

					option = tmp_option;
				}
				return 1;
			}
			cur = cur->next;
		}
	}

	cur = NULL;
	new_static_lease = xmalloc(sizeof(struct static_lease));
	new_static_lease->mac = mac;
	new_static_lease->ip = ip;
	new_static_lease->lease = lease;
	new_static_lease->option = NULL;
	new_static_lease->next = NULL;	
	
	/* If it's the first node to be added... */
	if(*lease_struct == NULL)
	{
		*lease_struct = new_static_lease;		
	}
	else
	{
		cur = *lease_struct;
		while(cur->next != NULL)
		{
			cur = cur->next;
		}

		cur->next = new_static_lease;		
	}

	return 1;

}

/* 
return  -1 fail
return 0 success
*/
int addOption2StaticLease(struct static_lease *lease_struct, uint8_t *mac, uint8_t op, uint8_t len, uint8_t* val)
{
	struct static_lease *cur = lease_struct;
	struct static_option *new_option  = NULL;
	uint8_t *data = NULL;

	while(cur != NULL)
	{
		/* If the client has the correct mac  */
		if(memcmp(cur->mac, mac, 6) == 0)
		{
			new_option = xmalloc(sizeof(struct static_option));
			data = xmalloc(len + 1);
			if (new_option == NULL || data == NULL)
			{
				return -1;
			}
			
			new_option->len = len;
			new_option->option = op;
			memcpy(data, val, len);
			new_option->data = data;
			
			new_option->next = cur->option;
			cur->option = new_option;
			return 0;
		}

		cur = cur->next;
	}

	return -1;
}

/*
return 0 - success
return -1 - fail
Note:
we only del static lease which for twin ip
*/
int delStaticLease(struct static_lease **lease_struct)
{
	struct static_lease *cur = *lease_struct;
	struct static_lease *prev = cur;
	struct static_option *option = NULL, *tmp = NULL;


	while(cur != NULL)
	{
		/*  by huangwenzhong, 23Aug12 */
		/* 
		static lease of twin ip  is not zero
		other static lease is zero. so we can simply del static lease whose lease
		is not zero.
		but I feel it is ugly.
		*/
		if(cur->lease != 0)
		{
			del_lease_by_chaddr(cur->mac);/* del static lease from lease array first */
			option = cur->option;			
			while(option != NULL)
			{
				tmp = option->next;
				if (option->data != NULL)
				{
					free(option->data);
				}
				free(option);
				option = tmp;
			}

			if (cur->ip != NULL)
			{
				free(cur->ip);				
			}

			if (cur->mac != NULL)
			{
				free(cur->mac);
			}
			
			if (prev == cur)
			{
				free(cur);
				prev = NULL;
				*lease_struct = NULL;
			}
			else
			{
				prev->next = cur->next;				
				free(cur);
			}
			return 0;/* we assume have only one */
		}
		prev = cur;
		cur = cur->next;
	}

	return -1;
}

void tp_handle_static_lease(struct static_lease **lease_struct, tp_static_lease *tp_lease)
{
	uint8_t *mac = NULL;
	uint32_t *ip = NULL;
	if (tp_lease->op == STATIC_LEASE_ADD)
	{
		mac = xmalloc(sizeof(unsigned char) * 8);
		ip = xmalloc(sizeof(uint32_t));

		memcpy(mac, tp_lease->mac, MAC_ADDR_SIZE);
		memcpy(ip, &(tp_lease->ip), sizeof(uint32_t));
		
		addStaticLease(lease_struct, mac, ip, tp_lease->lease); 
		
		addOption2StaticLease(*lease_struct, tp_lease->mac,
				DHCP_SUBNET, sizeof(tp_lease->mask), &(tp_lease->mask));
		addOption2StaticLease(*lease_struct, tp_lease->mac,
				DHCP_ROUTER, sizeof(tp_lease->gw), &(tp_lease->gw));
	}
	else if (tp_lease->op == STATIC_LEASE_DEL)
	{
		/* 
		if the pc whose mac is equal to tp_lease->mac is already get ip from 
		server, del its lease first.
		*/
		del_lease_by_chaddr(tp_lease->mac);
		delStaticLease(lease_struct);
	}
	return;
}

uint32_t getLeaseTimeByMac(struct static_lease *lease_struct, void *arg)
{
	uint32_t lease_time = 0;
	struct static_lease *cur = lease_struct;
	uint8_t *mac = arg;

	while(cur != NULL)
	{
		/* If the client has the correct mac  */
		if(memcmp(cur->mac, mac, 6) == 0)
		{			
			lease_time = cur->lease;
			break;
		}

		cur = cur->next;
	}

	return lease_time;

}

struct static_option* getOptionByMac(struct static_lease *lease_struct, uint8_t *mac)
{
	struct static_option *option = NULL;

	struct static_lease *cur = lease_struct;

	while(cur != NULL)
	{
		/* If the client has the correct mac  */
		if(memcmp(cur->mac, mac, MAC_ADDR_SIZE) == 0)
		{
			option = cur->option;
			break;
		}

		cur = cur->next;
	}

	return option;
}

#else

/* Takes the address of the pointer to the static_leases linked list,
 *   Address to a 6 byte mac address
 *   Address to a 4 byte ip address */
int addStaticLease(struct static_lease **lease_struct, uint8_t *mac, uint32_t *ip)
{

	struct static_lease *cur;
	struct static_lease *new_static_lease;

	/* Build new node */
	new_static_lease = xmalloc(sizeof(struct static_lease));
	new_static_lease->mac = mac;
	new_static_lease->ip = ip;
	new_static_lease->next = NULL;

	/* If it's the first node to be added... */
	if(*lease_struct == NULL)
	{
		*lease_struct = new_static_lease;
	}
	else
	{
		cur = *lease_struct;
		while(cur->next != NULL)
		{
			cur = cur->next;
		}

		cur->next = new_static_lease;
	}

	return 1;

}
#endif

/* Check to see if a mac has an associated static lease */
uint32_t getIpByMac(struct static_lease *lease_struct, void *arg)
{
	uint32_t return_ip;
	struct static_lease *cur = lease_struct;
	uint8_t *mac = arg;

	return_ip = 0;

	while(cur != NULL)
	{
		/* If the client has the correct mac  */
		if(memcmp(cur->mac, mac, 6) == 0)
		{
			return_ip = *(cur->ip);
			break;
		}

		cur = cur->next;
	}

	return return_ip;

}

/* Check to see if an ip is reserved as a static ip */
uint32_t reservedIp(struct static_lease *lease_struct, uint32_t ip)
{
	struct static_lease *cur = lease_struct;

	uint32_t return_val = 0;

	while(cur != NULL)
	{
		/* If the client has the correct ip  */
		if(*cur->ip == ip)
			return_val = 1;

		cur = cur->next;
	}

	return return_val;

}

#ifdef UDHCP_DEBUG
/* Print out static leases just to check what's going on */
/* Takes the address of the pointer to the static_leases linked list */
void printStaticLeases(struct static_lease **arg)
{
	/* Get a pointer to the linked list */
	struct static_lease *cur = *arg;

	while(cur != NULL)
	{
		/* printf("PrintStaticLeases: Lease mac Address: %x\n", cur->mac); */
		printf("PrintStaticLeases: Lease mac Value: %x\n", *(cur->mac));
		/* printf("PrintStaticLeases: Lease ip Address: %x\n", cur->ip); */
		printf("PrintStaticLeases: Lease ip Value: %x\n", *(cur->ip));

		cur = cur->next;
	}


}
#endif



