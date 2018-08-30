/*
 * @file Member ID Definition of Group OpenVPN.
 *
 * Moderator: Chad
 * Group ID: 0x00720000/0x80720000
 */

#ifndef _CSID_OVPN_H
#define _CSID_OVPN_H

#include "csid_gid.h"

#define _CFG_OVPN(x)       (CSID_GIDC_OVPN|(x))
#define _STA_OVPN(x)       (CSID_GIDS_OVPN|(x))

/* Extra Definitions */

//Enable/Disable Switch
#define	OVPN_DISABLE		    0
#define	OVPN_ENABLE	            1

// Connecting type
#define OVPN_CNT_AUTO			0
#define OVPN_CNT_MANUAL		    1
//#define OVPN_CNT_ONDEMAND		2

//Tunnel Status
#define OVPN_S_CONNECTED		1
#define OVPN_S_WAIT			    2 //Wait for traffic
#define OVPN_S_DISCONNECT		3
#define OVPN_S_ERROR			8
#define OVPN_S_CONNECTING		9

//Tunnel Altered Type
#define OVPN_S_TUN_CFG_CHG	        1
#define OVPN_S_TUN_NEW_SHARE_KEY	2    
#define OVPN_S_TUN_NEW_CA           3
#define OVPN_S_TUN_NEW_DH	        4
#define OVPN_S_TUN_NEW_SERVER_KEY	5    
#define OVPN_S_TUN_NEW_CLIENT_KEY	6    
#define OVPN_S_TUN_DEL_CLIENT_KEY	7    

/* Status Items */
#define CSID_S_OVPN_ALTERED				    _STA_OVPN(0x0000) //T=u8,  openvpn setting altered
#define CSID_S_OVPN_ACTION_ALTERED          _STA_OVPN(0x0001) //T=u8,  openvpn which tunnel's button clicked
#define CSID_S_OVPN_CLI_CA_ALTERED          _STA_OVPN(0x0002) //T=u8,  openvpn Client CA config altered
#define CSID_S_OVPN_CLI_CA_ACTION           _STA_OVPN(0x0003) //T=u8,  openvpn Client CA action [New|DEL]
#define CSID_S_OVPN_TUN_NEW_CA              _STA_OVPN(0x0100) //T=u8,  openvpn Root CA #n try to [NEW_CA|DEL_CA]
#define CSID_S_OVPN_TUN_NEW_DH              _STA_OVPN(0x0200) //T=u8,  openvpn Root CA #n try to alter
#define CSID_S_OVPN_TUN_NEW_KEY             _STA_OVPN(0x0300) //T=u8,  openvpn Root CA #n try to alter
#define CSID_S_OVPN_TUN_NEW_SHARE_KEY       _STA_OVPN(0x0400) //T=u8,  openvpn Root CA #n try to alter
#define CSID_S_OVPN_TUN_ALTERED	            _STA_OVPN(0x0500) //T=u8,  openvpn tunnel #n try to [CFG_CHG|NEW_KEY|ADD_CLIENT|DEL_CLIENT]
#define CSID_S_OVPN_TUN_STATUS              _STA_OVPN(0x0600) //T=u8,  openvpn tunnel status
#define CSID_S_OVPN_TUN_PID                 _STA_OVPN(0x0700) //T=u16, openvpn tunnel pid
#define CSID_S_OVPN_TUN_IFNAME              _STA_OVPN(0x0800) //T=str, openvpn tunnel interface name

/* Configuration Items */
#define CSID_C_OVPN_ENABLE				    _CFG_OVPN(0x0000) //T=u8,  D=0, 0: Disable, 1: Enable

/* Tunnel Setting */
#define CSID_C_OVPN_TUN_ENABLE				_CFG_OVPN(0x0100) //T=u8,  D=0, Tunnel Enable   0:Disable, 1: Enable
#define CSID_C_OVPN_TUN_MODE				_CFG_OVPN(0x0200) //T=u8,  D=0, Tunnel Mode     0: Server, 1: Client
#define CSID_C_OVPN_TUN_NAME			    _CFG_OVPN(0x0300) //T=str, Tunnel name

