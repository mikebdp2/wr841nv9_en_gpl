/*
 * Copyright (c) 2010, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef _NET80211_IEEE80211_IOCTL_H_
#define _NET80211_IEEE80211_IOCTL_H_

/*
 * IEEE 802.11 ioctls.
 */
#ifndef EXTERNAL_USE_ONLY
#include <_ieee80211.h>
/* duplicate defination - to avoid including ieee80211_var.h */
#define IEEE80211_ADDR_COPY(dst,src)    OS_MEMCPY(dst, src, IEEE80211_ADDR_LEN)
#define IEEE80211_KEY_XMIT      0x01    /* key used for xmit */
#define IEEE80211_KEY_RECV      0x02    /* key used for recv */
#define IEEE80211_ADDR_EQ(a1,a2)        (OS_MEMCMP(a1, a2, IEEE80211_ADDR_LEN) == 0)
#define IEEE80211_APPIE_MAX                  1024 /* max appie buffer size */
#define IEEE80211_KEY_GROUP     0x04    /* key used for WPA group operation */
#define IEEE80211_SCAN_MAX_SSID     10
#endif /* EXTERNAL_USE_ONLY */


/*
 * Max size of optional information elements.  We artificially
 * constrain this; it's limited only by the max frame size (and
 * the max parameter size of the wireless extensions).
 */
#define	IEEE80211_MAX_OPT_IE	512
#define	IEEE80211_MAX_WSC_IE	256

/*
 * WPA/RSN get/set key request.  Specify the key/cipher
 * type and whether the key is to be used for sending and/or
 * receiving.  The key index should be set only when working
 * with global keys (use IEEE80211_KEYIX_NONE for ``no index'').
 * Otherwise a unicast/pairwise key is specified by the bssid
 * (on a station) or mac address (on an ap).  They key length
 * must include any MIC key data; otherwise it should be no
 more than IEEE80211_KEYBUF_SIZE.
 */
struct ieee80211req_key {
	u_int8_t	ik_type;	/* key/cipher type */
	u_int8_t	ik_pad;
	u_int16_t	ik_keyix;	/* key index */
	u_int8_t	ik_keylen;	/* key length in bytes */
	u_int8_t	ik_flags;
/* NB: IEEE80211_KEY_XMIT and IEEE80211_KEY_RECV defined elsewhere */
#define	IEEE80211_KEY_DEFAULT	0x80	/* default xmit key */
	u_int8_t	ik_macaddr[IEEE80211_ADDR_LEN];
	u_int64_t	ik_keyrsc;	/* key receive sequence counter */
	u_int64_t	ik_keytsc;	/* key transmit sequence counter */
	u_int8_t	ik_keydata[IEEE80211_KEYBUF_SIZE+IEEE80211_MICBUF_SIZE];
};

/*
 * Delete a key either by index or address.  Set the index
 * to IEEE80211_KEYIX_NONE when deleting a unicast key.
 */
struct ieee80211req_del_key {
	u_int8_t	idk_keyix;	/* key index */
	u_int8_t	idk_macaddr[IEEE80211_ADDR_LEN];
};

/*
 * MLME state manipulation request.  IEEE80211_MLME_ASSOC
 * only makes sense when operating as a station.  The other
 * requests can be used when operating as a station or an
 * ap (to effect a station).
 */
struct ieee80211req_mlme {
	u_int8_t	im_op;		/* operation to perform */
#define	IEEE80211_MLME_ASSOC		1	/* associate station */
#define	IEEE80211_MLME_DISASSOC		2	/* disassociate station */
#define	IEEE80211_MLME_DEAUTH		3	/* deauthenticate station */
#define	IEEE80211_MLME_AUTHORIZE	4	/* authorize station */
#define	IEEE80211_MLME_UNAUTHORIZE	5	/* unauthorize station */
#define	IEEE80211_MLME_STOP_BSS		6	/* stop bss */
#define IEEE80211_MLME_CLEAR_STATS	7	/* clear station statistic */
#define IEEE80211_MLME_AUTH	        8	/* auth resp to station */
#define IEEE80211_MLME_REASSOC	        9	/* reassoc to station */
	u_int8_t	im_ssid_len;	/* length of optional ssid */
	u_int16_t	im_reason;	/* 802.11 reason code */
	u_int16_t	im_seq;	        /* seq for auth */
	u_int8_t	im_macaddr[IEEE80211_ADDR_LEN];
	u_int8_t	im_ssid[IEEE80211_NWID_LEN];
	u_int8_t        im_optie[IEEE80211_MAX_OPT_IE];
	u_int16_t       im_optie_len;
};


/*
 * Retrieve the WPA/RSN information element for an associated station.
 */
struct ieee80211req_wpaie {
	u_int8_t	wpa_macaddr[IEEE80211_ADDR_LEN];
	u_int8_t	wpa_ie[IEEE80211_MAX_OPT_IE];
	u_int8_t    rsn_ie[IEEE80211_MAX_OPT_IE];
#ifdef ATH_WPS_IE
	u_int8_t    wps_ie[IEEE80211_MAX_OPT_IE];
#endif /* ATH_WPS_IE */
};


/*
 * Retrieve per-node statistics.
 */
struct ieee80211req_sta_stats {
	union {
		/* NB: explicitly force 64-bit alignment */
		u_int8_t	macaddr[IEEE80211_ADDR_LEN];
		u_int64_t	pad;
	} is_u;
	struct ieee80211_nodestats is_stats;
};


/*
 * Scan result data returned for IEEE80211_IOC_SCAN_RESULTS.
 */
struct ieee80211req_scan_result {
	u_int16_t	isr_len;		/* length (mult of 4) */
	u_int16_t	isr_freq;		/* MHz */
	u_int32_t	isr_flags;		/* channel flags */
	u_int8_t	isr_noise;
	u_int8_t	isr_rssi;
	u_int8_t	isr_intval;		/* beacon interval */
	u_int16_t	isr_capinfo;		/* capabilities */
	u_int8_t	isr_erp;		/* ERP element */
	u_int8_t	isr_bssid[IEEE80211_ADDR_LEN];
	u_int8_t	isr_nrates;
	u_int8_t	isr_rates[IEEE80211_RATE_MAXSIZE];
	u_int8_t	isr_ssid_len;		/* SSID length */
	u_int16_t	isr_ie_len;		/* IE length */
	u_int8_t	isr_pad[4];
#if	TP_FEATURE_GET_SCAN_KEY_TYPE
	u_int8_t	isr_keytype;	/* 0 None, 1 WEP, 2 WPA-PSK, 3 WPA2-PSK */
#endif
	/* variable length SSID followed by IE data */
};



#ifdef __linux__
/*
 * Wireless Extensions API, private ioctl interfaces.
 *
 * NB: Even-numbered ioctl numbers have set semantics and are privileged!
 *	(regardless of the incorrect comment in wireless.h!)
 *	
 *	Note we can only use 32 private ioctls, and yes they are all claimed.
 */
