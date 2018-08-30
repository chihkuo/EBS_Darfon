/*
 * @file Member ID Definition of Group WEBUI.
 *
 * Moderator: SerenaWang
 * Group ID: 0x00200000
 */

#ifndef _CSID_WEBUI_H
#define _CSID_WEBUI_H

#include "csid_gid.h"

#define _CFG_WEBUI(x)         (CSID_GIDC_WEBUI|(x))
#define _STA_WEBUI(x)         (CSID_GIDS_WEBUI|(x))

#define FW_QUERY_STOP			    0x00
#define FW_QUERYING 			    0x01
#define FW_QUERY_DONE 		    0x02
#define FW_DOWNLOADING		    0x03
#define FW_DOWNLOAD_FAIL	    0x04
#define FW_DOWNLOAD_DONE	    0x05
#define FW_UPGRADING			    0x06
#define FW_UPGRADE_DONE		    0x07
#define FW_UPGRADE_FAIL		    0x08

/* Firmware Upgrade Status */
#define FWUPG_ACTION_NONE       0x00
#define FWUPG_DOWNLOAD_START    0x01
#define FWUPG_DOWNLOAD_DONE     0x02
#define FWUPG_DOWNLOAD_FAIL     0x03
#define FWUPG_UPGRADE_START     0x04
#define FWUPG_UPGRADE_DONE      0x05
#define FWUPG_UPGRADE_FAIL      0x06

/* Configuration Items */

//==========================================================
// 0x0000~0x0FFF: Reserved.
//==========================================================

