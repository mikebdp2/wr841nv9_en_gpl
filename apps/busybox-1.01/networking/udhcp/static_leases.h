/* static_leases.h */
#ifndef _STATIC_LEASES_H
#define _STATIC_LEASES_H

#include "dhcpd.h"

#if MODULE_SMART_DMZ

int addStaticLease(struct static_lease **lease_struct, uint8_t *mac, uint32_t *ip, uint32_t lease);
uint32_t getLeaseTimeByMac(struct static_lease *lease_struct, void *arg);/*  by huangwenzhong, 21Aug12 */
struct static_option* getOptionByMac(struct static_lease *lease_struct, uint8_t *mac);
void tp_handle_static_lease(struct static_lease **lease_struct, tp_static_lease *tp_lease);/*  by huangwenzhong, 21Aug12 */

#else

/* Config file will pass static lease info to this function which will add it
 * to a data structure that can be searched later */
int addStaticLease(struct static_lease **lease_struct, uint8_t *mac, uint32_t *ip);

#endif

/* Check to see if a mac has an associated static lease */
uint32_t getIpByMac(struct static_lease *lease_struct, void *arg);

/* Check to see if an ip is reserved as a static ip */
uint32_t reservedIp(struct static_lease *lease_struct, uint32_t ip);

#ifdef UDHCP_DEBUG
/* Print out static leases just to check what's going on */
void printStaticLeases(struct static_lease **lease_struct);
#endif

#endif