#ifndef _NET_IF_H
#include <linux/if.h>
#endif
#define	IEEE80211_IOCTL_SETPARAM	(SIOCIWFIRSTPRIV+0)
#define	IEEE80211_IOCTL_GETPARAM	(SIOCIWFIRSTPRIV+1)
#define	IEEE80211_IOCTL_SETKEY		(SIOCIWFIRSTPRIV+2)
#define	IEEE80211_IOCTL_SETWMMPARAMS	(SIOCIWFIRSTPRIV+3)
#define	IEEE80211_IOCTL_DELKEY		(SIOCIWFIRSTPRIV+4)
#define	IEEE80211_IOCTL_GETWMMPARAMS	(SIOCIWFIRSTPRIV+5)
#define	IEEE80211_IOCTL_SETMLME		(SIOCIWFIRSTPRIV+6)
#define	IEEE80211_IOCTL_GETCHANINFO	(SIOCIWFIRSTPRIV+7)
#define	IEEE80211_IOCTL_SETOPTIE	(SIOCIWFIRSTPRIV+8)
#define	IEEE80211_IOCTL_GETOPTIE	(SIOCIWFIRSTPRIV+9)
#define	IEEE80211_IOCTL_ADDMAC		(SIOCIWFIRSTPRIV+10)        /* Add ACL MAC Address */
#define	IEEE80211_IOCTL_DELMAC		(SIOCIWFIRSTPRIV+12)        /* Del ACL MAC Address */
#define	IEEE80211_IOCTL_GETCHANLIST	(SIOCIWFIRSTPRIV+13)
#define	IEEE80211_IOCTL_SETCHANLIST	(SIOCIWFIRSTPRIV+14)
#define IEEE80211_IOCTL_KICKMAC		(SIOCIWFIRSTPRIV+15)
#define	IEEE80211_IOCTL_CHANSWITCH	(SIOCIWFIRSTPRIV+16)
#define	IEEE80211_IOCTL_GETMODE		(SIOCIWFIRSTPRIV+17)
#define	IEEE80211_IOCTL_SETMODE		(SIOCIWFIRSTPRIV+18)
#define IEEE80211_IOCTL_GET_APPIEBUF	(SIOCIWFIRSTPRIV+19)
#define IEEE80211_IOCTL_SET_APPIEBUF	(SIOCIWFIRSTPRIV+20)
#define IEEE80211_IOCTL_SET_ACPARAMS	(SIOCIWFIRSTPRIV+21)
#define IEEE80211_IOCTL_FILTERFRAME	(SIOCIWFIRSTPRIV+22)
#define IEEE80211_IOCTL_SET_RTPARAMS	(SIOCIWFIRSTPRIV+23)
#define IEEE80211_IOCTL_DBGREQ	        (SIOCIWFIRSTPRIV+24)
#define IEEE80211_IOCTL_SEND_MGMT	(SIOCIWFIRSTPRIV+26)
#define IEEE80211_IOCTL_SET_MEDENYENTRY (SIOCIWFIRSTPRIV+27)
#define IEEE80211_IOCTL_GET_MACADDR	(SIOCIWFIRSTPRIV+29)        /* Get ACL List */
#define IEEE80211_IOCTL_SET_HBRPARAMS	(SIOCIWFIRSTPRIV+30)
#define IEEE80211_IOCTL_SET_RXTIMEOUT	(SIOCIWFIRSTPRIV+31)

enum {
	IEEE80211_PARAM_TURBO		= 1,	/* turbo mode */
	IEEE80211_PARAM_MODE		= 2,	/* phy mode (11a, 11b, etc.) */
	IEEE80211_PARAM_AUTHMODE	= 3,	/* authentication mode */
	IEEE80211_PARAM_PROTMODE	= 4,	/* 802.11g protection */
	IEEE80211_PARAM_MCASTCIPHER	= 5,	/* multicast/default cipher */
	IEEE80211_PARAM_MCASTKEYLEN	= 6,	/* multicast key length */
	IEEE80211_PARAM_UCASTCIPHERS	= 7,	/* unicast cipher suites */
	IEEE80211_PARAM_UCASTCIPHER	= 8,	/* unicast cipher */
	IEEE80211_PARAM_UCASTKEYLEN	= 9,	/* unicast key length */
	IEEE80211_PARAM_WPA		= 10,	/* WPA mode (0,1,2) */
	IEEE80211_PARAM_ROAMING		= 12,	/* roaming mode */
	IEEE80211_PARAM_PRIVACY		= 13,	/* privacy invoked */
	IEEE80211_PARAM_COUNTERMEASURES	= 14,	/* WPA/TKIP countermeasures */
	IEEE80211_PARAM_DROPUNENCRYPTED	= 15,	/* discard unencrypted frames */
	IEEE80211_PARAM_DRIVER_CAPS	= 16,	/* driver capabilities */
	IEEE80211_PARAM_MACCMD		= 17,	/* MAC ACL operation */
	IEEE80211_PARAM_WMM		= 18,	/* WMM mode (on, off) */
	IEEE80211_PARAM_HIDESSID	= 19,	/* hide SSID mode (on, off) */
	IEEE80211_PARAM_APBRIDGE	= 20,	/* AP inter-sta bridging */
	IEEE80211_PARAM_KEYMGTALGS	= 21,	/* key management algorithms */
	IEEE80211_PARAM_RSNCAPS		= 22,	/* RSN capabilities */
	IEEE80211_PARAM_INACT		= 23,	/* station inactivity timeout */
	IEEE80211_PARAM_INACT_AUTH	= 24,	/* station auth inact timeout */
	IEEE80211_PARAM_INACT_INIT	= 25,	/* station init inact timeout */
	IEEE80211_PARAM_ABOLT		= 26,	/* Atheros Adv. Capabilities */
	IEEE80211_PARAM_DTIM_PERIOD	= 28,	/* DTIM period (beacons) */
	IEEE80211_PARAM_BEACON_INTERVAL	= 29,	/* beacon interval (ms) */
	IEEE80211_PARAM_DOTH		= 30,	/* 11.h is on/off */
	IEEE80211_PARAM_PWRTARGET	= 31,	/* Current Channel Pwr Constraint */
	IEEE80211_PARAM_GENREASSOC	= 32,	/* Generate a reassociation request */
	IEEE80211_PARAM_COMPRESSION	= 33,	/* compression */
	IEEE80211_PARAM_FF		= 34,	/* fast frames support */
	IEEE80211_PARAM_XR		= 35,	/* XR support */
	IEEE80211_PARAM_BURST		= 36,	/* burst mode */
	IEEE80211_PARAM_PUREG		= 37,	/* pure 11g (no 11b stations) */
	IEEE80211_PARAM_AR		= 38,	/* AR support */
	IEEE80211_PARAM_WDS		= 39,	/* Enable 4 address processing */
	IEEE80211_PARAM_BGSCAN		= 40,	/* bg scanning (on, off) */
	IEEE80211_PARAM_BGSCAN_IDLE	= 41,	/* bg scan idle threshold */
	IEEE80211_PARAM_BGSCAN_INTERVAL	= 42,	/* bg scan interval */
	IEEE80211_PARAM_MCAST_RATE	= 43,	/* Multicast Tx Rate */
	IEEE80211_PARAM_COVERAGE_CLASS	= 44,	/* coverage class */
	IEEE80211_PARAM_COUNTRY_IE	= 45,	/* enable country IE */
	IEEE80211_PARAM_SCANVALID	= 46,	/* scan cache valid threshold */
	IEEE80211_PARAM_ROAM_RSSI_11A	= 47,	/* rssi threshold in 11a */
	IEEE80211_PARAM_ROAM_RSSI_11B	= 48,	/* rssi threshold in 11b */
	IEEE80211_PARAM_ROAM_RSSI_11G	= 49,	/* rssi threshold in 11g */
	IEEE80211_PARAM_ROAM_RATE_11A	= 50,	/* tx rate threshold in 11a */
	IEEE80211_PARAM_ROAM_RATE_11B	= 51,	/* tx rate threshold in 11b */
	IEEE80211_PARAM_ROAM_RATE_11G	= 52,	/* tx rate threshold in 11g */
	IEEE80211_PARAM_UAPSDINFO	= 53,	/* value for qos info field */
	IEEE80211_PARAM_SLEEP		= 54,	/* force sleep/wake */
	IEEE80211_PARAM_QOSNULL		= 55,	/* force sleep/wake */
	IEEE80211_PARAM_PSPOLL		= 56,	/* force ps-poll generation (sta only) */
	IEEE80211_PARAM_EOSPDROP	= 57,	/* force uapsd EOSP drop (ap only) */
	IEEE80211_PARAM_MARKDFS		= 58,	/* mark a dfs interference channel when found */
	IEEE80211_PARAM_REGCLASS	= 59,	/* enable regclass ids in country IE */
	IEEE80211_PARAM_CHANBW		= 60,	/* set chan bandwidth preference */
	IEEE80211_PARAM_WMM_AGGRMODE	= 61,	/* set WMM Aggressive Mode */
	IEEE80211_PARAM_SHORTPREAMBLE	= 62, 	/* enable/disable short Preamble */
	IEEE80211_PARAM_BLOCKDFSCHAN	= 63, 	/* enable/disable use of DFS channels */
	IEEE80211_PARAM_CWM_MODE	= 64,	/* CWM mode */
	IEEE80211_PARAM_CWM_EXTOFFSET	= 65,	/* CWM extension channel offset */
	IEEE80211_PARAM_CWM_EXTPROTMODE	= 66,	/* CWM extension channel protection mode */
	IEEE80211_PARAM_CWM_EXTPROTSPACING = 67,/* CWM extension channel protection spacing */
	IEEE80211_PARAM_CWM_ENABLE	= 68,/* CWM state machine enabled */
	IEEE80211_PARAM_CWM_EXTBUSYTHRESHOLD = 69,/* CWM extension channel busy threshold */
	IEEE80211_PARAM_CWM_CHWIDTH	= 70,	/* CWM STATE: current channel width */
	IEEE80211_PARAM_SHORT_GI	= 71,	/* half GI */
	IEEE80211_PARAM_FAST_CC		= 72,	/* fast channel change */