#define CSID_C_WEBUI_WIZARD_EXECED            _CFG_WEBUI(0x1001)	//T=u8, D=0, 1:yes 0:no
#define CSID_C_WEBUI_WIZARD_TMP_LANIP         _CFG_WEBUI(0x1002)	//T=ipv4, D={192.168.123.254}, lan ip
#define CSID_C_WEBUI_FW_QUERY_AUTO_ENABLE     _CFG_WEBUI(0x1003)  //T=u8, enable auto fw query
#define CSID_C_WEBUI_FW_QUERY_LAST_TIME       _CFG_WEBUI(0x1004)  //T=u32, fw query's last ntp time
#define CSID_C_WEBUI_FW_QUERY_TIME            _CFG_WEBUI(0x1005)  //T=u32, auto fw query's time(sec)
#define CSID_C_WEBUI_FW_QUERY_REGION_NAME     _CFG_WEBUI(0x1006)  //T=str, the region name of fw query
#define CSID_C_WEBUI_FW_QUERY_VER             _CFG_WEBUI(0x1007)  //T=str, the fw version of fw query
#define CSID_C_WEBUI_FW_QUERY_VER_MATCH       _CFG_WEBUI(0x1008)  //T=u32, the matching flag between current fw version and fw query version
#define CSID_C_WEBUI_FW_QUERY_DATE            _CFG_WEBUI(0x1009)  //T=str, the release date of fw query
#define CSID_C_WEBUI_FW_QUERY_DW_URL          _CFG_WEBUI(0x100a)  //T=str, the fw download url of fw query
#define	CSID_C_WEBUI_FAKE_WEPKEYLENGTH 	      _CFG_WEBUI(0x100b)	//T=u8, D=0, D-link DWR927 WEP Key Lengh is fake
#define	CSID_C_WEBUI_PASS_CHANGED     	      _CFG_WEBUI(0x100c)	//T=u8, D=0, Zyxel change password flag
#define	CSID_C_WEBUI_QOSC_WAN_IF              _CFG_WEBUI(0x100d)	//T=u8, D=0, Nobrand UI use
#define	CSID_C_WEBUI_DHCP_FLAG 	              _CFG_WEBUI(0x100e)	//T=u8, D=0, DHCP SERVER UI flag
#define	CSID_C_WEBUI_VPNPPTP_ENABLE 	        _CFG_WEBUI(0x100f)	//T=u8, D=0, VPN PPTP total switch
#define	CSID_C_WEBUI_VPNL2TP_ENABLE 	        _CFG_WEBUI(0x1010)	//T=u8, D=0, VPN L2TP total switch
#define CSID_C_WEBUI_VLAN_GROUP_NAME          _CFG_WEBUI(0x1011)  //T=str, the Zyxel VLAN Group name,reserve 8
#define	CSID_C_WEBUI_ROUTER_TYPE    	        _CFG_WEBUI(0x1012)	//T=u8, D=0, 0:wireless router 1:AP router 2:wired router
#define CSID_C_WEBUI_FW_QUERY_BOOT_TYPE       _CFG_WEBUI(0x1013)  //T=u8, FW auto upgrade type when boot
#define CSID_C_WEBUI_FW_UPG_RULE_SCHE_NO      _CFG_WEBUI(0x1014)  //T=u8, FW auto upgrade by schedule
#define CSID_C_WEBUI_VLAN_PORT_NUM            _CFG_WEBUI(0x1019)  //T=u8, D=5, AMIT VLAN PORT NUMBER
#define CSID_C_WEBUI_VLAN_VAP_NUM             _CFG_WEBUI(0x101a)  //T=u8, D=8, AMIT VLAN VAP NUMBER
#define CSID_C_WEBUI_ALLOW_IP                 _CFG_WEBUI(0x101b)  //T=ipv4, allow ip
#define CSID_C_WEBUI_AP_LIST_VER              _CFG_WEBUI(0x101c)  //T=str, D-Link GK team record dif.htm ver
#define CSID_C_WEBUI_VPN_CMD                  _CFG_WEBUI(0x101d)	//T=u8, D=0, VPN scenario 0:S to S 1:S to H 2:H to S 3:H to H 4:Dynamic-32
#define CSID_C_WEBUI_VPN_REMOTE_RADIO         _CFG_WEBUI(0x103d)  //T=u8, D=0, D-link 640L VPN REMOTE,SITE TO SITE RADIO
#define CSID_C_WEBUI_WIFI_FILTER_DESC         _CFG_WEBUI(0x103e)  //T=str, IODATA WIFI MAC Filter ruse Description-32
#define CSID_C_WEBUI_LANSETMASK_GROUPNUM      _CFG_WEBUI(0x105e)  //T=u8, D=0, AMIT VLAN TAG_INDEX-7
#define CSID_C_WEBUI_DHCP_RECORD_RULE         _CFG_WEBUI(0x1065)  //T=u32, for DHCP server-8
#define CSID_C_WEBUI_QOS_RESOURCE             _CFG_WEBUI(0x106d)  //T=u32, D=0, AMIT QoS number-64
#define CSID_C_WEBUI_QOS_SERVICE              _CFG_WEBUI(0x10ad)  //T=u32, D=0, AMIT QoS number-64
#define CSID_C_WEBUI_L2TPCLI_TUNNEL_MAX       _CFG_WEBUI(0x10ed)  //T=u32, D=22, AMIT VPN
#define CSID_C_WEBUI_PPTPCLI_TUNNEL_MAX       _CFG_WEBUI(0x10ee)  //T=u32, D=22, AMIT VPN
#define CSID_C_WEBUI_GRE_TUNNEL_MAX           _CFG_WEBUI(0x10ef)  //T=u32, D=32, AMIT VPN
#define CSID_C_WEBUI_SSLVPNEXT_TUNNEL_MAX     _CFG_WEBUI(0x10f0)  //T=u32, D=32, AMIT VPN
#define CSID_C_WEBUI_SSLVPNUSER_TUNNEL_MAX    _CFG_WEBUI(0x10f1)  //T=u32, D=34, AMIT VPN
#define CSID_C_WEBUI_SSLVPNUSER_S_TUNNEL_MAX  _CFG_WEBUI(0x10f2)  //T=u32, D=2, AMIT VPN
#define CSID_C_WEBUI_PORTBASE_DHCPMOD         _CFG_WEBUI(0x10f3)  //T=u32, D=1, AMIT VLAN-5
#define CSID_C_WEBUI_WAN_TMP_OPMODE           _CFG_WEBUI(0x10f8)	//T=str, save Operation Mode
#define CSID_C_WEBUI_TR069_WAN                _CFG_WEBUI(0x10f9)	//T=str, AMIT tr069 wan
#define CSID_C_WEBUI_FIROPTIONS_NUM           _CFG_WEBUI(0x10fa)  //T=u8, D=0, AMIT....-5
/*缺1*/
#define CSID_C_WEBUI_UI_FLAG                  _CFG_WEBUI(0x1100)	//T=u8, D=0, for UI save flags-16
#define CSID_C_WEBUI_IO_RULE_RECORD_RULE      _CFG_WEBUI(0x1110)  //T=u32, record the rule for I/O management-16
#define CSID_C_WEBUI_TEMP_IP                  _CFG_WEBUI(0x1120)  //T=ipv4, for any UI save temporary IP Address-16
#define CSID_C_WEBUI_3G_COUNTRY               _CFG_WEBUI(0x1121)  //T=u32, D=0, 3G country-8
#define CSID_C_WEBUI_3G_COUNTRY_SIMA          _CFG_WEBUI(0x1129)  //T=u32, D=0, 3G country-sima-8
#define CSID_C_WEBUI_3G_COUNTRY_SIMB          _CFG_WEBUI(0x1131)  //T=u32, D=0, 3G country-samb-8
#define CSID_C_WEBUI_3G_PROVIDER              _CFG_WEBUI(0x1139)  //T=u32, D=0, 3G provider-8
#define CSID_C_WEBUI_3G_PROVIDER_SIMA         _CFG_WEBUI(0x1141)  //T=u32, D=0, 3G provider-sima-8
#define CSID_C_WEBUI_3G_PROVIDER_SIMB         _CFG_WEBUI(0x1149)  //T=u32, D=0, 3G provider-simb-8
#define CSID_C_WEBUI_3G_NETWORK               _CFG_WEBUI(0x1151)  //T=u32, D=0, 3G network-8
#define CSID_C_WEBUI_3G_NETWORK_SIMA          _CFG_WEBUI(0x1159)  //T=u32, D=0, 3G network-sima-8
#define CSID_C_WEBUI_3G_NETWORK_SIMB          _CFG_WEBUI(0x1161)  //T=u32, D=0, 3G network-simb-8
#define CSID_C_WEBUI_LOCALCERT_RECORD_RULE    _CFG_WEBUI(0x1170)  //T=u32, for Local Certificate List-32
#define CSID_C_WEBUI_EOIPTNAM_RECORD_RULE     _CFG_WEBUI(0x1190)  //T=u32, for EOIPTNAM List-32
#define CSID_C_WEBUI_SSLCC_RECORD_RULE        _CFG_WEBUI(0x11b0)  //T=u32, for SSLCC List-16
#define CSID_C_WEBUI_SSLCC_TUNNEL_MAX         _CFG_WEBUI(0x11c0)  //T=u32, D=2, for SSLCC tunel
#define CSID_C_WEBUI_PORTBASE_VID             _CFG_WEBUI(0x11c1)  //T=u32, D=1, for VLAN-5
#define CSID_C_WEBUI_PORTBASE_WANVID          _CFG_WEBUI(0x11c6)  //T=u32, D=1, for VLAN-5
#define CSID_C_WEBUI_3G_MULTI_NETWORKNAME     _CFG_WEBUI(0x11cb)	//T=str, Multitech 3g network name
/*從11cc到11cf 缺4*/