/* Tunnel Setting for Server*/
#define CSID_C_OVPN_TUN_MAX_CLIENT			_CFG_OVPN(0x0400) //T=u8,  D=5, Max number of clients
#define CSID_C_OVPN_TUN_CLIENT_LIST		    _CFG_OVPN(0x0500) //T=str, Linked-List Type: "IDX_0:CA_LIST[IDX_0],IDX_1:CA_LIST[IDX_1],IDX_2:CA_LIST[IDX_2], ...etc" 
#define CSID_C_OVPN_TUN_CLI_TO_CLI		    _CFG_OVPN(0x0600) //T=u8,  D=1, Client to Client
#define CSID_C_OVPN_TUN_DUPLICATE_CN		_CFG_OVPN(0x0700) //T=u8,  D=1, Duplicate CN
#define CSID_C_OVPN_TUN_CA_EXPIRE           _CFG_OVPN(0x0800) //T=u16, D=3650, CA Expired
#define CSID_C_OVPN_TUN_CA_CRT              _CFG_OVPN(0x0900) //T=str, ca.crt file
#define CSID_C_OVPN_TUN_CA_KEY              _CFG_OVPN(0x0a00) //T=str, ca.key file
#define CSID_C_OVPN_TUN_DH                  _CFG_OVPN(0x0b00) //T=str, dh$keylen.pem
#define CSID_C_OVPN_TUN_INDEX               _CFG_OVPN(0x0c00) //T=str, index file
#define CSID_C_OVPN_TUN_SERIAL              _CFG_OVPN(0x0d00) //T=str, serial file
#define CSID_C_OVPN_TUN_SC_CRT              _CFG_OVPN(0x0e00) //T=str, Server/Client.crt file 
#define CSID_C_OVPN_TUN_SC_CSR              _CFG_OVPN(0x0f00) //T=str, Server/Client.csr file 
#define CSID_C_OVPN_TUN_SC_KEY              _CFG_OVPN(0x1000) //T=str, Server/Client.key file 
#define CSID_C_OVPN_TUN_KEY_SIZE            _CFG_OVPN(0x1100) //T=u16, D=512, Key Size
#define CSID_C_OVPN_TUN_KEY_EXPIRE          _CFG_OVPN(0x1200) //T=u16, D=3650, Key expired time
#define CSID_C_OVPN_TUN_KEY_COUNTRY         _CFG_OVPN(0x1300) //T=str, Country
#define CSID_C_OVPN_TUN_KEY_PROVINCE        _CFG_OVPN(0x1400) //T=str, Province
#define CSID_C_OVPN_TUN_KEY_CITY            _CFG_OVPN(0x1500) //T=str, City
#define CSID_C_OVPN_TUN_KEY_ORG             _CFG_OVPN(0x1600) //T=str, Organiztion 
#define CSID_C_OVPN_TUN_KEY_EMAIL           _CFG_OVPN(0x1700) //T=str, E-mail

/* Tunnel Setting for Client */
#define CSID_C_OVPN_TUN_CNT_TYPE			_CFG_OVPN(0x1800) //T=u8,  D=0, Connection Type 0: Auto 1: Manual
#define CSID_C_OVPN_TUN_HOST		        _CFG_OVPN(0x1900) //T=str, Remote Host/IP
#define CSID_C_OVPN_TUN_ROUTE		        _CFG_OVPN(0x1a00) //T=str, Peer Routing
#define CSID_C_OVPN_TUN_NAT		            _CFG_OVPN(0x1b00) //T=u8,  Enable/Disable NAT for this tunnel 

/* Tunnel Setting for Gerneral */
#define CSID_C_OVPN_TUN_ENCRY_TYPE		    _CFG_OVPN(0x1c00) //T=u8,  D=0, Encryption Method, 0: Static Key, 1: PKI (X.509)
#define CSID_C_OVPN_TUN_SHARE_KEY_LEN		_CFG_OVPN(0x1d00) //T=u8,  D=0, Key length, 0: 384 Bit, 1: 512 Bit, 2: 768Bit, 3: 1024 Bit, 4: 2048 Bit
#define CSID_C_OVPN_TUN_SHARE_KEY        	_CFG_OVPN(0x1e00) //T=str, Share/TLS Key.
 
