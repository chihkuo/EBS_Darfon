/*
 * @file Member ID Definition of Group WAN Bonding 
 *
 * Moderator: tstilin
 * Group ID: 0x00B200/0x80B200
 */

#ifndef _CSID_WANBONDING_H
#define _CSID_WANBONDING_H

#include "csid_gid.h"

#define _CFG_WANBONDING(x)		(CSID_GIDC_WANBONDING|(x))
#define _STA_WANBONDING(x)		(CSID_GIDS_WANBONDING|(x))

/* Extra Definitions */
#define WANBONDING_STATUS_DISCONNECT     0     //"Disconnected"
#define WANBONDING_STATUS_CONNECTING     1     //"Connecting..."
#define WANBONDING_STATUS_CONNECT        2     //"Connected"
#define WANBONDING_STATUS_DISCONNECTING  3     //"Disconnecting..."

#define WANBONDING_STATUS_ERR_UNKNOWN    100   //"Unknown error"     unknown error
#define WANBONDING_STATUS_ERR_FQDN       101   //"FQDN error"        domain name
#define WANBONDING_STATUS_ERR_R_MAC      102   //"R_MAC error"       Remote MAC
#define WANBONDING_STATUS_ERR_L_MAC      103   //"L_MAC error"       Local MAC
#define WANBONDING_STATUS_ERR_R_PSW      104   //"R_Password error"  Remote ipsec password
#define WANBONDING_STATUS_ERR_L_PSW      105   //"L_Password error"  Local ipsec password
#define WANBONDING_STATUS_ERR_LAN        106   //"LANs collision"    LANs Subnet collision
#define WANBONDING_STATUS_ERR_R_PARSE    107   //"R_Parse error"     Remote Received parse
#define WANBONDING_STATUS_ERR_L_PARSE    108   //"L_Parse error"     Local Received parse
#define WANBONDING_STATUS_ERR_R_READY    109   //"R_Ready error"     Remote Not Ready for new connection
#define WANBONDING_STATUS_ERR_L_READY    110   //"L_Ready wait"      LOCAL Not Ready for connection

/*max wan bonding i/f, bond0~bond31*/
#define MAX_WANBONDING_SIZE  (32)
#define MAX_WANBONDING_SLAVE_SIZE (255) 

#define CSID_C_WANBONDING_ENABLE			_CFG_WANBONDING(0x0001)	//T=u32,D=0,WAN Bonding enable, 0:disable, 1:enable
#define CSID_C_WANBONDING_LOCAL_MAC			_CFG_WANBONDING(0x0002)	//T=mac,D=1,WAN Bonding Local WAN MAC
#define CSID_C_WANBONDING_MAX_SLAVE			_CFG_WANBONDING(0x0003)	//T=u32, D=2, max slaves each bonding i/f
#define CSID_C_WANBONDING_ENCRY_MODE		_CFG_WANBONDING(0x0004)	//T=u32, D=1,bonding local encry-mode for list, 0:None, 1: AES256
#define CSID_C_WANBONDING_ENCRY_KEY			_CFG_WANBONDING(0x0005)	//T=str, D=0,bonding local encry-key for list, active when encry-mode=AES256.
#define CSID_C_WANBONDING_DEBUG				_CFG_WANBONDING(0x0006)	//T=u32, D=0, bit 0: enable(1)/disable(0) debugserver by UDP (boottime), bit 1: enable(1)/disable(0) log2console (runtime), bit 2: enable(1)/disable(0) log2syslog  (runtime), others: reserved
#define CSID_C_WANBONDING_MAX_NO			_CFG_WANBONDING(0x0007) //T=u32, D=?,max number of bonding can be created, 68560:3, 7621:8

// WAN Bonding status
#define CSID_S_WANBONDING_STATUS_LIST		_STA_WANBONDING(0x0010)	//T=u32, ui status, DISCONNECT=0, CONNECTING=1, CONNECT=2, DISCONNECTING=3, others: Ref WANBONDING_STATUS_ERR_XXX

//WAN Bonding altered
#define CSID_S_WANBONDING_ALTERED_LIST		_STA_WANBONDING(0x0030)	//T=u32, ui altered offset 0: for all, offset 1 for bond0, ...