/* 記錄max rule的CSID, 保留11d0到1200 可加48組, 由此加 */
#define CSID_C_WEBUI_ROUTINGDIP_MAX_RULE      _CFG_WEBUI(0x11d0)  //T=u32, D=64, UI max rule number
#define CSID_C_WEBUI_ROUTINGOSPF_MAX_RULE     _CFG_WEBUI(0x11d1)  //T=u32, D=32, UI max rule number
#define CSID_C_WEBUI_RBGP_SNET_MAX_RULE       _CFG_WEBUI(0x11d2)  //T=u32, D=32, UI max rule number
#define CSID_C_WEBUI_ROUTINGBGP_MAX_RULE      _CFG_WEBUI(0x11d3)  //T=u32, D=32, UI max rule number
#define CSID_C_WEBUI_LOADBALANCE_MAX_RULE     _CFG_WEBUI(0x11d4)  //T=u32, D=64, UI max rule number
/*從11d5到11ff 缺42*/
#define CSID_C_WEBUI_UI_SETTING               _CFG_WEBUI(0x1200)	//T=str, for UI save settings
#define CSID_C_WEBUI_VLAN_CHANGE              _CFG_WEBUI(0x1201)	//T=u32, D=0, for nobrand VLAN
#define CSID_C_WEBUI_HTTPS_RADMIN_ENABLE      _CFG_WEBUI(0x1202)	//T=u32, D=1, AMIT HTTPS_REMOTE_ADMIN-5
#define CSID_C_WEBUI_ADMIN_ENABLE             _CFG_WEBUI(0x1207)	//T=u32, D=0, AMIT ADMIN-5
#define CSID_C_WEBUI_WLANAPID_ENABLE          _CFG_WEBUI(0x120c)	//T=u32, D=0, AMIT wlanpid-8
#define CSID_C_WEBUI_DI_PROFILE_RECORD        _CFG_WEBUI(0x1214)	//T=u32, Digital Input (DI)-10
#define CSID_C_WEBUI_DO_PROFILE_RECORD        _CFG_WEBUI(0x121e)	//T=u32, Digital Input (DO)-10
#define CSID_C_WEBUI_REMOTE_ADMIN_ENABLE      _CFG_WEBUI(0x1228)	//T=u8, rule enable, 1:yes 0:no-4
#define CSID_C_WEBUI_APCLI_PROFILE_RECORD     _CFG_WEBUI(0x126c)	//T=u32, for profile list-16
#define CSID_C_WEBUI_APCLI2_PROFILE_RECORD    _CFG_WEBUI(0x127c)	//T=u32, for profile list-16
/* ============ 
    Ex : 0x127c + 0x10 = 0x128c(下一筆開始的位址) ; C_WEBUI_APCLI2_PROFILE_RECORD 用 0x127c ~ 128b 
============ */
/* ============ 上面零星的用完由此開始加, ps.下一筆是0x128c ============ */

