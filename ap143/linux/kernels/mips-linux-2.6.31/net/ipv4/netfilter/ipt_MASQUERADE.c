/* Masquerade.  Simple mapping which alters range to a local IP address
   (depending on route). */

/* 9/25/2010, Optimize for strict implementation of Cone, by yingming.yu@atheros.com
 * (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <net/protocol.h>
#include <net/ip.h>
#include <net/checksum.h>
#include <net/route.h>
#include <net/netfilter/nf_nat_rule.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/x_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("Xtables: automatic-address SNAT");

/*handle NATTYPE Stuff,only if NATTYPE module was defined*/
#if defined(CONFIG_IP_NF_TARGET_NATTYPE_MODULE)
    void (*nattype_update_rule)(struct sk_buff *skb, __be32 newsrc);
/* Registration hooks for nattype. */
int
masq_register_nattype(void *nattype_fn)
{
    if (!nattype_fn)
        return -1;
    nattype_update_rule = nattype_fn;
    printk("%s: register nattype_update_rule %p\n", __func__,nattype_update_rule);
	return 0;
}
EXPORT_SYMBOL(masq_register_nattype);
/* Unregistration hooks for nattype. */
int
masq_unregister_nattype(void)
{
    nattype_update_rule = NULL;
    printk("%s:unregister nattype_update_rule\n",__func__);
	return 0;
}
EXPORT_SYMBOL(masq_unregister_nattype);
#endif

/* FIXME: Multiple targets. --RR */
static bool masquerade_tg_check(const struct xt_tgchk_param *par)
{
	const struct nf_nat_multi_range_compat *mr = par->targinfo;

	if (mr->range[0].flags & IP_NAT_RANGE_MAP_IPS) {
		pr_debug("masquerade_check: bad MAP_IPS.\n");
		return false;
	}
	if (mr->rangesize != 1) {
		pr_debug("masquerade_check: bad rangesize %u\n", mr->rangesize);
		return false;
	}
	return true;
}

static unsigned int
masquerade_tg(struct sk_buff *skb, const struct xt_target_param *par)
{
	struct nf_conn *ct;
	struct nf_conn_nat *nat;
	enum ip_conntrack_info ctinfo;
	struct nf_nat_range newrange;
	const struct nf_nat_multi_range_compat *mr;
	const struct rtable *rt;
	__be32 newsrc;

	NF_CT_ASSERT(par->hooknum == NF_INET_POST_ROUTING);

	ct = nf_ct_get(skb, &ctinfo);
	nat = nfct_nat(ct);

	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED
			    || ctinfo == IP_CT_RELATED + IP_CT_IS_REPLY));

	/* Source address is 0.0.0.0 - locally generated packet that is
	 * probably not supposed to be masqueraded.
	 */
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip == 0)
		return NF_ACCEPT;

	mr = par->targinfo;
	rt = skb_rtable(skb);
	newsrc = inet_select_addr(par->out, rt->rt_gateway, RT_SCOPE_UNIVERSE);
	if (!newsrc) {
		printk("MASQUERADE: %s ate my IP address\n", par->out->name);
		return NF_DROP;
	}

/*handle NATTYPE Stuff,only if NATTYPE module was defined*/
#if defined(CONFIG_IP_NF_TARGET_NATTYPE_MODULE)
    if(nattype_update_rule)
        nattype_update_rule(skb,newsrc);
#endif

	nat->masq_index = par->out->ifindex;

	/* Transfer from original range. */
	newrange = ((struct nf_nat_range)
		{ mr->range[0].flags | IP_NAT_RANGE_MAP_IPS,
		  newsrc, newsrc,
		  mr->range[0].min, mr->range[0].max });

	/* Hand modified range to generic setup. */
	return nf_nat_setup_info(ct, &newrange, IP_NAT_MANIP_SRC);
}

static int
device_cmp(struct nf_conn *i, void *ifindex)
{
	const struct nf_conn_nat *nat = nfct_nat(i);

	if (!nat)
		return 0;

	return nat->masq_index == (int)(long)ifindex;
}

static int masq_device_event(struct notifier_block *this,
			     unsigned long event,
			     void *ptr)
{
	const struct net_device *dev = ptr;
	struct net *net = dev_net(dev);

	if (event == NETDEV_DOWN) {
		/* Device was downed.  Search entire table for
		   conntracks which were associated with that device,
		   and forget them. */
		NF_CT_ASSERT(dev->ifindex != 0);

		nf_ct_iterate_cleanup(net, device_cmp,
				      (void *)(long)dev->ifindex);
	}

	return NOTIFY_DONE;
}

static int masq_inet_event(struct notifier_block *this,
			   unsigned long event,
			   void *ptr)
{
	struct net_device *dev = ((struct in_ifaddr *)ptr)->ifa_dev->dev;
	return masq_device_event(this, event, dev);
}

static struct notifier_block masq_dev_notifier = {
	.notifier_call	= masq_device_event,
};

static struct notifier_block masq_inet_notifier = {
	.notifier_call	= masq_inet_event,
};
 

static struct xt_target masquerade_tg_reg __read_mostly = {
	.name		= "MASQUERADE",
	.family		= NFPROTO_IPV4,
	.target		= masquerade_tg,
	.targetsize	= sizeof(struct nf_nat_multi_range_compat),
	.table		= "nat",
	.hooks		= 1 << NF_INET_POST_ROUTING,
	.checkentry	= masquerade_tg_check,
	.me		= THIS_MODULE,
};

static int __init masquerade_tg_init(void)
{
	int ret;
#if defined(CONFIG_IP_NF_TARGET_NATTYPE_MODULE)
    nattype_update_rule = NULL;
#endif
	ret = xt_register_target(&masquerade_tg_reg);

	if (ret == 0) {
		/* Register for device down reports */
		register_netdevice_notifier(&masq_dev_notifier);
		/* Register IP address change reports */
		register_inetaddr_notifier(&masq_inet_notifier);
	}

	return ret;
}

static void __exit masquerade_tg_exit(void)
{
	xt_unregister_target(&masquerade_tg_reg);
	unregister_netdevice_notifier(&masq_dev_notifier);
	unregister_inetaddr_notifier(&masq_inet_notifier);
}

module_init(masquerade_tg_init);
module_exit(masquerade_tg_exit);