/* Wan Bonding interface, the max I/F is 32(0x20)*/ //(master)
#define CSID_C_WANBONDING_IF_ENABLE_LIST		_CFG_WANBONDING(0x0070)	//T=u32, D=1,(L)bonding enble-rule, list, 0: disable, 1: enable 
#define CSID_C_WANBONDING_IF_ID_LIST			_CFG_WANBONDING(0x0090)	//T=u32, D=0,(L)index of Bonding I/F, list
#define CSID_C_WANBONDING_IF_NAME_LIST			_CFG_WANBONDING(0x00B0)	//T=str, D=0,(L)bonding name, list
#define CSID_C_WANBONDING_IF_REMOTE_LAN_IP_LIST		_CFG_WANBONDING(0x00D0)	//T=ipv4,   ,(R)bonding remote LAN IP,  list  (fw eternal usage only)
#define CSID_C_WANBONDING_IF_OPMODE_LIST		_CFG_WANBONDING(0x00F0)	//T=u32, D=0,(R)binding op-mode, list, 0: balance-rr, 1:active-backup, 2:blanace-xor, 3:broadcast (fw eternal usage only, be fixed to 0)																																		
#define CSID_C_WANBONDING_IF_ENCRY_MODE_LIST	_CFG_WANBONDING(0x0110)	//T=u32, D=1,(L)bonding remote encry-mode, list, 0:None, 1: AES256
#define CSID_C_WANBONDING_IF_ENCRY_KEY_LIST		_CFG_WANBONDING(0x0130)	//T=str, D=0,(L)bonding remote encry-key, list, active when encry-mode=AES256.
#define CSID_C_WANBONDING_IF_KEEP_ALIVE_LIST	_CFG_WANBONDING(0x0150)	//T=u32, D=1,(L)bonding keep-alive, list, 0: disable, 1: enable
#define CSID_C_WANBONDING_IF_REMOTE_MAC_LIST	_CFG_WANBONDING(0x0170)	//T=mac, D=1,(L)remote first MAC of WAN port, list 
#define CSID_C_WANBONDING_IF_IP_LIST			_CFG_WANBONDING(0x0190)	//T=ipv4,   ,(L)IP Address for LOCAL Bonding I/F, list  (fw internal usage only)
#define CSID_C_WANBONDING_IF_TIMEOUT_EN_LIST	_CFG_WANBONDING(0x01B0)	//T=u32, D=0,(L)bonding enable-timeout, list, 0: disable, 1: enable 
#define CSID_C_WANBONDING_IF_TIMEOUT_THR_LIST	_CFG_WANBONDING(0x01D0)	//T=u32, D=0,(L)bonding timeout threshold, how long will be re-create while without response, list, unit:sec  
#define CSID_C_WANBONDING_IF_AUTO_WEIGHT_LIST	_CFG_WANBONDING(0x01F0)	//T=u32, D=1,(L)bonding Network Auto WEIGHT, list, 0:disable, 1:enable