#define	CSID_C_WEBUI_LOGTYPE          	      _CFG_WEBUI(0x1300)	//T=str, D="0000", Log Type for UI
#define	CSID_C_WEBUI_WIZARD_LANG          	  _CFG_WEBUI(0x1400)	//T=u8, D=0, D-link web wizard language select
#define	CSID_C_WEBUI_PICTURE_AUTH        	  _CFG_WEBUI(0x1500)	//T=u8, D=0, D-link picture authentication when login
#define	CSID_C_WEBUI_LOCK_WPS_PIN         	  _CFG_WEBUI(0x1600)	//T=u8, D=0, D-link lock wps button
#define	CSID_C_WEBUI_WEP_KEY_LEN      	      _CFG_WEBUI(0x1700)	//T=u8, D=0, D-link,[0] is 2.4G,[1] is 5G,[2] is guest zone 2.4G, [3] is guest zone 5G
#define	CSID_C_WEBUI_FAKE_CHANNEL      	      _CFG_WEBUI(0x1800)	//T=u8, D=0, D-link 840L channel 11 12 13 is fake

/* each 64(or 32) rules, 2015/7/9起 CSID命名改成 C_WEBUI_???_RECORD */
#define CSID_C_WEBUI_VPNLIST_RECORD_RULE	  _CFG_WEBUI(0x2000)	//T=u32, VPN list
#define CSID_C_WEBUI_VPNPPTPACC_RECORD_RULE	  _CFG_WEBUI(0x2040)	//T=u32, C_PPTPSERV_ACCOUNT_USER list
#define CSID_C_WEBUI_VPNPPTPCLI_RECORD_RULE	  _CFG_WEBUI(0x2080)	//T=u32, C_PPTP_CLI_TUNLNAME list
#define CSID_C_WEBUI_VPNL2TPACC_RECORD_RULE	  _CFG_WEBUI(0x20c0)	//T=u32, C_L2TPSERV_ACCOUNT_USER list
#define CSID_C_WEBUI_VPNL2TPNAM_RECORD_RULE	  _CFG_WEBUI(0x2100)	//T=u32, C_L2TP_CLI_TUNLNAME list
#define CSID_C_WEBUI_GRETNAM_RECORD_RULE	  _CFG_WEBUI(0x2140)	//T=u32, C_GRE_TNAME list
#define CSID_C_WEBUI_LOADBALANCE_RECORD_RULE  _CFG_WEBUI(0x2180)	//T=u32, load balance policy list
#define CSID_C_WEBUI_FIRWALLPRO_RECORD_RULE   _CFG_WEBUI(0x21c0)	//T=u32, firewall profile list
#define CSID_C_WEBUI_URLBLOCK_RECORD_RULE     _CFG_WEBUI(0x2200)	//T=u32, C_URLBLOCK_RULE_URL list
#define CSID_C_WEBUI_PDUMG_OUTLE_RECORD       _CFG_WEBUI(0x2240)	//T=u32, PDUMG_OUTLE
#define CSID_C_WEBUI_URLBLOCKEXT_RECORD_RULE  _CFG_WEBUI(0x2280)	//T=u32, C_URLBLOCK_FILE_EXTENSION_RULE_LIST
#define CSID_C_WEBUI_SSLVPNAPP_RECORD_RULE    _CFG_WEBUI(0x22c0)	//T=u32, C_SSLVPN_APPLICATION_NAME
#define CSID_C_WEBUI_SSLVPNEXT_RECORD_RULE    _CFG_WEBUI(0x2300)	//T=u32, C_SSLVPN_EXTENDER_NAME
#define CSID_C_WEBUI_SSLVPNUSER_RECORD_RULE   _CFG_WEBUI(0x2340)	//T=str, C_SSL_VPN_USER_TYPE
#define CSID_C_WEBUI_POEPORTMG_PORT_RECORD    _CFG_WEBUI(0x2380)	//T=u32, PoE Port Management list
#define CSID_C_WEBUI_VIRTUALCOM_RECORD_RULE   _CFG_WEBUI(0x23c0)	//T=u32, C_VCOMPUTER_GLOBAL_IP
#define CSID_C_WEBUI_URLDESPORT_RULE          _CFG_WEBUI(0x2400)	//T=u32, URLDESPORT rule
#define CSID_C_WEBUI_WEBCONTENT_RECORD_RULE   _CFG_WEBUI(0x2440)  //T=u32, WEBCONTENT rule
#define CSID_C_WEBUI_PKDPORT_RECORD_RULE      _CFG_WEBUI(0x2480)	//T=u32, PACKET FILTERS DES Port
#define CSID_C_WEBUI_PKWKNOW_RECORD_RULE      _CFG_WEBUI(0x24c0)	//T=u32, C_PKFILTER_WELLKNOWSERVICE_RULE
#define CSID_C_WEBUI_SPAPTPORT_RECORD_RULE    _CFG_WEBUI(0x2500)	//T=u32, C_SPAP_TRIGGER_PORT
#define CSID_C_WEBUI_ROUTINGDIP_RECORD_RULE   _CFG_WEBUI(0x2540)	//T=u32, C_ROUTING_STATIC_DEST_IP
#define CSID_C_WEBUI_ROUTINGBGP_RECORD_RULE   _CFG_WEBUI(0x2580)	//T=u32, C_ROUTING_BGP_RULE_IP
#define CSID_C_WEBUI_ROUTINGOSPF_RECORD_RULE  _CFG_WEBUI(0x25c0)	//T=u32, C_ROUTING_OSPF_RULE_SUBNET
#define CSID_C_WEBUI_TRUCACERT_RECORD_RULE    _CFG_WEBUI(0x2600)	//T=u32, for Trusted CA Certificate List
#define CSID_C_WEBUI_TRUCLICERT_RECORD_RULE   _CFG_WEBUI(0x2640)	//T=u32, for Trusted Client Certificate List
#define CSID_C_WEBUI_TAGBASE_RECORD_RULE      _CFG_WEBUI(0x2680)  //T=u32, for tagbase vlan list
#define CSID_C_WEBUI_QOS_RECORD_RULE          _CFG_WEBUI(0x26c0)  //T=u32, for QoS
#define CSID_C_WEBUI_EXTSERV_SERV_NAME        _CFG_WEBUI(0x2700)  //T=u32, from grouping`s C_EXTSERV_SERV_NAME
#define CSID_C_WEBUI_TAGBASE_DHCPSERVER       _CFG_WEBUI(0x2740)  //T=u32, for vlan dhcp server
#define CSID_C_WEBUI_PRO_SIMA_RECORD_RULE     _CFG_WEBUI(0x2780)  //T=u32, for profils sim A-32
#define CSID_C_WEBUI_PRO_SIMB_RECORD_RULE     _CFG_WEBUI(0x27a0)  //T=u32, for profils sim B-32
#define CSID_C_WEBUI_L7_RECORD_RULE           _CFG_WEBUI(0x27c0)  //T=u32, record L7 index-32
#define CSID_C_WEBUI_RS232_REMOTE_RECORD      _CFG_WEBUI(0x27e0)  //T=u32, record Modbus Remote Slave List-64
#define CSID_C_WEBUI_PORTFW_PROTOCOL          _CFG_WEBUI(0x2820)  //T=u8,  D=0, PORTFW_PROTOCOL-64
#define CSID_C_WEBUI_SNMPPRI_RECORD           _CFG_WEBUI(0x2860)  //T=u32, record SNMPPRI-64
#define CSID_C_WEBUI_STATIC_ARP_RECORD        _CFG_WEBUI(0x28a0)  //T=u32, record ARP-64
#define CSID_C_WEBUI_MANAG_EVTS_RECORD        _CFG_WEBUI(0x28e0)  //T=u32, record Managing Event Configuration-64
#define CSID_C_WEBUI_GNSS_RM_RECORD           _CFG_WEBUI(0x2920)  //T=u32, record gnss-32
#define CSID_C_WEBUI_RBGP_SNET_RECORD_RULE    _CFG_WEBUI(0x2940)  //T=u32, ROUTINGBGP_SUBNET_RECORD_RULE-32
#define CSID_C_WEBUI_MNG_EVENT_RECORD_RULE    _CFG_WEBUI(0x2960)  //T=u32, Managing Event List-32
#define CSID_C_WEBUI_NOT_EVENT_RECORD_RULE    _CFG_WEBUI(0x2980)  //T=u32, Notified Event List-32
#define CSID_C_WEBUI_DATA_UNAME_RECORD_RULE   _CFG_WEBUI(0x29a0)  //T=u32, for C_SYS_DATA_USAGE_NAME-32
#define CSID_C_WEBUI_DATALOGPXY_RECORD        _CFG_WEBUI(0x29e0)  //T=u32, record data logpxy-64
#define CSID_C_WEBUI_PKSPORT_RECORD_RULE      _CFG_WEBUI(0x2a20)  //T=u32, PACKET FILTERS DES Port-64
#define CSID_C_WEBUI_PKSWKNOW_RECORD_RULE     _CFG_WEBUI(0x2a60)  //T=u32, C_PKFILTER_WELLKNOWSERVICE_RULE-64
#define CSID_C_WEBUI_DATALOG_RECORD           _CFG_WEBUI(0x2aa0)  //T=u32, record data log-64
#define CSID_C_WEBUI_NOTIFY_EVTS_RECORD       _CFG_WEBUI(0x2ae0)  //T=u32, record notify event-64
#define CSID_C_WEBUI_APJOB_SCHEDULE_RECOR     _CFG_WEBUI(0x2b00)  //T=u32, record AP Job Scheduling Rule-32