#define CSID_C_OVPN_TUN_PORT		        _CFG_OVPN(0x1f00) //T=u16, D=0, Port,           0: UDP, 1: TCP
#define CSID_C_OVPN_TUN_PROTOCOL		    _CFG_OVPN(0x2000) //T=u8,  D=0, Protocol,       0: UDP, 1: TCP
#define CSID_C_OVPN_TUN_DEVICE		        _CFG_OVPN(0x2100) //T=u8,  D=0, Device,         0: TUN (Routed VPN), 1: TAP (Bridged VPN)
#define CSID_C_OVPN_TUN_LOCAL_IP		    _CFG_OVPN(0x2200) //T=ipv4,TUN device should assign Local IP manually
#define CSID_C_OVPN_TUN_REMOTE_IP		    _CFG_OVPN(0x2300) //T=ipv4,TUN device should assign Remote IP manually
#define CSID_C_OVPN_TUN_COMPRESS		    _CFG_OVPN(0x2400) //T=u8,  D=0, Comporess Type, 0: LZO,  1: None, 2: Disable
#define CSID_C_OVPN_TUN_CIPHER		        _CFG_OVPN(0x2500) //T=u8,  D=0, Ciphers Type,   0: Default  
#define CSID_C_OVPN_TUN_KEEPALIVE_TYPE		_CFG_OVPN(0x2600) //T=u8,  D=0, Keep Alive Type 0: Enable, 1: Disable
#define CSID_C_OVPN_TUN_KEEPALIVE_INTERVAL	_CFG_OVPN(0x2700) //T=u8,  D=10,  Keep Alive Interval
#define CSID_C_OVPN_TUN_KEEPALIVE_TIMEOUT	_CFG_OVPN(0x2800) //T=u8,  D=120, Keep Alive Timeout

/* Root CA: Reserved 100 Client CAs are shared with all Tunnel with Server mode */
#define CSID_C_OVPN_CLI_CA_NAME             _CFG_OVPN(0x2900) //T=str, Client Name
#define CSID_C_OVPN_CLI_CA_CSR              _CFG_OVPN(0x2a00) //T=str, Client.cst file
#define CSID_C_OVPN_CLI_CA_CRT              _CFG_OVPN(0x2b00) //T=str, Client.crt file
#define CSID_C_OVPN_CLI_CA_KEY              _CFG_OVPN(0x2c00) //T=str, Client.key file
#define CSID_C_OVPN_CLI_CA_PEM              _CFG_OVPN(0x2d00) //T=str, Client.pem file