	/*
	 * 11n A-MPDU, A-MSDU support
	 */
	IEEE80211_PARAM_AMPDU		= 73,	/* 11n a-mpdu support */
	IEEE80211_PARAM_AMPDU_LIMIT	= 74,	/* a-mpdu length limit */
	IEEE80211_PARAM_AMPDU_DENSITY	= 75,	/* a-mpdu density */
	IEEE80211_PARAM_AMPDU_SUBFRAMES	= 76,	/* a-mpdu subframe limit */
	IEEE80211_PARAM_AMSDU		= 77,	/* a-msdu support */
	IEEE80211_PARAM_AMSDU_LIMIT	= 78,	/* a-msdu length limit */

	IEEE80211_PARAM_COUNTRYCODE	= 79,	/* Get country code */
	IEEE80211_PARAM_TX_CHAINMASK	= 80,	/* Tx chain mask */
	IEEE80211_PARAM_RX_CHAINMASK	= 81,	/* Rx chain mask */
	IEEE80211_PARAM_RTSCTS_RATECODE	= 82,	/* RTS Rate code */
	IEEE80211_PARAM_HT_PROTECTION	= 83,	/* Protect traffic in HT mode */
	IEEE80211_PARAM_RESET_ONCE	= 84,	/* Force a reset */
	IEEE80211_PARAM_SETADDBAOPER	= 85,	/* Set ADDBA mode */
	IEEE80211_PARAM_TX_CHAINMASK_LEGACY = 86, /* Tx chain mask for legacy clients */
	IEEE80211_PARAM_11N_RATE	= 87,	/* Set ADDBA mode */
	IEEE80211_PARAM_11N_RETRIES	= 88,	/* Tx chain mask for legacy clients */
	IEEE80211_PARAM_DBG_LVL		= 89,	/* Debug Level for specific VAP */
	IEEE80211_PARAM_WDS_AUTODETECT	= 90,	/* Configurable Auto Detect/Delba for WDS mode */
	IEEE80211_PARAM_ATH_RADIO	= 91,	/* returns the name of the radio being used */
	IEEE80211_PARAM_IGNORE_11DBEACON = 92,	/* Don't process 11d beacon (on, off) */
	IEEE80211_PARAM_STA_FORWARD	= 93,	/* Enable client 3 addr forwarding */

	/*
	 * Mcast Enhancement support
	 */
	IEEE80211_PARAM_ME          = 94,   /* Set Mcast enhancement option: 0 disable, 1 tunneling, 2 translate  4 to disable snoop feature*/
	IEEE80211_PARAM_MEDUMP		= 95,	/* Dump the snoop table for mcast enhancement */
	IEEE80211_PARAM_MEDEBUG		= 96,	/* mcast enhancement debug level */
	IEEE80211_PARAM_ME_SNOOPLENGTH	= 97,	/* mcast snoop list length */
	IEEE80211_PARAM_ME_TIMER	= 98,	/* Set Mcast enhancement timer to update the snoop list, in msec */
	IEEE80211_PARAM_ME_TIMEOUT	= 99,	/* Set Mcast enhancement timeout for STA's without traffic, in msec */
	IEEE80211_PARAM_PUREN		= 100,	/* pure 11n (no 11bg/11a stations) */
	IEEE80211_PARAM_BASICRATES	= 101,	/* Change Basic Rates */
	IEEE80211_PARAM_NO_EDGE_CH	= 102,	/* Avoid band edge channels */
	IEEE80211_PARAM_WEP_TKIP_HT	= 103,	/* Enable HT rates with WEP/TKIP encryption */
	IEEE80211_PARAM_RADIO		= 104,	/* radio on/off */
	IEEE80211_PARAM_NETWORK_SLEEP	= 105,	/* set network sleep enable/disable */
	IEEE80211_PARAM_DROPUNENC_EAPOL	= 106,

	/*
	 * Headline block removal
	 */
	IEEE80211_PARAM_HBR_TIMER	= 107,
	IEEE80211_PARAM_HBR_STATE	= 108,

	/*
	 * Unassociated power consumpion improve
	 */
	IEEE80211_PARAM_SLEEP_PRE_SCAN	= 109,
	IEEE80211_PARAM_SCAN_PRE_SLEEP	= 110,
	IEEE80211_PARAM_VAP_IND		= 111,  /* Independent VAP mode for Repeater and AP-STA config */