/* For UI Control on Event Management (0x4500 ~ 0x45FF) */
#define CSID_C_WEBUI_EVM_SUPPORT_POWER        _CFG_WEBUI(0x4500)    //T=u32, D=0, Support Power Control on Event Management UI, 1:yes 0:no
#define CSID_C_WEBUI_EVM_SUPPORT_DI           _CFG_WEBUI(0x4501)    //T=u32, D=0, Support DI Control on Event Management UI, 1:yes 0:no
#define CSID_C_WEBUI_EVM_SUPPORT_DO           _CFG_WEBUI(0x4502)    //T=u32, D=0, Support DO Control on Event Management UI, 1:yes 0:no
#define CSID_C_WEBUI_EVM_SUPPORT_SMS          _CFG_WEBUI(0x4503)    //T=u32, D=0, Support SMS on Event Management UI, 1:yes 0:no
#define CSID_C_WEBUI_EVM_SUPPORT_WIFI         _CFG_WEBUI(0x4504)    //T=u32, D=0, Support WiFi on Event Management UI, 1:yes 0:no
#define CSID_C_WEBUI_EVM_SUPPORT_3G_LTE       _CFG_WEBUI(0x4505)    //T=u32, D=0, Support 3G/LTE on Event Management UI, 1:yes 0:no
#define CSID_C_WEBUI_EVM_SUPPORT_MODBUS       _CFG_WEBUI(0x4506)    //T=u32, D=0, Support Modbus Control on Event Management UI, 1:yes 0:no
#define CSID_C_WEBUI_EVM_SUPPORT_POE          _CFG_WEBUI(0x4507)    //T=u32, D=0, Support Power over Ethernet Control on Event Management UI, 1:yes 0:no
#define CSID_C_WEBUI_EVM_SUPPORT_DUAL_SIM     _CFG_WEBUI(0x4508)    //T=u32, D=0, Support Dual SIM Control on Event Management UI, 1:yes 0:no