/* New OpenVPN Server Setting CSID */
#define CSID_C_OVPN_SVR_ENABLE					_CFG_OVPN(0x3010) 	//T=u8, D=0, Server Enable  0:Disable, 1: Enable
#define CSID_C_OVPN_SVR_INTERFACE				_CFG_OVPN(0x3020) 	//T=u8, D=0, Server Interface
#define CSID_C_OVPN_SVR_PROTOCOL		   		_CFG_OVPN(0x3030) 	//T=u8, D=1, Protocol, 0: UDP, 1: TCP
#define CSID_C_OVPN_SVR_PORT		        	_CFG_OVPN(0x3040) 	//T=u16, D=443, Port
#define CSID_C_OVPN_SVR_DEVICE		        	_CFG_OVPN(0x3050) 	//T=u8, D=0, Device, 0: TUN (Routed VPN), 1: TAP (Bridged VPN)
#define CSID_C_OVPN_SVR_AUTH_MODE				_CFG_OVPN(0x3060)	//T=u8, D=0, 0:TLS, 1:Static Key
#define CSID_C_OVPN_SVR_L_ENDPOINT_IP			_CFG_OVPN(0x3070)	//T=str, OpenVPN local endpoint ip
#define CSID_C_OVPN_SVR_R_ENDPOINT_IP			_CFG_OVPN(0x3080)	//T=str, OpenVPN remote endpoint ip
#define CSID_C_OVPN_SVR_IP						_CFG_OVPN(0x3090)	//T=str, OpenVPN Server IP
#define CSID_C_OVPN_SVR_MASK					_CFG_OVPN(0x30A0)	//T=str, OpenVPN Server netmask
#define CSID_C_OVPN_SVR_GW						_CFG_OVPN(0x30B0)	//T=str, OpenVPN Server gateway
#define CSID_C_OVPN_SVR_IPPOOL_START  			_CFG_OVPN(0x30C0)	//T=str, OpenVPN Server pool ip start
#define CSID_C_OVPN_SVR_IPPOOL_END  			_CFG_OVPN(0x30D0)	//T=str, OpenVPN Server pool ip end
#define CSID_C_OVPN_SVR_CA						_CFG_OVPN(0x30E0)	//T=str, OpenVPN ca certificate
#define CSID_C_OVPN_SVR_CERT					_CFG_OVPN(0x30F0)	//T=str, OpenVPN server certificate
#define CSID_C_OVPN_SVR_DH                  	_CFG_OVPN(0x3100) 	//T=str, dh.pem
#define CSID_C_OVPN_SVR_STATIC_KEY				_CFG_OVPN(0x3110)	//T=str, OpenVPN static key
#define CSID_C_OVPN_SVR_CIPHER		        	_CFG_OVPN(0x3120) 	//T=u32, D=1, 0:None,1:bf,2:aes128,3:aes192,4:aes256,5:aes512, Encryption cipher
#define CSID_C_OVPN_SVR_HASH		        	_CFG_OVPN(0x3130)	//T=u32, D=1, 0:None,1:sha1,2:sha256,3:sha512,4:md4,5:md5, Hash algorithm
#define CSID_C_OVPN_SVR_ADVANCE					_CFG_OVPN(0x3140)	//T=u32, D=0, OpenVPN Server advance option
#define CSID_C_OVPN_SVR_TLS_CIPHER				_CFG_OVPN(0x3150)	//T=u32, D=2, 0:None,1:rc4-md5,2:rsa-aes128,3:rsa-aes256,4:rsa-sha384,5:dhe-aes128,6:dhe-aes256,7:dhe-sha384 TLS CIPHER
#define CSID_C_OVPN_SVR_LZO						_CFG_OVPN(0x3160)	//T=u32, D=0, 0:Adaptive,1:yes,2:no OpenVPN Server lzo
#define CSID_C_OVPN_SVR_TLS_KEY					_CFG_OVPN(0x3170)	//T=str, OpenVPN Server tls key
#define CSID_C_OVPN_SVR_MTU						_CFG_OVPN(0x3180)	//T=u32, D=1500, OpenVPN Server mtu
#define CSID_C_OVPN_SVR_UDP_FRAGMENT			_CFG_OVPN(0x3190)	//T=u32, D=1500, OpenVPN Server udp fragment
#define CSID_C_OVPN_SVR_MSS_FIX_ENABLE			_CFG_OVPN(0x31A0)	//T=u32, D=0, OpenVPN Server mss fix
#define CSID_C_OVPN_SVR_CLI_TO_CLI		    	_CFG_OVPN(0x31B0) 	//T=u8, D=1, Client to Client
#define CSID_C_OVPN_SVR_DUPLICATE_CN			_CFG_OVPN(0x31C0) 	//T=u8, D=1, Duplicate CN
#define CSID_C_OVPN_SVR_REDIRECT_ENABLE			_CFG_OVPN(0x31D0)	//T=u32, D=1, OpenVPN Redirect Internet traffic 
#define CSID_C_OVPN_SVR_MAX_CLIENT				_CFG_OVPN(0x31E0) 	//T=u8, D=5, Max number of clients
#define CSID_C_OVPN_SVR_CCD_FILE				_CFG_OVPN(0x31F0)	//T=str, OpenVPN Server CCD Default File
#define CSID_C_OVPN_SVR_CLIENT_CONNECT_SCRIPT	_CFG_OVPN(0x3200)	//T=str, OpenVPN Server Client connection script
#define CSID_C_OVPN_SVR_ADD_CONFIGURE			_CFG_OVPN(0x3210)	//T=str, OpenVPN Server additional configuration
#define CSID_C_OVPN_SVR_DHCP_PROXY_ENABLE		_CFG_OVPN(0x3220)	//T=u32, D=1, OpenVPN Server DHCP Proxy mode


#endif //ifndef _CSID_OVPN_H