	/* support for wapi: set auth mode and key */
	IEEE80211_PARAM_SETWAPI		= 112,
	IEEE80211_IOCTL_GREEN_AP_PS_ENABLE = 113,
	IEEE80211_IOCTL_GREEN_AP_PS_TIMEOUT = 114,
	IEEE80211_IOCTL_GREEN_AP_PS_ON_TIME = 115,
	IEEE80211_PARAM_WPS		= 116,
	IEEE80211_PARAM_RX_RATE		= 117,
	IEEE80211_PARAM_CHEXTOFFSET	= 118,
	IEEE80211_PARAM_CHSCANINIT	= 119,
	IEEE80211_PARAM_MPDU_SPACING	= 120,
	IEEE80211_PARAM_HT40_INTOLERANT	= 121,
	IEEE80211_PARAM_CHWIDTH		= 122,
	IEEE80211_PARAM_EXTAP		= 123,   /* Enable client 3 addr forwarding */
        IEEE80211_PARAM_COEXT_DISABLE    = 124,
	IEEE80211_PARAM_ME_DROPMCAST	= 125,	/* drop mcast if empty entry */
	IEEE80211_PARAM_ME_SHOWDENY	= 126,	/* show deny table for mcast enhancement */
	IEEE80211_PARAM_ME_CLEARDENY	= 127,	/* clear deny table for mcast enhancement */
	IEEE80211_PARAM_ME_ADDDENY	= 128,	/* add deny entry for mcast enhancement */
    IEEE80211_PARAM_GETIQUECONFIG = 129, /*print out the iQUE config*/
    IEEE80211_PARAM_CCMPSW_ENCDEC = 130,  /* support for ccmp s/w encrypt decrypt */
      