/* each 80 or 128 or 256 rules */
#define CSID_C_WEBUI_PKSIP_RECORD_RULE        _CFG_WEBUI(0x5000)	//T=u32, PACKET FILTERS Source IP list-80
#define CSID_C_WEBUI_PKDIP_RECORD_RULE        _CFG_WEBUI(0x5050)	//T=u32, PACKET FILTERS Des IP list-80
#define CSID_C_WEBUI_PKBOTHSIP0_RECORD_RULE   _CFG_WEBUI(0x50a0)	//T=u32, C_PKFILTER_BOTH_RULE_SIP0 list-80
#define CSID_C_WEBUI_PKDMAC_RULE              _CFG_WEBUI(0x50f0)	//T=u32, ... list-80
#define CSID_C_WEBUI_URL_SOURCEIP_RULE        _CFG_WEBUI(0x5140)	//T=u32, ... list-80
#define CSID_C_WEBUI_URLMAC_RULE              _CFG_WEBUI(0x5190)	//T=u32, ... list-80
#define CSID_C_WEBUI_WEBCT_SOURCEIP_RULE      _CFG_WEBUI(0x51e0)	//T=u32, ... list-80
#define CSID_C_WEBUI_WEBCTMAC_RULE            _CFG_WEBUI(0x5230)	//T=u32, ... list-80
#define CSID_C_WEBUI_L7_SOURCEIP_RULE         _CFG_WEBUI(0x5280)	//T=u32, L7-Source IP selectbox-80
#define CSID_C_WEBUI_L7_RULE                  _CFG_WEBUI(0x52d0)	//T=u32, L7-Source MAC selectbox-80
#define CSID_C_WEBUI_SCHE_RECORD_RULE         _CFG_WEBUI(0x5320)    //T=u32, record the schedult rule-128
#define CSID_C_WEBUI_VIRTUALSRV_RECORD_RULE   _CFG_WEBUI(0x53a0)	//T=u32, C_PORTFW_SERVER_PORT-128
#define CSID_C_WEBUI_MACCTL_RECORD_RULE       _CFG_WEBUI(0x5420)	//T=u32, C_MACCTL_RULE_MAC list-256
#define CSID_C_WEBUI_DHCP_SVR_OPTIONS_RECORD  _CFG_WEBUI(0x5520)    //T=u8, D=255, Record DHCP Server Options list-256

