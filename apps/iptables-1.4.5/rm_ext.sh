#!/bin/bash

unused_so='libipt_ULOG.so	libxt_dscp.so	libxt_NFQUEUE.so	libxt_statistic.so 
libipt_ecn.so     libxt_CLASSIFY.so     libxt_DSCP.so       libxt_owner.so    libxt_tcpmss.so 
libipt_ECN.so     libxt_cluster.so      libxt_pkttype.so	libxt_tos.so 
libipt_LOG.so     libxt_connbytes.so    libxt_hashlimit.so  libxt_quota.so    libxt_TOS.so 
libipt_MIRROR.so  libxt_connlimit.so    libxt_helper.so     libxt_rateest.so  libxt_u32.so 
libipt_NETMAP.so  libxt_length.so     libxt_RATEEST.so 
libipt_realm.so   libxt_limit.so      libxt_recent.so 
libipt_ttl.so     libxt_CONNSECMARK.so  libxt_sctp.so 
libipt_TTL.so     libxt_dccp.so         libxt_NFLOG.so      libxt_SECMARK.so'

ipsec_vpn_so='	libipt_ah.so	libxt_esp.so'

guestNetwork_so='  libxt_connmark.so	libxt_CONNMARK.so'

duplicate_so=' libxt_mark.so'

if [[ $MODULE_IPSEC_VPN = "n" ]]
then
	unused_so+=$ipsec_vpn_so
fi

if [[ $MODULE_GUESTNETWORK_V2 = "n" ]]
then
	unused_so+=$guestNetwork_so
fi

if [[ $MODULE_IPSEC_VPN = "n" && $MODULE_GUESTNETWORK_V2 = "n" ]]
then
	unused_so+=$duplicate_so
fi

for i in $unused_so
do
	rm -rf ${INSTALL_ROOT}/libexec/xtables/$i
done