      /* Support for repeater placement */ 
    IEEE80211_PARAM_CUSTPROTO_ENABLE = 131,
    IEEE80211_PARAM_GPUTCALC_ENABLE  = 132,
    IEEE80211_PARAM_DEVUP            = 133,
    IEEE80211_PARAM_MACDEV           = 134,
    IEEE80211_PARAM_MACADDR1         = 135,
    IEEE80211_PARAM_MACADDR2         = 136, 
    IEEE80211_PARAM_GPUTMODE         = 137, 
    IEEE80211_PARAM_TXPROTOMSG       = 138,
    IEEE80211_PARAM_RXPROTOMSG       = 139,
    IEEE80211_PARAM_STATUS           = 140,
    IEEE80211_PARAM_ASSOC            = 141,
    IEEE80211_PARAM_NUMSTAS          = 142,
    IEEE80211_PARAM_STA1ROUTE        = 143,
    IEEE80211_PARAM_STA2ROUTE        = 144,
    IEEE80211_PARAM_STA3ROUTE        = 145, 
    IEEE80211_PARAM_STA4ROUTE        = 146, 
    IEEE80211_PARAM_TDLS_ENABLE      = 147,  /* TDLS support */
    IEEE80211_PARAM_SET_TDLS_RMAC    = 148,  /* Set TDLS link */
    IEEE80211_PARAM_CLR_TDLS_RMAC    = 149,  /* Clear TDLS link */
    IEEE80211_PARAM_TDLS_MACADDR1    = 150,
    IEEE80211_PARAM_TDLS_MACADDR2    = 151,
    IEEE80211_PARAM_TDLS_ACTION      = 152,  
#if  ATH_SUPPORT_AOW
    IEEE80211_PARAM_SWRETRIES                   = 153,
    IEEE80211_PARAM_RTSRETRIES                  = 154,
    IEEE80211_PARAM_AOW_LATENCY                 = 155,
    IEEE80211_PARAM_AOW_STATS                   = 156,
    IEEE80211_PARAM_AOW_LIST_AUDIO_CHANNELS     = 157,
    IEEE80211_PARAM_AOW_PLAY_LOCAL              = 158,
    IEEE80211_PARAM_AOW_CLEAR_AUDIO_CHANNELS    = 159,
    IEEE80211_PARAM_AOW_INTERLEAVE              = 160,
    IEEE80211_PARAM_AOW_ER                      = 161,
    IEEE80211_PARAM_AOW_PRINT_CAPTURE           = 162,
    IEEE80211_PARAM_AOW_ENABLE_CAPTURE          = 163,
    IEEE80211_PARAM_AOW_FORCE_INPUT             = 164,
    IEEE80211_PARAM_AOW_EC                      = 165,
    IEEE80211_PARAM_AOW_EC_FMAP                 = 166,
    IEEE80211_PARAM_AOW_ES                      = 167,
    IEEE80211_PARAM_AOW_ESS                     = 168,
    IEEE80211_PARAM_AOW_ESS_COUNT               = 169,
    IEEE80211_PARAM_AOW_ESTATS                  = 170,
    IEEE80211_PARAM_AOW_AS                      = 171,
    IEEE80211_PARAM_AOW_PLAY_RX_CHANNEL         = 172,
    IEEE80211_PARAM_AOW_SIM_CTRL_CMD            = 173,
    IEEE80211_PARAM_AOW_FRAME_SIZE              = 174,
    IEEE80211_PARAM_AOW_ALT_SETTING             = 175,
    IEEE80211_PARAM_AOW_ASSOC_ONLY              = 176,
    IEEE80211_PARAM_AOW_EC_RAMP                 = 177,
    IEEE80211_PARAM_AOW_DISCONNECT_DEVICE       = 178,
#endif  /* ATH_SUPPORT_AOW */
    IEEE80211_PARAM_PERIODIC_SCAN = 179,
#if ATH_SUPPORT_AP_WDS_COMBO
    IEEE80211_PARAM_NO_BEACON     = 180,  /* No beacon xmit on VAP */
#endif
    IEEE80211_PARAM_VAP_COUNTRY_IE   = 181, /* 802.11d country ie per vap */
    IEEE80211_PARAM_VAP_DOTH         = 182, /* 802.11h per vap */
    IEEE80211_PARAM_STA_QUICKKICKOUT = 183, /* station quick kick out */
    IEEE80211_PARAM_AUTO_ASSOC       = 184,
    IEEE80211_PARAM_RXBUF_LIFETIME   = 185, /* lifetime of reycled rx buffers */
    IEEE80211_PARAM_2G_CSA           = 186, /* 2.4 GHz CSA is on/off */
    IEEE80211_PARAM_WAPIREKEY_USK = 187,
    IEEE80211_PARAM_WAPIREKEY_MSK = 188,
    IEEE80211_PARAM_WAPIREKEY_UPDATE = 189,
#if ATH_SUPPORT_IQUE
    IEEE80211_PARAM_RC_VIVO          = 190, /* Use separate rate control algorithm for VI/VO queues */
#endif
    IEEE80211_PARAM_CLR_APPOPT_IE    = 191,  /* Clear Cached App/OptIE */
    IEEE80211_PARAM_SW_WOW           = 192,   /* wow by sw */
    IEEE80211_PARAM_QUIET_PERIOD    = 193,
    IEEE80211_PARAM_QBSS_LOAD       = 194,
    IEEE80211_PARAM_RRM_CAP         = 195,
    IEEE80211_PARAM_WNM_CAP         = 196,
#if UMAC_SUPPORT_WDS
    IEEE80211_PARAM_ADD_WDS_ADDR    = 197,  /* add wds addr */
#endif
#ifdef __CARRIER_PLATFORM__
    IEEE80211_PARAM_PLTFRM_PRIVATE = 198, /* platfrom's private ioctl*/
#endif

#if UMAC_SUPPORT_VI_DBG
    /* Support for Video Debug */
    IEEE80211_PARAM_DBG_CFG            = 199,
    IEEE80211_PARAM_DBG_NUM_STREAMS    = 200,
    IEEE80211_PARAM_STREAM_NUM         = 201,
    IEEE80211_PARAM_DBG_NUM_MARKERS    = 202,
    IEEE80211_PARAM_MARKER_NUM         = 203,
    IEEE80211_PARAM_MARKER_OFFSET_SIZE = 204,
    IEEE80211_PARAM_MARKER_MATCH       = 205,
    IEEE80211_PARAM_RXSEQ_OFFSET_SIZE  = 206,
    IEEE80211_PARAM_RX_SEQ_RSHIFT      = 207,
    IEEE80211_PARAM_RX_SEQ_MAX         = 208,
    IEEE80211_PARAM_RX_SEQ_DROP        = 209,
    IEEE80211_PARAM_TIME_OFFSET_SIZE   = 210,
    IEEE80211_PARAM_RESTART            = 211,
    IEEE80211_PARAM_RXDROP_STATUS      = 212,
#endif    
    IEEE80211_PARAM_TDLS_DIALOG_TOKEN  = 213,  /* Dialog Token of TDLS Discovery Request */
    IEEE80211_PARAM_TDLS_DISCOVERY_REQ = 214,  /* Do TDLS Discovery Request */
    IEEE80211_PARAM_TDLS_AUTO_ENABLE   = 215,  /* Enable TDLS auto setup */
    IEEE80211_PARAM_TDLS_OFF_TIMEOUT   = 216,  /* Seconds of Timeout for off table : TDLS_OFF_TABLE_TIMEOUT */
    IEEE80211_PARAM_TDLS_TDB_TIMEOUT   = 217,  /* Seconds of Timeout for teardown block : TD_BLOCK_TIMEOUT */
    IEEE80211_PARAM_TDLS_WEAK_TIMEOUT  = 218,  /* Seconds of Timeout for weak peer : WEAK_PEER_TIMEOUT */
    IEEE80211_PARAM_TDLS_RSSI_MARGIN   = 219,  /* RSSI margin between AP path and Direct link one */
    IEEE80211_PARAM_TDLS_RSSI_UPPER_BOUNDARY= 220,  /* RSSI upper boundary of Direct link path */
    IEEE80211_PARAM_TDLS_RSSI_LOWER_BOUNDARY= 221,  /* RSSI lower boundary of Direct link path */
    IEEE80211_PARAM_TDLS_PATH_SELECT        = 222,  /* Enable TDLS Path Select bewteen AP path and Direct link one */
    IEEE80211_PARAM_TDLS_RSSI_OFFSET        = 223,  /* RSSI offset of TDLS Path Select */
    IEEE80211_PARAM_TDLS_PATH_SEL_PERIOD    = 224,  /* Period time of Path Select */
#if ATH_SUPPORT_IBSS_DFS
    IEEE80211_PARAM_IBSS_DFS_PARAM     = 225,
#endif 
    IEEE80211_PARAM_TDLS_TABLE_QUERY        = 236,  /* Print Table info. of AUTO-TDLS */
#if ATH_SUPPORT_IBSS_NETLINK_NOTIFICATION
    IEEE80211_PARAM_IBSS_SET_RSSI_CLASS     = 237,	
    IEEE80211_PARAM_IBSS_START_RSSI_MONITOR = 238,
    IEEE80211_PARAM_IBSS_RSSI_HYSTERESIS    = 239,
#endif
#ifdef ATH_SUPPORT_TxBF
    IEEE80211_PARAM_TXBF_AUTO_CVUPDATE = 240,       /* Auto CV update enable*/
    IEEE80211_PARAM_TXBF_CVUPDATE_PER = 241,        /* per theshold to initial CV update*/
#endif
    IEEE80211_PARAM_MAXSTA              = 242,
    IEEE80211_PARAM_RRM_STATS               =243,
    IEEE80211_PARAM_RRM_SLWINDOW            =244,
    IEEE80211_PARAM_MFP_TEST    = 245,
    IEEE80211_PARAM_SCAN_BAND   = 246,                /* only scan channels of requested band */
#if ATH_SUPPORT_FLOWMAC_MODULE
    IEEE80211_PARAM_FLOWMAC            = 247, /* flowmac enable/disable ath0*/
#endif
#if CONFIG_RCPI        
    IEEE80211_PARAM_TDLS_RCPI_HI     = 248,    /* RCPI params: hi,lo threshold and margin */
    IEEE80211_PARAM_TDLS_RCPI_LOW    = 249,    /* RCPI params: hi,lo threshold and margin */
    IEEE80211_PARAM_TDLS_RCPI_MARGIN = 250,    /* RCPI params: hi,lo threshold and margin */
    IEEE80211_PARAM_TDLS_SET_RCPI    = 251,    /* RCPI params: set hi,lo threshold and margin */
    IEEE80211_PARAM_TDLS_GET_RCPI    = 252,    /* RCPI params: get hi,lo threshold and margin */
#endif  
    IEEE80211_PARAM_TDLS_PEER_UAPSD_ENABLE  = 253, /* Enable TDLS Peer U-APSD Power Save feature */   
    IEEE80211_PARAM_TDLS_QOSNULL            = 254, /* Send QOSNULL frame to remote peer */
    IEEE80211_PARAM_STA_PWR_SET_PSPOLL      = 255,  /* Set ips_use_pspoll flag for STA */
    IEEE80211_PARAM_NO_STOP_DISASSOC        = 256,  /* Do not send disassociation frame on stopping vap */
#if UMAC_SUPPORT_IBSS
    IEEE80211_PARAM_IBSS_CREATE_DISABLE = 257,      /* if set, it prevents IBSS creation */
#endif
#if ATH_SUPPORT_WIFIPOS
    IEEE80211_PARAM_WIFIPOS_TXCORRECTION = 258,      /* Set/Get TxCorrection */
    IEEE80211_PARAM_WIFIPOS_RXCORRECTION = 259,      /* Set/Get RxCorrection */
#endif
#if UMAC_SUPPORT_CHANUTIL_MEASUREMENT
    IEEE80211_PARAM_CHAN_UTIL_ENAB      = 260,
    IEEE80211_PARAM_CHAN_UTIL           = 261,      /* Get Channel Utilization value (scale: 0 - 255) */
#endif /* UMAC_SUPPORT_CHANUTIL_MEASUREMENT */
    IEEE80211_PARAM_DBG_LVL_HIGH        = 262, /* Debug Level for specific VAP (upper 32 bits) */
    IEEE80211_PARAM_PROXYARP_CAP        = 263, /* Enable WNM Proxy ARP feature */
    IEEE80211_PARAM_DGAF_DISABLE        = 264, /* Hotspot 2.0 DGAF Disable feature */
    IEEE80211_PARAM_L2TIF_CAP           = 265, /* Hotspot 2.0 L2 Traffic Inspection and Filtering */
    IEEE80211_PARAM_WEATHER_RADAR_CHANNEL = 266, /* weather radar channel selection is bypassed */
    IEEE80211_PARAM_SEND_DEAUTH           = 267,/* for sending deauth while doing interface down*/
    IEEE80211_PARAM_WEP_KEYCACHE          = 268,/* wepkeys mustbe in first fourslots in Keycache*/
#if ATH_SUPPORT_WPA_SUPPLICANT_CHECK_TIME    
    IEEE80211_PARAM_REJOINT_ATTEMP_TIME   = 269, /* Set the Rejoint time */
#endif
    IEEE80211_PARAM_WNM_SLEEP           = 270,      /* WNM-Sleep Mode */
    IEEE80211_PARAM_WNM_BSS_CAP         = 271,
    IEEE80211_PARAM_WNM_TFS_CAP         = 272,
    IEEE80211_PARAM_WNM_TIM_CAP         = 273,
    IEEE80211_PARAM_WNM_SLEEP_CAP		= 274,
    IEEE80211_PARAM_RRM_DEBUG           = 275, /* RRM debugging parameter */
#if UMAC_SUPPORT_SMARTANTENNA
    IEEE80211_PARAM_SMARTANT_RETRAIN_THRESHOLD = 276,    /* number of packets required for retrain check */
    IEEE80211_PARAM_SMARTANT_RETRAIN_INTERVAL = 277,    /* periodic retrain interval */
    IEEE80211_PARAM_SMARTANT_RETRAIN_DROP = 278,    /* % change in goodput to tigger performance training */
    IEEE80211_PARAM_SMARTANT_MIN_GOODPUT_THRESHOLD = 279, /* Minimum Good put threshold to tigger performance training */
    IEEE80211_PARAM_SMARTANT_GOODPUT_AVG_INTERVAL =  280, /* Number of intervals Good put need to be averaged to use in performance training tigger */
#endif
    IEEE80211_PARAM_WNM_FMS_CAP = 281,
    IEEE80211_PARAM_PRINT_CHAIN_RSSI = 282,
    IEEE80211_PARAM_EXT_IFACEUP_ACS  = 283, /* Enable external auto channel selection entity
                                               at VAP init time */
    IEEE80211_PARAM_NOPBN               = 284, /* don't send push button notification */
    IEEE80211_PARAM_PARENT_IFINDEX      = 285, /* parent net_device ifindex for this VAP */
    IEEE80211_PARAM_SET_TXPWRADJUST     = 286,
	IEEE80211_PARAM_APONLY 				= 287, /* dynamic APONLY support */
	
#if CONFIG_WLAN_STA_ADDRTYPE
	IEEE80211_PARAM_SET_ADDRDETECT_TYPE = 288,	/* address type detect module need this parameter which is delivered from web page */
	IEEE80211_PARAM_SET_CRYPT_TYPE = 289,		/* address type detect module need know encrypt type */
#endif
};