/* for Pre-Query, JOIN into bondx, or not. NOW, define WAN1, WAN2, WAN3 only*/
#define CSID_C_WANBONDING_IF_PQ1_LOCALWAN_LIST	_CFG_WANBONDING(0x0300)	//T=u32,    ,(L)local wan interface for Pre-Query wan 1 for list
#define CSID_C_WANBONDING_IF_PQ1_REMOTEDON_LIST	_CFG_WANBONDING(0x0320)	//T=str,    ,(L)the remote domain name or ip for Pre-Query wan 1 for list
#define CSID_C_WANBONDING_IF_PQ1_WEIGHT_LIST	_CFG_WANBONDING(0x0340)	//T=u32, D=1,(L)local wan 1 weight(0:High/1:Middle/2:Low) for Pre-Query wan 1 for list
#define CSID_C_WANBONDING_IF_PQ1_ENABLE_LIST	_CFG_WANBONDING(0x0360)	//T=u32, D=1,(L)Pre-Query wan 1 ENABLE for list, 0: disable, 1: enable 
//Reserved: 0x0380, 0x03A0
#define CSID_C_WANBONDING_IF_PQ2_LOCALWAN_LIST	_CFG_WANBONDING(0x03C0)	//T=u32,    ,(L)local wan interface for Pre-Query wan 2 for list
#define CSID_C_WANBONDING_IF_PQ2_REMOTEDON_LIST	_CFG_WANBONDING(0x03E0)	//T=str,    ,(L)the remote domain name or ip for Pre-Query wan 2 for list
#define CSID_C_WANBONDING_IF_PQ2_WEIGHT_LIST	_CFG_WANBONDING(0x0400)	//T=u32, D=1,(L)local wan 2 weight(0:High/1:Middle/2:Low) for Pre-Query wan 2 for list
#define CSID_C_WANBONDING_IF_PQ2_ENABLE_LIST	_CFG_WANBONDING(0x0420)	//T=u32, D=1,(L)Pre-Query wan 2 ENABLE for list, 0: disable, 1: enable 
//Reserved: 0x0440, 0x0460
#define CSID_C_WANBONDING_IF_PQ3_LOCALWAN_LIST	_CFG_WANBONDING(0x0480)	//T=u32,    ,(L)local wan interface for Pre-Query wan 3 for list
#define CSID_C_WANBONDING_IF_PQ3_REMOTEDON_LIST	_CFG_WANBONDING(0x04A0)	//T=str,    ,(L)the remote domain name or ip for Pre-Query wan 3 for list
#define CSID_C_WANBONDING_IF_PQ3_WEIGHT_LIST	_CFG_WANBONDING(0x04C0)	//T=u32, D=1,(L)local wan 3 weight(0:High/1:Middle/2:Low) for Pre-Query wan 3 for list
#define CSID_C_WANBONDING_IF_PQ3_ENABLE_LIST	_CFG_WANBONDING(0x04E0)	//T=u32, D=1,(L)Pre-Query wan 3 ENABLE fo list, 0: disable, 1: enable 
//Reserved: 0x0500, 0x0520
#define CSID_C_WANBONDING_IF_PQ4_LOCALWAN_LIST	_CFG_WANBONDING(0x0540)	//T=u32,    ,(L)local wan interface for Pre-Query wan 4 for list
#define CSID_C_WANBONDING_IF_PQ4_REMOTEDON_LIST	_CFG_WANBONDING(0x0560)	//T=str,    ,(L)the remote domain name or ip for Pre-Query wan 4 for list
#define CSID_C_WANBONDING_IF_PQ4_WEIGHT_LIST	_CFG_WANBONDING(0x0580)	//T=u32, D=1,(L)local wan 4 weight(0:High/1:Middle/2:Low) for Pre-Query wan 4 for list
#define CSID_C_WANBONDING_IF_PQ4_ENABLE_LIST	_CFG_WANBONDING(0x05A0)	//T=u32, D=1,(L)Pre-Query wan 4 ENABLE for list, 0: disable, 1: enable 

#define CSID_S_WANBONDING_IF_PQ_INDEX_LIST		_STA_WANBONDING(0x0050)	//T=u32,    ,(L)query index for Pre-Query for list

/* Bonding-slave list(pool type, pool size: 0xFF) */ //(slave)
#define CSID_S_WANBONDING_SLAVE_IF_ID_LIST			_STA_WANBONDING(0x0110)	//T=u32, D=0, (L)Slave ID fo list
#define CSID_S_WANBONDING_SLAVE_IF_REFID_LIST		_STA_WANBONDING(0x0210)	//T=u32, D=FF,(L)Ref to which bonding-ID for list, this slave is refer-ed by which bonding I/F. FF: without any reference.(index_b)
#define CSID_S_WANBONDING_SLAVE_IF_LOCALWAN_LIST	_STA_WANBONDING(0x0310)	//T=u32, D=FF,(L)slave interface of the local router for list(local wan id, wan1=>0, wan2=>1,...)(index_l)
#define CSID_S_WANBONDING_SLAVE_IF_WEIGHT_LIST		_STA_WANBONDING(0x0410)	//T=u32, D=0(auto),(L) the percentage of weight. (-1)=> disable
#define CSID_S_WANBONDING_SLAVE_IF_REMOTEGW_LIST	_STA_WANBONDING(0x0510)	//T=ipv4,    ,(R)the remote gateway for this slave interface for list
#define CSID_S_WANBONDING_SLAVE_IF_REMOTEWAN_LIST	_STA_WANBONDING(0x0610)	//T=u32, D=FF,(R)slave interface of the remote router for list(remote wan id, wan1=>0, wan2=>1,...)(index_r)
#define CSID_S_WANBONDING_SLAVE_IF_ENABLE_LIST		_STA_WANBONDING(0x0710)	//T=u32, D=1 ,(L)bonding slave enable-rule for list, 0: disable, 1: enable

#endif //_CSID_WANBONDING_H