/* Status Items */
#define CSID_S_WEBUI_CONFIRM		       	_STA_WEBUI(0x0010) //T=u8,  D=0
#define CSID_S_WEBUI_LEAVE				      _STA_WEBUI(0x0020) //T=u8,  D=0
#define CSID_S_WEBUI_CONFIRM_FAIL		    _STA_WEBUI(0x0030) //T=u8,  D=0
#define CSID_S_WEBUI_CONFIRM_FAIL_INDEX _STA_WEBUI(0x0040) //T=u8,  D=0
#define	CSID_S_WEBUI_CONFIRM_FAIL_NAME	_STA_WEBUI(0x0050) //T=str, D="", Confirm fail dev.
#define CSID_S_WEBUI_USERREBOOT         _STA_WEBUI(0x0060) //T=u32, D=0, max transmission unit
#define CSID_S_WEBUI_SPILT_RESULT       _STA_WEBUI(0x0070) //T=u8,  D=0, paggy need for D-Link use
#define CSID_S_WEBUI_FWDATE       		  _STA_WEBUI(0x0080) //T=str, D=""
#define CSID_S_WEBUI_LANGUAGE       		_STA_WEBUI(0x0090) //T=str, D="US"
#define	CSID_S_WEBUI_ODM_STRINGS       	_STA_WEBUI(0x0100) //T=str, D="", ODM Strings