//#define IEEE80211_IOC_P2P_FLUSH           616    /* IOCTL to flush P2P state */
#define IEEE80211_IOC_SCAN_REQ            624    /* IOCTL to request a scan */
//needed, below
#define IEEE80211_IOC_SCAN_RESULTS        IEEE80211_IOCTL_SCAN_RESULTS

#define IEEE80211_IOC_SSID                626    /* set ssid */
#define IEEE80211_IOC_MLME                IEEE80211_IOCTL_SETMLME
//#define IEEE80211_IOC_CHANNEL             628    /* set channel */

#define IEEE80211_IOC_WPA                 IEEE80211_PARAM_WPA    /* WPA mode (0,1,2) */
#define IEEE80211_IOC_AUTHMODE            IEEE80211_PARAM_AUTHMODE
#define IEEE80211_IOC_KEYMGTALGS          IEEE80211_PARAM_KEYMGTALGS    /* key management algorithms */
//#define IEEE80211_IOC_WPS_MODE            632    /* Wireless Protected Setup mode  */

#define IEEE80211_IOC_UCASTCIPHERS        IEEE80211_PARAM_UCASTCIPHERS    /* unicast cipher suites */
//#define IEEE80211_IOC_UCASTCIPHER         IEEE80211_PARAM_UCASTCIPHER    /* unicast cipher */
#define IEEE80211_IOC_MCASTCIPHER         IEEE80211_PARAM_MCASTCIPHER    /* multicast/default cipher */
//unused below
//#define IEEE80211_IOC_START_HOSTAP        636    /* Start hostap mode BSS */

#define IEEE80211_IOC_DROPUNENCRYPTED     637    /* discard unencrypted frames */
#define IEEE80211_IOC_PRIVACY             638    /* privacy invoked */
#define IEEE80211_IOC_OPTIE               IEEE80211_IOCTL_SETOPTIE    /* optional info. element */
#define IEEE80211_IOC_BSSID               640    /* GET bssid */


/* NB: require in+out parameters so cannot use wireless extensions, yech */
#define	IEEE80211_IOCTL_GETKEY		(SIOCDEVPRIVATE+3)
#define	IEEE80211_IOCTL_GETWPAIE	(SIOCDEVPRIVATE+4)
#define	IEEE80211_IOCTL_STA_STATS	(SIOCDEVPRIVATE+5)
//#define	IEEE80211_IOCTL_STA_INFO	(SIOCDEVPRIVATE+6)
//#define	SIOC80211IFCREATE		(SIOCDEVPRIVATE+7)
//#define	SIOC80211IFDESTROY	 	(SIOCDEVPRIVATE+8)
#define	IEEE80211_IOCTL_SCAN_RESULTS	(SIOCDEVPRIVATE+9)
//#define IEEE80211_IOCTL_RES_REQ         (SIOCDEVPRIVATE+10)
#define IEEE80211_IOCTL_GETMAC          (SIOCDEVPRIVATE+11)
//#define IEEE80211_IOCTL_CONFIG_GENERIC  (SIOCDEVPRIVATE+12)
//#define SIOCIOCTLTX99                   (SIOCDEVPRIVATE+13)
//#define IEEE80211_IOCTL_P2P_BIG_PARAM   (SIOCDEVPRIVATE+14)
//#define SIOCDEVVENDOR                   (SIOCDEVPRIVATE+15)    /* Used for ATH_SUPPORT_LINUX_VENDOR */
//#define	IEEE80211_IOCTL_GET_SCAN_SPACE  (SIOCDEVPRIVATE+16)

/* added APPIEBUF related definations */
enum{
    IEEE80211_APPIE_FRAME_BEACON     = 0,
    IEEE80211_APPIE_FRAME_PROBE_REQ  = 1,   
    IEEE80211_APPIE_FRAME_PROBE_RESP = 2,   
    IEEE80211_APPIE_FRAME_ASSOC_REQ  = 3,   
    IEEE80211_APPIE_FRAME_ASSOC_RESP = 4,   
    IEEE80211_APPIE_FRAME_TDLS_FTIE  = 5,   /* TDLS SMK_FTIEs */
    IEEE80211_APPIE_FRAME_AUTH       = 6,   
    IEEE80211_APPIE_NUM_OF_FRAME     = 7,   
    IEEE80211_APPIE_FRAME_WNM        = 8
};
struct ieee80211req_getset_appiebuf {
    u_int32_t app_frmtype; /*management frame type for which buffer is added*/
    u_int32_t app_buflen;  /*application supplied buffer length */
    u_int8_t  app_buf[];
};

/* the following definations are used by application to set filter 
 * for receiving management frames */
enum {
     IEEE80211_FILTER_TYPE_BEACON      =   0x1,  
     IEEE80211_FILTER_TYPE_PROBE_REQ   =   0x2,   
     IEEE80211_FILTER_TYPE_PROBE_RESP  =   0x4,   
     IEEE80211_FILTER_TYPE_ASSOC_REQ   =   0x8,   
     IEEE80211_FILTER_TYPE_ASSOC_RESP  =   0x10,   
     IEEE80211_FILTER_TYPE_AUTH        =   0x20,   
     IEEE80211_FILTER_TYPE_DEAUTH      =   0x40,   
     IEEE80211_FILTER_TYPE_DISASSOC    =   0x80,   
     IEEE80211_FILTER_TYPE_ACTION      =   0x100,   
     IEEE80211_FILTER_TYPE_ALL         =   0xFFF  /* used to check the valid filter bits */  
}; 

struct ieee80211req_set_filter {
      u_int32_t app_filterype; /* management frame filter type */
};


struct ieee80211_wlanconfig_nawds {
    u_int8_t num;
    u_int8_t mode;
    u_int8_t defcaps;
    u_int8_t override;
    u_int8_t mac[IEEE80211_ADDR_LEN];
    u_int8_t caps;
};

struct ieee80211_wlanconfig_wnm_bssmax {
    u_int16_t idleperiod;
};
struct ieee80211_wlanconfig_wds {
    u_int8_t mac_node[IEEE80211_ADDR_LEN];
    u_int8_t mac[IEEE80211_ADDR_LEN];
    u_int32_t flags;
};

struct ieee80211_wlanconfig_setmaxrate {
    u_int8_t mac[IEEE80211_ADDR_LEN];
    u_int8_t maxrate;
};
  
#define TFS_MAX_FILTER_LEN 50
#define TFS_MAX_TCLAS_ELEMENTS 2
#define TFS_MAX_SUBELEMENTS 2
#define TFS_MAX_REQUEST 2
#define TFS_MAX_RESPONSE 600

#define FMS_MAX_SUBELEMENTS    2
#define FMS_MAX_TCLAS_ELEMENTS 2
#define FMS_MAX_REQUEST        2
#define FMS_MAX_RESPONSE       2

typedef enum {
    IEEE80211_WNM_TFS_AC_DELETE_AFTER_MATCH = 0,
    IEEE80211_WNM_TFS_AC_NOTIFY = 1,
} IEEE80211_WNM_TFS_ACTIONCODE;

typedef enum {
    IEEE80211_WNM_TCLAS_CLASSIFIER_TYPE0 = 0,
    IEEE80211_WNM_TCLAS_CLASSIFIER_TYPE1 = 1,
    IEEE80211_WNM_TCLAS_CLASSIFIER_TYPE2 = 2,
    IEEE80211_WNM_TCLAS_CLASSIFIER_TYPE3 = 3,
    IEEE80211_WNM_TCLAS_CLASSIFIER_TYPE4 = 4,
} IEEE80211_WNM_TCLAS_CLASSIFIER;

typedef enum {
    IEEE80211_WNM_TCLAS_CLAS4_VERSION_4 = 4,
    IEEE80211_WNM_TCLAS_CLAS4_VERSION_6 = 6,
} IEEE80211_WNM_TCLAS_VERSION;

struct clas3 {
    u_int16_t filter_offset;
    u_int32_t filter_len;
    u_int8_t  filter_value[TFS_MAX_FILTER_LEN];
    u_int8_t  filter_mask[TFS_MAX_FILTER_LEN];
} __packed;

#ifndef IEEE80211_IPV4_LEN
#define IEEE80211_IPV4_LEN 4
#endif

#ifndef IEEE80211_IPV6_LEN
#define IEEE80211_IPV6_LEN 16
#endif


struct clas4_v4 {
    u_int8_t     version;
    u_int8_t     source_ip[IEEE80211_IPV4_LEN];
    u_int8_t     reserved1[IEEE80211_IPV6_LEN - IEEE80211_IPV4_LEN];
    u_int8_t     dest_ip[IEEE80211_IPV4_LEN];
    u_int8_t     reserved2[IEEE80211_IPV6_LEN - IEEE80211_IPV4_LEN];
    u_int16_t    source_port;
    u_int16_t    dest_port;
    u_int8_t     dscp;
    u_int8_t     protocol;
    u_int8_t     reserved;
    u_int8_t     reserved3[2];
}__packed;

struct clas4_v6 {
    u_int8_t     version;
    u_int8_t     source_ip[IEEE80211_IPV6_LEN];
    u_int8_t     dest_ip[IEEE80211_IPV6_LEN];
    u_int16_t    source_port;
    u_int16_t    dest_port;
    u_int8_t     dscp;
    u_int8_t     next_header;
    u_int8_t     flow_label[3];
}__packed;


struct tfsreq_tclas_element {
    u_int8_t classifier_type;
    u_int8_t classifier_mask;
    u_int8_t priority;
    union {
        struct clas3 clas3;
        union {
            struct clas4_v4 clas4_v4;
            struct clas4_v6 clas4_v6;
        } clas4;
    } clas;
} __packed;

struct tfsreq_subelement {
    u_int32_t num_tclas_elements;
    u_int8_t tclas_processing;
    struct tfsreq_tclas_element tclas[TFS_MAX_TCLAS_ELEMENTS];
} __packed;

struct ieee80211_wlanconfig_wnm_tfs_req {
    u_int8_t tfsid;
    u_int8_t actioncode;
    u_int8_t num_subelements;
    struct tfsreq_subelement subelement[TFS_MAX_SUBELEMENTS];
} __packed;



struct ieee80211_wlanconfig_wnm_tfs {
    u_int8_t num_tfsreq;
    struct ieee80211_wlanconfig_wnm_tfs_req  tfs_req[TFS_MAX_REQUEST];
} __packed;

struct tfsresp_element {
	u_int8_t tfsid;
    u_int8_t status;
} __packed;

struct ieee80211_wnm_tfsresp {
    u_int8_t num_tfsresp;
    struct tfsresp_element  tfs_resq[TFS_MAX_RESPONSE];
} __packed;

typedef struct  ieee80211_wnm_rate_identifier_s {
    u_int8_t mask;
    u_int8_t mcs_idx;
    u_int16_t rate;
}__packed ieee80211_wnm_rate_identifier_t;

struct fmsreq_subelement {
    u_int8_t del_itvl;
    u_int8_t max_del_itvl;
    u_int8_t tclas_processing;
    u_int32_t num_tclas_elements;
    ieee80211_wnm_rate_identifier_t rate_id;
    struct tfsreq_tclas_element tclas[FMS_MAX_TCLAS_ELEMENTS];
} __packed;

struct ieee80211_wlanconfig_wnm_fms_req {
    u_int8_t fms_token;
    u_int8_t num_subelements;
    struct fmsreq_subelement subelement[FMS_MAX_SUBELEMENTS];
} __packed;