#define CSID_S_WEBUI_FW_QUERY_REGION_NAME     _STA_WEBUI(0x0110) //T=str, the region name of fw query
#define CSID_S_WEBUI_FW_QUERY_ALTERED         _STA_WEBUI(0x0111) //T=u32, D=0, fw query check alter
#define CSID_S_WEBUI_FW_QUERY_VER             _STA_WEBUI(0x0112) //T=str, the fw version of fw query
#define CSID_S_WEBUI_FW_QUERY_VER_MATCH       _STA_WEBUI(0x0113) //T=u32, the matching flag between current fw version and fw query version
#define CSID_S_WEBUI_FW_QUERY_DATE            _STA_WEBUI(0x0114) //T=str, the release date of fw query
#define CSID_S_WEBUI_FW_QUERY_STATUS          _STA_WEBUI(0x0115) //T=u32, D=0, fw query auto download status
#define CSID_S_WEBUI_FW_QUERY_PERCENTAGE      _STA_WEBUI(0x0116) //T=u32, D=0, fw download and upgrade's Percentage
#define CSID_S_WEBUI_FW_QUERY_TIME            _STA_WEBUI(0x0117) //T=u32, D=0, fw download and upgrade's Sec
#define CSID_S_WEBUI_FW_UPGRADE_FLAG          _STA_WEBUI(0x0118) //T=u32, D=0, fw upgrade flag to count progress
#define CSID_S_WEBUI_FW_UPGRADE_START         _STA_WEBUI(0x0119) //T=u8, D=0, 1:start upg

#define CSID_S_WEBUI_FW_QUERY_DW_URL          _STA_WEBUI(0x0120) //T=str, the fw download url of fw query

#define CSID_S_WEBUI_FW_UPGRADE_STATUS        _STA_WEBUI(0x0121) //T=u32, the status of firmware upgrade

#define CSID_S_WEBUI_FW_QUERY_URL             _STA_WEBUI(0x0122) //T=str, the url of fw query
#define CSID_S_WEBUI_FW_NOTICE                _STA_WEBUI(0x0123) //T=u32, the notice of firmware
#define CSID_S_WEBUI_FW_URL_FILE_NAME         _STA_WEBUI(0x0124) //T=str, download url fw name
#define CSID_S_WEBUI_WAN_NUMBER               _STA_WEBUI(0x0125) //T=u32, D=0, WAN Total number
/* 0x0120~0x012F are reserved */

#define CSID_S_WEBUI_DET_DEVICE               _STA_WEBUI(0x0130) //T=u8, D=0, DIR-508L, 1 is click from phone UI to pc UI
#define CSID_S_WEBUI_URL_FOR_F5               _STA_WEBUI(0x0131) //T=str, DIR-508L want click F5 remain in the same page, not goto index.htm
/* 0x0131~0x013F are reserved */

#define CSID_S_WEBUI_ALLOW_SYSINFO            _STA_WEBUI(0x0140) //T=u8, allow access to sysinfo.htm without login
#define CSID_S_WEBUI_LOGIN_TIME               _STA_WEBUI(0x0141) //T=str, user login time
#define CSID_S_WEBUI_LOGIN_TRY                _STA_WEBUI(0x0142) //T=u8, number of failed login attempt

/* FOTA check FW */
#define CSID_S_WEBUI_CHECK_FW_SERVER            _STA_WEBUI(0x0150)	//T=str, for FOTA check fw Server
#define CSID_S_WEBUI_CHECK_FW_HTTP              _STA_WEBUI(0x0151)	//T=str, for FOTA check fw Http
#define CSID_S_WEBUI_CHECK_FW_HTTPS_MD5         _STA_WEBUI(0x0152)	//T=str, for FOTA check fw Https md5
#define CSID_S_WEBUI_CHECK_FW_STATUS         	_STA_WEBUI(0x0153)	//T=u32, for FOTA check fw Status

#endif //ifndef _CSID_WEBUI_H