struct ieee80211_wlanconfig_wnm_fms {
    u_int8_t num_fmsreq;
    struct ieee80211_wlanconfig_wnm_fms_req  fms_req[FMS_MAX_REQUEST];
} __packed;

enum {
    IEEE80211_WNM_TIM_HIGHRATE_ENABLE = 0x1,
    IEEE80211_WNM_TIM_LOWRATE_ENABLE = 0x2,
};

struct ieee80211_wlanconfig_wnm_tim {
    u_int8_t interval;
    u_int8_t enable_highrate;
    u_int8_t enable_lowrate;
} __packed;

struct ieee80211_wlanconfig_wnm {
    union {
        struct ieee80211_wlanconfig_wnm_bssmax bssmax;
        struct ieee80211_wlanconfig_wnm_tfs tfs;
        struct ieee80211_wlanconfig_wnm_fms fms;
        struct ieee80211_wlanconfig_wnm_tim tim;
    } data;
} __packed;


/* generic structure to support sub-ioctl due to limited ioctl */
typedef enum {
    IEEE80211_WLANCONFIG_NOP,
    IEEE80211_WLANCONFIG_NAWDS_SET_MODE,
    IEEE80211_WLANCONFIG_NAWDS_SET_DEFCAPS,
    IEEE80211_WLANCONFIG_NAWDS_SET_OVERRIDE,
    IEEE80211_WLANCONFIG_NAWDS_SET_ADDR,
    IEEE80211_WLANCONFIG_NAWDS_CLR_ADDR,
    IEEE80211_WLANCONFIG_NAWDS_GET,
    IEEE80211_WLANCONFIG_WNM_SET_BSSMAX,
    IEEE80211_WLANCONFIG_WNM_GET_BSSMAX,
    IEEE80211_WLANCONFIG_WNM_TFS_ADD,
    IEEE80211_WLANCONFIG_WNM_TFS_DELETE,
    IEEE80211_WLANCONFIG_WNM_FMS_ADD_MODIFY,
    IEEE80211_WLANCONFIG_WNM_SET_TIMBCAST,
    IEEE80211_WLANCONFIG_WNM_GET_TIMBCAST,
    IEEE80211_WLANCONFIG_WDS_ADD_ADDR,
    IEEE80211_WLANCONFIG_SET_MAX_RATE,
    
#if	TP_FEATURE_GET_WIRELESS_STATE   
    IEEE80211_WLANCONFIG_ACS_GET,			/* add for auto channel select,  by yuyi, 19Aug11 */
    IEEE80211_WLANCONFIG_WDS_STATE_GET,		/* add for wds state , by yuyi, 19Aug11 */
    IEEE80211_WLANCONFIG_STA_KEYSTATE,		/* add for key state, by yuyi, 19Aug11 */
    IEEE80211_WLANCONFIG_TRAFFIC,			/* add for wlan traffic, by yuyi, 19Aug11 */
#endif 

	IEEE80211_WLANCONFIG_FAILEDRXBUF_GET,/* by Chen-Lifeng, 18 Jun, 11 */
	
#if CONFIG_WLAN_STA_ADDRTYPE
	IEEE80211_WLANCONFIG_SET_WAN_MAC,		/* add for adder type detect */
	IEEE80211_WLANCONFIG_GET_ADDR_TYPE,		/* add for adder type detect */
#endif
} IEEE80211_WLANCONFIG_CMDTYPE;

#if	TP_FEATURE_GET_WIRELESS_STATE
/* add for wlan statistic,  by yuyi, 19Aug11 */
struct ieee80211req_traffic_info{
u_int32_t       isi_rx_packets;
u_int32_t       isi_tx_packets;
u_int32_t       isi_rx_bytes;
u_int32_t       isi_tx_bytes;
};
#endif

typedef enum {
    IEEE80211_WLANCONFIG_OK          = 0,
    IEEE80211_WLANCONFIG_FAIL        = 1,
} IEEE80211_WLANCONFIG_STATUS;

struct ieee80211_wlanconfig {
    IEEE80211_WLANCONFIG_CMDTYPE cmdtype;  /* sub-command */
    IEEE80211_WLANCONFIG_STATUS status;     /* status code */
    union {
#if UMAC_SUPPORT_NAWDS
        struct ieee80211_wlanconfig_nawds nawds;
#endif
#if UMAC_SUPPORT_WNM
        struct ieee80211_wlanconfig_wnm wnm;
#endif
#if UMAC_SUPPORT_WDS
        struct ieee80211_wlanconfig_wds wds;
#endif

#if	TP_FEATURE_GET_WIRELESS_STATE
#if UMAC_SUPPORT_ACS
       int32_t acs_channel;	/* support for ACS, by yuyi, 19Aug11 */
#endif 
       int32_t wds_state; /* add for wds state, by yuyi, 19Aug11 */

       int32_t key_state; /* add for key state, by yuyi, 19Aug11 */

       struct ieee80211req_traffic_info traffic_info; /* add for wlan traffic, by yuyi, 19Aug11 */
#endif

#if CONFIG_WLAN_STA_ADDRTYPE
		u_int8_t wan_mac[IEEE80211_ADDR_LEN]; /* WAN MAC */
		u_int8_t addr_type;					  /* Default addr type */
#endif
    } data;

    struct ieee80211_wlanconfig_setmaxrate smr;
};

/* kev event_code value for Atheros IEEE80211 events */
enum {
    IEEE80211_EV_SCAN_DONE,
    IEEE80211_EV_CHAN_START,
    IEEE80211_EV_CHAN_END,
    IEEE80211_EV_RX_MGMT,
    IEEE80211_EV_P2P_SEND_ACTION_CB,
    IEEE80211_EV_IF_RUNNING,
#if UMAC_SUPPORT_WNM
    IEEE80211_EV_IF_WNM_TFS_REQ,
#endif
    IEEE80211_EV_IF_NOT_RUNNING,
	IEEE80211_EV_AUTH_COMPLETE_AP,
	IEEE80211_EV_ASSOC_COMPLETE_AP,
	IEEE80211_EV_DEAUTH_COMPLETE_AP,
	IEEE80211_EV_AUTH_IND_AP,
	IEEE80211_EV_AUTH_COMPLETE_STA,
	IEEE80211_EV_ASSOC_COMPLETE_STA,
	IEEE80211_EV_DEAUTH_COMPLETE_STA,
	IEEE80211_EV_DISASSOC_COMPLETE_STA,
	IEEE80211_EV_AUTH_IND_STA,
	IEEE80211_EV_DEAUTH_IND_STA,
	IEEE80211_EV_ASSOC_IND_STA,
	IEEE80211_EV_DISASSOC_IND_STA,
	IEEE80211_EV_DEAUTH_IND_AP,
	IEEE80211_EV_DISASSOC_IND_AP,
	IEEE80211_EV_WAPI,
};

#endif /* __linux__ */

#define IEEE80211_VAP_PROFILE_NUM_ACL 64
#define IEEE80211_VAP_PROFILE_MAX_VAPS 8

#endif /* _NET80211_IEEE80211_IOCTL_H_ */
