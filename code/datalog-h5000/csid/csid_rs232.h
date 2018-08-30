/*
 * @file Member ID Definition of RS232 Modem
 *
 * Moderator: Eric chen
 * Group ID: 0x00420000/0x80420000
 */
#ifndef _CSID_RS232_H
#define _CSID_RS232_H

#include "csid_gid.h"

#define _CFG_RS232(x)		(CSID_GIDC_RS232|(x))
#define _STA_RS232(x)		(CSID_GIDS_RS232|(x))

/* Extra Definitions */
#define CNT_TYPE_RS232_ONDEMAND				0x00
#define CNT_TYPE_RS232_AUTORECONNECT		0x01
#define CNT_TYPE_RS232_MANUAL				0x02

#define DIALUP_CNT_TYPE_ONDEMAND          0x00
#define DIALUP_CNT_TYPE_AUTORECONNECT     0x01
#define DIALUP_CNT_TYPE_MANUAL            0x02
#define DIALUP_CNT_TYPE_ONFAILOVER        0x03

#define CNT_STATUS_RS232_DISCONNECT			0x00
#define CNT_STATUS_RS232_CONNECTING			0x01
#define CNT_STATUS_RS232_CONNECT			0x02
#define CNT_STATUS_RS232_DISCONNECTING		0x03
#define CNT_STATUS_RS232_WAIT_TRAFFIC		0x05
#define CNT_STATUS_RS232_AUTH_FAIL			0x06
#define CNT_STATUS_RS232_BACKUP				0x07
#define CNT_STATUS_RS232_NOT_INSCHEDULE		0x08

#define DIALUP_CNT_STATUS_DISCONNECT     0x00
#define DIALUP_CNT_STATUS_CONNECTING     0x01
#define DIALUP_CNT_STATUS_CONNECT        0x02
#define DIALUP_CNT_STATUS_DISCONNECTING  0x03
#define DIALUP_CNT_STATUS_WAIT_TRAFFIC   0x05
#define DIALUP_CNT_STATUS_AUTH_FAIL      0x06

/*	//define in CSID_3G.h
#define ONDEMAND_STATUS_DISCONNECT_NO_IP	0x00
#define ONDEMAND_STATUS_CONNECT_NO_IP		0x01
#define ONDEMAND_STATUS_CONNECT_GET_IP		0x02
*/
#define AUTHTYPE_RS232_AUTO					0x00
#define AUTHTYPE_RS232_PAP					0x01
#define AUTHTYPE_RS232_CHAP					0x02

#define KEEP_ALIVE_RS232_DISABLE			0x00
#define KEEP_ALIVE_RS232_PING				0x01
#define KEEP_ALIVE_RS232_LCP				0x02

#define AUTOBAK_STATUS_RS232_DISCONNECT		0x00
#define AUTOBAK_STATUS_RS232_CONNECTING		0x01
#define AUTOBAK_STATUS_RS232_CONNECT		0x02
#define AUTOBAK_STATUS_RS232_DISCONNECTING	0x03

/*
#define BAUDRATE_38400		0x00
#define BAUDRATE_57600		0x01
#define BAUDRATE_115200		0x02
#define BAUDRATE_230400		0x03
#define BAUDRATE_460800		0x04
*/


/* Configuration Items */
#define CSID_C_RS232_MODEM_SUPPORT			_CFG_RS232(0x0001)	//T=u32, D=0, 0:Not support, 1: Serial Modem support

#define CSID_C_RS232_NUMBER					_CFG_RS232(0x0002)	//T=str, Dialed Number
#define CSID_C_RS232_USER					_CFG_RS232(0x0003)	//T=str, user name
#define CSID_C_RS232_PASSWORD				_CFG_RS232(0x0004)	//T=str, user password
#define CSID_C_RS232_IDLETIME				_CFG_RS232(0x0005)	//T=u32, max idle time
#define CSID_C_RS232_CNT_TYPE				_CFG_RS232(0x0006)	//T=u32, connect type, Dial-on-Demand, autoconnect...
#define CSID_C_RS232_BUADRATE				_CFG_RS232(0x0007)	//T=u32, D=1, 0:38400/1:57600/2:115200/3:230400/4:460800 bps
#define CSID_C_RS232_IF_IP					_CFG_RS232(0x0008)	//T=ipv4, static interface ip
#define CSID_C_RS232_IF_PRIDNS				_CFG_RS232(0x0009)	//T=ipv4, dynamic interface primary DNS
#define CSID_C_RS232_IF_SECDNS				_CFG_RS232(0x000A)	//T=ipv4, dynamic interface secondary DNS
#define CSID_C_RS232_EXTRA_SEETING			_CFG_RS232(0x000B)	//T=str, for customer extra modem AT command

#define CSID_C_RS232_SERVICENAME			_CFG_RS232(0x0010)	//T=str, service name
#define CSID_C_RS232_MTU					_CFG_RS232(0x0011)	//T=u32, D=0, max transmission unit
#define CSID_C_RS232_AUTOBAK_ENABLE			_CFG_RS232(0x0012)	//T=u32, auto backup enable, 0:disable, 1:enable
#define CSID_C_RS232_PING_HOST_IP			_CFG_RS232(0x0013)	//T=ipv4, ping host IP for 3g auto backup
#define CSID_C_RS232_AUTHTYPE				_CFG_RS232(0x0014)	//T=u32, auth type, Auto, PAP, CHAP...
#define CSID_C_RS232_KEEP_ALIVE				_CFG_RS232(0x0015)	//T=u32, 3g keep alive, disable, ping, lcp
#define CSID_C_RS232_KA_PING_INTERVAL		_CFG_RS232(0x0016)	//T=u32, D=60, ping interval for 3g keep alive
#define CSID_C_RS232_KA_PING_IP				_CFG_RS232(0x0017)	//T=ipv4, ping IP for 3g keep alive
#define CSID_C_RS232_KA_LCP_INTERVAL		_CFG_RS232(0x0018)	//T=u32, D=10, lcp interval for 3g keep alive 
#define CSID_C_RS232_KA_LCP_TIMES			_CFG_RS232(0x0019)	//T=u32, D=3, lcp times for 3g keep alive
#define CSID_C_RS232_NAT_DISABLE			_CFG_RS232(0x001A)	//T=u32, wantype NAT disable
#define CSID_C_RS232_MODE_CHANGE            _CFG_RS232(0x001B)  //T=u32, D=1, 0:All off, 1:RS-232 2:RS-422/RS-485 TX 3:RS-485 RX
#define CSID_C_RS232_LISTEN_PORT            _CFG_RS232(0x001C)  //T=u32, D=4001, port number for virtual com port 
#define CSID_C_RS232_SERIAL_FUNCTION        _CFG_RS232(0x001D)  //T=u32, D=65535, show the serial operatiom mode options
#define CSID_C_RS232_SERIAL_PORT_NAME       _CFG_RS232(0x0020)  //T=str, Not used because change to S-csid, fix compile error for old version

#define CSID_C_RS232_MB_SERIAL_PROTOCOL			_CFG_RS232(0x0100)  //T=u8, D=1, modbus serial port protocol 1:RTU 2:ASCII
#define CSID_C_RS232_MB_SLAVE_ID				_CFG_RS232(0x0101)  //T=u8, D=1, modbus slave id
#define CSID_C_RS232_MB_FUNCTION_CODE			_CFG_RS232(0x0102)  //T=u8,  modbus function code
#define CSID_C_RS232_MB_STARTING_ADDRESS		_CFG_RS232(0x0103)  //T=u16, modbus starting address
#define CSID_C_RS232_MB_QUANTITY				_CFG_RS232(0x0104)  //T=u16, modbus quantity
#define CSID_C_RS232_MB_TIMER_ENABLE			_CFG_RS232(0x0105)  //T=u8, 1:Enable 2:Disable
#define CSID_C_RS232_MB_TIMER_INTERVAL			_CFG_RS232(0x0106)  //T=u32, modbus timer interval
#define CSID_C_RS232_MB_RECV_DATA_TYPE			_CFG_RS232(0x0107)  //T=u8,  modbus slave data type
#define CSID_C_RS232_MB_FTP_SERVER_IP			_CFG_RS232(0x0108)  //T=ipv4, client's ftp server ip for modbus data storage
#define CSID_C_RS232_MB_FTP_USER_NAME			_CFG_RS232(0x0109)  //T=str, client's ftp server username
#define CSID_C_RS232_MB_FTP_PASSWORD			_CFG_RS232(0x0110)  //T=str, client's ftp server password
#define CSID_C_RS232_VC_KEEPALIVE_ENABLE		_CFG_RS232(0x0111)  //T=u8, enable TCP keepalive
#define CSID_C_RS232_VC_CONNECTION_IDLETIME	 	_CFG_RS232(0x0112)  //T=u32, D=0, virtual COM connection idle time, unit is ms
#define CSID_C_RS232_VC_TRUSTED_IP_ACCESS		_CFG_RS232(0x0113)  //T=u8, D=1, trusted ip access, 1:allow all, 2:specific IP
#define CSID_C_RS232_VC_IP_ADDR_1_ENABLE		_CFG_RS232(0x0114)  //T=u8, D=0, virtual COM ip addr1 enable
#define CSID_C_RS232_VC_IP_ADDR_2_ENABLE		_CFG_RS232(0x0115)  //T=u8, D=0, virtual COM ip addr2 enable
#define CSID_C_RS232_VC_IP_ADDR_3_ENABLE		_CFG_RS232(0x0116)  //T=u8, D=0, virtual COM ip addr3 enable
#define CSID_C_RS232_VC_IP_ADDR_4_ENABLE		_CFG_RS232(0x0117)  //T=u8, D=0, virtual COM ip addr4 enable
#define CSID_C_RS232_VC_IP_ADDRESS_1_From       _CFG_RS232(0x0118) //T=ipv4, virtual COM IP ADDRESS 1 from
#define CSID_C_RS232_VC_IP_ADDRESS_1_To         _CFG_RS232(0x0119) //T=ipv4, virtual COM IP ADDRESS 1 to
#define CSID_C_RS232_VC_IP_ADDRESS_2_From       _CFG_RS232(0x0120) //T=ipv4, virtual COM IP ADDRESS 2 from
#define CSID_C_RS232_VC_IP_ADDRESS_2_To         _CFG_RS232(0x0121) //T=ipv4, virtual COM IP ADDRESS 2 to
#define CSID_C_RS232_VC_IP_ADDRESS_3_From       _CFG_RS232(0x0122) //T=ipv4, virtual COM IP ADDRESS 3 from
#define CSID_C_RS232_VC_IP_ADDRESS_3_To         _CFG_RS232(0x0123) //T=ipv4, virtual COM IP ADDRESS 3 to
#define CSID_C_RS232_VC_IP_ADDRESS_4_From       _CFG_RS232(0x0124) //T=ipv4, virtual COM IP ADDRESS 4 from
#define CSID_C_RS232_VC_IP_ADDRESS_4_To         _CFG_RS232(0x0125) //T=ipv4, virtual COM IP ADDRESS 4 to
#define CSID_C_RS232_MB_TRUSTED_IP_ACCESS		_CFG_RS232(0x0126)  //T=u8, D=1, trusted ip access, 1:allow all, 2:specific IP
#define CSID_C_RS232_MB_IP_ADDR_1_ENABLE		_CFG_RS232(0x0127)  //T=u8, D=0, modbus ip addr1 enable
#define CSID_C_RS232_MB_IP_ADDR_2_ENABLE		_CFG_RS232(0x0128)  //T=u8, D=0, modbus ip addr2 enable
#define CSID_C_RS232_MB_IP_ADDR_3_ENABLE		_CFG_RS232(0x0129)  //T=u8, D=0, modbus ip addr3 enable
#define CSID_C_RS232_MB_IP_ADDR_4_ENABLE		_CFG_RS232(0x0130)  //T=u8, D=0, modbus ip addr4 enable
#define CSID_C_RS232_MB_IP_ADDRESS_1_From       _CFG_RS232(0x0131) //T=ipv4, modbus IP ADDRESS 1 from
#define CSID_C_RS232_MB_IP_ADDRESS_1_To         _CFG_RS232(0x0132) //T=ipv4, modbus IP ADDRESS 1 to
#define CSID_C_RS232_MB_IP_ADDRESS_2_From       _CFG_RS232(0x0133) //T=ipv4, modbus IP ADDRESS 2 from
#define CSID_C_RS232_MB_IP_ADDRESS_2_To         _CFG_RS232(0x0134) //T=ipv4, modbus IP ADDRESS 2 to
#define CSID_C_RS232_MB_IP_ADDRESS_3_From       _CFG_RS232(0x0135) //T=ipv4, modbus IP ADDRESS 3 from
#define CSID_C_RS232_MB_IP_ADDRESS_3_To         _CFG_RS232(0x0136) //T=ipv4, modbus IP ADDRESS 3 to
#define CSID_C_RS232_MB_IP_ADDRESS_4_From       _CFG_RS232(0x0137) //T=ipv4, modbus IP ADDRESS 4 from
#define CSID_C_RS232_MB_IP_ADDRESS_4_To         _CFG_RS232(0x0138) //T=ipv4, modbus IP ADDRESS 4 to
#define CSID_C_RS232_MB_KEEPALIVE_ENABLE		_CFG_RS232(0x0139)  //T=u8, enable TCP keepalive
#define CSID_C_RS232_MB_LISTEN_PORT    	        _CFG_RS232(0x0140)  //T=u32, D=502, port number for modbus tcp
#define CSID_C_RS232_MB_SERIAL_RSP_TIMEOUT	 	_CFG_RS232(0x0141)  //T=u32, D=1000, modbus serial response timeout, unit is ms
#define CSID_C_RS232_MB_SERIAL_TIMEOUT_RETRY	_CFG_RS232(0x0142)  //T=u8, D=0, serial timeout retry (0-5)
#define CSID_C_RS232_MB_TX_DELAY_ENABLE		 	_CFG_RS232(0x0143)  //T=u8,  delay a while for transmit new message
#define CSID_C_RS232_MB_CONN_IDLE_TIMEOUT	 	_CFG_RS232(0x0144)  //T=u32, D=300, client connection idle timeout, unit is second
#define CSID_C_RS232_MB_TCP_CONNECTION_MAX		_CFG_RS232(0x0145)  //T=u8, D=4, maximum client connection (1-4)
#define CSID_C_RS232_MB_PRIORITY_1_ENABLE		_CFG_RS232(0x0146)  //T=u8, D=0, modbus priority 1 enable
#define CSID_C_RS232_MB_PRIORITY_2_ENABLE		_CFG_RS232(0x0147)  //T=u8, D=0, modbus priority 2 enable
#define CSID_C_RS232_MB_PRIORITY_3_ENABLE		_CFG_RS232(0x0148)  //T=u8, D=0, modbus priority 3 enable
#define CSID_C_RS232_MB_PRIORITY_4_ENABLE		_CFG_RS232(0x0149)  //T=u8, D=0, modbus priority 4 enable
#define CSID_C_RS232_MB_PRIORITY_5_ENABLE		_CFG_RS232(0x0150)  //T=u8, D=0, modbus priority 5 enable
#define CSID_C_RS232_MB_PRIORITY_1_ITEM			_CFG_RS232(0x0151)  //T=u8, D=0, modbus priority 1 item
#define CSID_C_RS232_MB_PRIORITY_2_ITEM			_CFG_RS232(0x0152)  //T=u8, D=0, modbus priority 2 item
#define CSID_C_RS232_MB_PRIORITY_3_ITEM			_CFG_RS232(0x0153)  //T=u8, D=0, modbus priority 3 item
#define CSID_C_RS232_MB_PRIORITY_4_ITEM			_CFG_RS232(0x0154)  //T=u8, D=0, modbus priority 4 item
#define CSID_C_RS232_MB_PRIORITY_5_ITEM			_CFG_RS232(0x0155)  //T=u8, D=0, modbus priority 5 item
#define CSID_C_RS232_MB_PRIORITY_1_IP	        _CFG_RS232(0x0156) //T=ipv4, modbus priority 1 ip
#define CSID_C_RS232_MB_PRIORITY_2_IP	        _CFG_RS232(0x0157) //T=ipv4, modbus priority 2 ip
#define CSID_C_RS232_MB_PRIORITY_3_IP	        _CFG_RS232(0x0158) //T=ipv4, modbus priority 3 ip
#define CSID_C_RS232_MB_PRIORITY_4_IP	        _CFG_RS232(0x0159) //T=ipv4, modbus priority 4 ip
#define CSID_C_RS232_MB_PRIORITY_5_IP	        _CFG_RS232(0x0160) //T=ipv4, modbus priority 5 ip
#define CSID_C_RS232_MB_PRIORITY_1_ID	        _CFG_RS232(0x0161) //T=u8, modbus priority 1 modbus id
#define CSID_C_RS232_MB_PRIORITY_2_ID	        _CFG_RS232(0x0162) //T=u8, modbus priority 2 modbus id
#define CSID_C_RS232_MB_PRIORITY_3_ID	        _CFG_RS232(0x0163) //T=u8, modbus priority 3 modbus id
#define CSID_C_RS232_MB_PRIORITY_4_ID	        _CFG_RS232(0x0164) //T=u8, modbus priority 4 modbus id
#define CSID_C_RS232_MB_PRIORITY_5_ID	        _CFG_RS232(0x0165) //T=u8, modbus priority 5 modbus id
#define CSID_C_RS232_MB_PRIORITY_1_CODE	        _CFG_RS232(0x0166) //T=u8, modbus priority 1 function code
#define CSID_C_RS232_MB_PRIORITY_2_CODE	        _CFG_RS232(0x0167) //T=u8, modbus priority 2 function code
#define CSID_C_RS232_MB_PRIORITY_3_CODE	        _CFG_RS232(0x0168) //T=u8, modbus priority 3 function code
#define CSID_C_RS232_MB_PRIORITY_4_CODE	        _CFG_RS232(0x0169) //T=u8, modbus priority 4 function code
#define CSID_C_RS232_MB_PRIORITY_5_CODE	        _CFG_RS232(0x0170) //T=u8, modbus priority 5 function code
#define CSID_C_RS232_MB_0Bh_Exception_ENABLE    _CFG_RS232(0x0171) //T=u8, modbus Enable 0Bh Exception
#define CSID_C_RS232_OPERATION_MODE			    _CFG_RS232(0x0172) //T=u8, 0:Disable 1:Virtual COM 2:Virtual Dialup 3:Modbus 4:IEC 60870-5
#define CSID_C_RS232_VC_OPERATION_MODE		    _CFG_RS232(0x0173) //T=u8, 1:TCP Client 2:TCP Server 3:UDP 4:RFC-2217
#define CSID_C_RS232_VD_OPERATION_MODE		    _CFG_RS232(0x0174) //T=u8,
#define CSID_C_RS232_MB_OPERATION_MODE		    _CFG_RS232(0x0175) //T=u8, 1:Gateway 2:Concentrator
#define CSID_C_RS232_INTERFACE		            _CFG_RS232(0x0176) //T=u8, D=1, 1:RS-232 2:RS-485
#define CSID_C_RS232_BAUDRATE  			      	_CFG_RS232(0x0177) //T=u32, D=9600, baudrate for communication com
#define CSID_C_RS232_DATA_BITS		        	_CFG_RS232(0x0178) //T=u8, D=8, data bit for communication com
#define CSID_C_RS232_PARITY				      	_CFG_RS232(0x0179) //T=u8, parity for communication com
#define CSID_C_RS232_DTRDSR                     _CFG_RS232(0x017A) //T=u8, D=1, to show the DTR/DSR option or not
#define CSID_C_RS232_STOP_BITS		        	_CFG_RS232(0x0180) //T=u8, stop bit for communication com
#define CSID_C_RS232_FLOWCTRL					_CFG_RS232(0x0181)  //T=u8,  D=0, set flow control, 0:None, 1:XON/XOFF, 2: RTS/CTS, 3: DTR/DSR
#define CSID_C_RS232_MB_MSG_BUFFER_ENABLE 	    _CFG_RS232(0x0182) //T=u8, modbus Enable message buffering
#define CSID_C_RS232_VC_TCP_CONNECTION_MAX		_CFG_RS232(0x0183)  //T=u8, D=1, maximun client connection (1-4)
#define CSID_C_RS232_VC_KEEPALIVE				_CFG_RS232(0x0184)  //T=u8, D=0, TCP keepalive setting
#define CSID_C_RS232_VC_IP_DOH_1_ENABLE			_CFG_RS232(0x0185)  //T=u8, D=0, when mode is TCP Client, address of server 1 enable
#define CSID_C_RS232_VC_IP_DOH_2_ENABLE			_CFG_RS232(0x0186)  //T=u8, D=0, when mode is TCP Client, address of server 2 enable
#define CSID_C_RS232_VC_IP_DOH_3_ENABLE			_CFG_RS232(0x0187)  //T=u8, D=0, when mode is TCP Client, address of server 3 enable
#define CSID_C_RS232_VC_IP_DOH_4_ENABLE			_CFG_RS232(0x0188)  //T=u8, D=0, when mode is TCP Client, address of server 4 enable
#define CSID_C_RS232_VC_CONNECTION_CTRL			_CFG_RS232(0x0189)  //T=u8, when mode is TCP Client, Always/On-Demand (default: Always)
#define CSID_C_RS232_VC_DOMAIN_NAME_OF_HOST_1	_CFG_RS232(0x018A)  //T=str, when mode is TCP Client, domain name of server 1
#define CSID_C_RS232_VC_DOMAIN_NAME_OF_HOST_2	_CFG_RS232(0x018B)  //T=str, when mode is TCP Client, domain name of server 2
#define CSID_C_RS232_VC_DOMAIN_NAME_OF_HOST_3	_CFG_RS232(0x018C)  //T=str, when mode is TCP Client, domain name of server 3
#define CSID_C_RS232_VC_DOMAIN_NAME_OF_HOST_4	_CFG_RS232(0x018D)  //T=str, when mode is TCP Client, domain name of server 4
#define CSID_C_RS232_VC_DOMAIN_OF_HOST_1		_CFG_RS232(0x0190)  //T=ipv4, when mode is TCP Client, address of server 1
#define CSID_C_RS232_VC_DOMAIN_OF_HOST_2		_CFG_RS232(0x0191)  //T=ipv4, when mode is TCP Client, address of server 2
#define CSID_C_RS232_VC_DOMAIN_OF_HOST_3		_CFG_RS232(0x0192)  //T=ipv4, when mode is TCP Client, address of server 3
#define CSID_C_RS232_VC_DOMAIN_OF_HOST_4		_CFG_RS232(0x0193)  //T=ipv4, when mode is TCP Client, address of server 4
#define CSID_C_RS232_VC_PORT_OF_HOST_1          _CFG_RS232(0x0194)  //T=u32, D=4001, port number of server 1 
#define CSID_C_RS232_VC_PORT_OF_HOST_2          _CFG_RS232(0x0195)  //T=u32, D=4001, port number of server 2
#define CSID_C_RS232_VC_PORT_OF_HOST_3          _CFG_RS232(0x0196)  //T=u32, D=4001, port number of server 3 
#define CSID_C_RS232_VC_PORT_OF_HOST_4          _CFG_RS232(0x0197)  //T=u32, D=4001, port number of server 4 
#define CSID_C_RS232_VC_PORT_OF_LOCAL_1         _CFG_RS232(0x0198)  //T=u32, port number of local client connected to server 1
#define CSID_C_RS232_VC_PORT_OF_LOCAL_2         _CFG_RS232(0x0199)  //T=u32, port number of local client connected to server 2
#define CSID_C_RS232_VC_PORT_OF_LOCAL_3         _CFG_RS232(0x0200)  //T=u32, port number of local client connected to server 3
#define CSID_C_RS232_VC_PORT_OF_LOCAL_4         _CFG_RS232(0x0201)  //T=u32, port number of local client connected to server 4
#define CSID_C_RS232_VC_IP_UDP_DOH_1_ENABLE		_CFG_RS232(0x0202)  //T=u8, D=0, when mode is UDP, address of server 1 enable
#define CSID_C_RS232_VC_IP_UDP_DOH_2_ENABLE		_CFG_RS232(0x0203)  //T=u8, D=0, when mode is UDP, address of server 2 enable
#define CSID_C_RS232_VC_IP_UDP_DOH_3_ENABLE		_CFG_RS232(0x0204)  //T=u8, D=0, when mode is UDP, address of server 3 enable
#define CSID_C_RS232_VC_IP_UDP_DOH_4_ENABLE		_CFG_RS232(0x0205)  //T=u8, D=0, when mode is UDP, address of server 4 enable
#define CSID_C_RS232_VC_UDP_IP_ADDR_1_From      _CFG_RS232(0x0206) //T=ipv4, virtual COM UDP mode IP ADDRESS 1 from
#define CSID_C_RS232_VC_UDP_IP_ADDR_1_To        _CFG_RS232(0x0207) //T=ipv4, virtual COM UDP mode IP ADDRESS 1 to
#define CSID_C_RS232_VC_UDP_IP_ADDR_2_From      _CFG_RS232(0x0208) //T=ipv4, virtual COM UDP mode IP ADDRESS 2 from
#define CSID_C_RS232_VC_UDP_IP_ADDR_2_To        _CFG_RS232(0x0209) //T=ipv4, virtual COM UDP mode IP ADDRESS 2 to
#define CSID_C_RS232_VC_UDP_IP_ADDR_3_From      _CFG_RS232(0x0210) //T=ipv4, virtual COM UDP mode IP ADDRESS 3 from
#define CSID_C_RS232_VC_UDP_IP_ADDR_3_To        _CFG_RS232(0x0211) //T=ipv4, virtual COM UDP mode IP ADDRESS 3 to
#define CSID_C_RS232_VC_UDP_IP_ADDR_4_From      _CFG_RS232(0x0212) //T=ipv4, virtual COM UDP mode IP ADDRESS 4 from
#define CSID_C_RS232_VC_UDP_IP_ADDR_4_To        _CFG_RS232(0x0213) //T=ipv4, virtual COM UDP mode IP ADDRESS 4 to
#define CSID_C_RS232_VC_UDP_REMOTE_PORT_1       _CFG_RS232(0x0214)  //T=u32, D=4001, port number connected to UDP server 1
#define CSID_C_RS232_VC_UDP_REMOTE_PORT_2       _CFG_RS232(0x0215)  //T=u32, D=4001, port number connected to UDP server 2
#define CSID_C_RS232_VC_UDP_REMOTE_PORT_3       _CFG_RS232(0x0216)  //T=u32, D=4001, port number connected to UDP server 3
#define CSID_C_RS232_VC_UDP_REMOTE_PORT_4       _CFG_RS232(0x0217)  //T=u32, D=4001, port number connected to UDP server 4
#define CSID_C_RS232_WAN_IFACE					_CFG_RS232(0x0218)  //T=u8, D=0, wan index

#define CSID_C_RS232_MB_GATEWAY_ENABLE		    _CFG_RS232(0x0219) //T=u8, D=1, modbus Enable gateway
#define CSID_C_RS232_MB_SLAVE_ENABLE		    _CFG_RS232(0x0220) //T=u8, D=0, modbus Enable slave
#define CSID_C_RS232_MB_SLAVE_IF_SERIAL_ENABLE	_CFG_RS232(0x0221) //T=u8, D=0, modbus slave Enable Serial Interface
#define CSID_C_RS232_MB_SLAVE_IF_TCP_NETWORK_ENABLE		_CFG_RS232(0x0222) //T=u8, D=0, modbus slave Enable TCP Network Interface
#define CSID_C_RS232_MB_SOURCE_IP_ITEM		_CFG_RS232(0x0230) //T=u8, D=0, 0:Specific IP Address 1:IP Range 2:IP Address-based group
#define CSID_C_RS232_MB_SOURCE_IP_START		_CFG_RS232(0x0240) //T=ipv4, modbus start of IP range or Specific IP Address
#define CSID_C_RS232_MB_SOURCE_IP_END		_CFG_RS232(0x0250) //T=ipv4, modbus end of IP range
#define CSID_C_RS232_MB_SOURCE_IP_GROUP		_CFG_RS232(0x0260) //T=str, modbus IP Address-based Group

#define CSID_C_RS232_MULTI_OPERATION_MODE			    _CFG_RS232(0x0270) //T=u8, D=0, 0:Disable 1:Virtual COM 2:Virtual Dialup 3:Modbus 4:IEC 60870-5
#define CSID_C_RS232_MULTI_SERIAL_INTERFACE	            _CFG_RS232(0x0280) //T=u8, D=1, 1:RS-232 2:RS-485
#define CSID_C_RS232_MULTI_SERIAL_BAUDRATE  	      	_CFG_RS232(0x0290) //T=u32, D=9600, baudrate for communication com
#define CSID_C_RS232_MULTI_SERIAL_DATA_BITS	        	_CFG_RS232(0x02A0) //T=u8, D=8, data bit for communication com
#define CSID_C_RS232_MULTI_SERIAL_PARITY		      	_CFG_RS232(0x02B0) //T=u8, parity for communication com
#define CSID_C_RS232_MULTI_SERIAL_STOP_BITS	        	_CFG_RS232(0x02C0) //T=u8, stop bit for communication com
#define CSID_C_RS232_MULTI_SERIAL_FLOWCTRL				_CFG_RS232(0x02D0) //T=u8, D=0, set flow control, 0:None, 1: RTS/CTS, 2: DTR/DSR
#define CSID_C_RS232_MB_MULTI_SERIAL_PROTOCOL			_CFG_RS232(0x02E0) //T=u8, D=1, modbus serial port protocol 1:RTU 2:ASCII
#define CSID_C_RS232_MB_MULTI_SERIAL_MODE				_CFG_RS232(0x02F0) //T=u8, D=0, modbus serial mode for gateway 0:Slave 1:Master
#define CSID_C_RS232_MB_REMOTE_SLAVE_IP					_CFG_RS232(0x0300) //T=ipv4, modbus remote slave ip
#define CSID_C_RS232_MB_REMOTE_SLAVE_PORT				_CFG_RS232(0x0400) //T=u32, modbus remote slave port
#define CSID_C_RS232_MB_REMOTE_SLAVE_ID_RANGE_START		_CFG_RS232(0x0500) //T=u8, modbus remote slave id range start
#define CSID_C_RS232_MB_REMOTE_SLAVE_ID_RANGE_END		_CFG_RS232(0x0600) //T=u8, modbus remote slave id range end
#define CSID_C_RS232_MB_LOCAL_SERIAL_PORT_ENABLE		_CFG_RS232(0x0700) //T=u32, modbus local serial port enable
#define CSID_C_RS232_MB_REMOTE_SLAVE_ENABLE		        _CFG_RS232(0x0800) //T=u8, D=1, modbus remote slave enable
#define CSID_C_RS232_VC_ITEM_NAME_OF_HOST_1				_CFG_RS232(0x0900) //T=u8, D=0, 0:IP Address, 1:Domain Name
#define CSID_C_RS232_VC_ITEM_NAME_OF_HOST_2				_CFG_RS232(0x0901) //T=u8, D=0, 0:IP Address, 1:Domain Name
#define CSID_C_RS232_VC_ITEM_NAME_OF_HOST_3				_CFG_RS232(0x0902) //T=u8, D=0, 0:IP Address, 1:Domain Name
#define CSID_C_RS232_VC_ITEM_NAME_OF_HOST_4				_CFG_RS232(0x0903) //T=u8, D=0, 0:IP Address, 1:Domain Name


#define CSID_C_RS232_IEC60870_MULTI_SERIAL_MODE		    _CFG_RS232(0x0A00) //T=u8, D=0, IEC60870-5 serial mode for gateway 0:Slave 1:Master
#define CSID_C_RS232_IEC60870_MULTI_SERIAL_TYPE			_CFG_RS232(0x0A10) //T=u8, D=0, IEC60870-5 serial type for gateway 0:Unbalanced 1:Balanced
#define CSID_C_RS232_IEC60870_104_T1					_CFG_RS232(0x0A20) //T=u8, D=15, IEC60870-5 104 T1 Timeout, unit is sec
#define CSID_C_RS232_IEC60870_104_T2					_CFG_RS232(0x0A21) //T=u8, D=10, IEC60870-5 104 T2 Timeout, unit is sec
#define CSID_C_RS232_IEC60870_104_T3					_CFG_RS232(0x0A22) //T=u8, D=20, IEC60870-5 104 T3 Timeout, unit is sec
#define CSID_C_RS232_IEC60870_104_ORI_ADDR				_CFG_RS232(0x0A23) //T=u8, D=1, IEC60870-5 104 Originator Address
#define CSID_C_RS232_IEC60870_104_CA_LEN				_CFG_RS232(0x0A24) //T=u8, D=2, IEC60870-5 104 Common Address Length
#define CSID_C_RS232_IEC60870_104_COT_LEN				_CFG_RS232(0x0A25) //T=u8, D=2, IEC60870-5 104 Cause of Transmission Length
#define CSID_C_RS232_IEC60870_104_IOA_LEN				_CFG_RS232(0x0A26) //T=u8, D=3, IEC60870-5 104 Information Object Address Length
#define CSID_C_RS232_IEC60870_104_LISTEN_PORT			_CFG_RS232(0x0A27) //T=u32, D=2404, IEC60870-5 104 Listen Port 
#define CSID_C_RS232_IEC60870_104_DES_ADDR				_CFG_RS232(0x0A28) //T=ipv4, IEC60870-5 104 Destination IP Address
#define CSID_C_RS232_IEC60870_104_DES_PORT				_CFG_RS232(0x0A29) //T=u32, D=2404, IEC60870-5 104 Destination Port
#define CSID_C_RS232_IEC60870_104_K_VALUE				_CFG_RS232(0x0A2A) //T=u8, D=12, IEC60870-5 104 k value
#define CSID_C_RS232_IEC60870_104_W_VALUE				_CFG_RS232(0x0A2B) //T=u8, D=8, IEC60870-5 104 w value
#define CSID_C_RS232_IEC60870_104_TCP_CONNECTION_MAX	_CFG_RS232(0x0A2C) //T=u8, D=4, IEC60870-5 104 Connection Max Number
#define CSID_C_RS232_IEC60870_104_TRUSTED_IP_ACCESS		_CFG_RS232(0x0A2D) //T=u8, D=0, Trusted IP Access, 0:Allow All, 1:Specific IPs
#define CSID_C_RS232_IEC60870_104_SOURCE_IP_ITEM		_CFG_RS232(0x0A30) //T=u8, D=0, 0:Specific IP Address 1:IP Range 2:IP Address-based group
#define CSID_C_RS232_IEC60870_104_SOURCE_IP_START		_CFG_RS232(0x0A40) //T=ipv4, IEC60870-5 104 start of IP range or Specific IP Address
#define CSID_C_RS232_IEC60870_104_SOURCE_IP_END			_CFG_RS232(0x0A50) //T=ipv4, IEC60870-5 104 end of IP range
#define CSID_C_RS232_IEC60870_104_SOURCE_IP_GROUP		_CFG_RS232(0x0A60) //T=str, IEC60870-5 104 IP Address-based Group
#define CSID_C_RS232_IEC60870_104_SOURCE_IP_ENABLE		_CFG_RS232(0x0A70) //T=u8, IEC60870-5 104 Source IP Enable

#define CSID_C_RS232_IEC60870_101_LINK_ADDR				_CFG_RS232(0x0A80) //T=u32, D=1, IEC60870-5 101 Link Address
#define CSID_C_RS232_IEC60870_101_LA_LEN				_CFG_RS232(0x0A81) //T=u8, D=2, IEC60870-5 101 Link Address Length
#define CSID_C_RS232_IEC60870_101_CA_LEN				_CFG_RS232(0x0A82) //T=u8, D=1, IEC60870-5 101 Common Address Length
#define CSID_C_RS232_IEC60870_101_COT_LEN				_CFG_RS232(0x0A83) //T=u8, D=1, IEC60870-5 101 Cause of Transmission Length
#define CSID_C_RS232_IEC60870_101_IOA_LEN				_CFG_RS232(0x0A84) //T=u8, D=2, IEC60870-5 101 Information Object Address Length
#define CSID_C_RS232_IEC60870_101_RESPONSE_TIMEOUT		_CFG_RS232(0x0A85) //T=u32, D=1000, IEC60870-5 101 Response Timeout, unit is ms
#define CSID_C_RS232_IEC60870_101_TIMEOUT_RETRIES		_CFG_RS232(0x0A86) //T=u8, D=0, IEC60870-5 101 Response Retries

/* data logging configuration */
#define CSID_C_RS232_DL_DATA_LOGGING_ENABLE		        _CFG_RS232(0x0A90) //T=u8, D=0, enable data logging
#define CSID_C_RS232_DL_STORAGE_DEVICE  		        _CFG_RS232(0x0A91) //T=u8, D=0, select data logging to storage device, 0:internal, 1:external 
#define CSID_C_RS232_DL_EXPORT_FILE_FORMAT		        _CFG_RS232(0x0A92) //T=u8, D=0, default CSV
#define CSID_C_RS232_DL_FTP_DOWNLOAD_ENABLE		        _CFG_RS232(0x0A93) //T=u8, D=0, enable FTP download
#define CSID_C_RS232_DL_FTP_DOWNLOAD_PREVIOUS	        _CFG_RS232(0x0A94) //T=u8, D=0, previous status FTP download
#define CSID_C_RS232_DL_SERVER_PORT		                _CFG_RS232(0x0A95) //T=u32, D=7001, data logging server port
#define CSID_C_RS232_DL_SERVER_ACTION                   _CFG_RS232(0x0A96) //T=u8, Data logging server action, 1:Start Record 0:Stop Recording
#define CSID_C_RS232_DL_STORAGE_DEVICE_PREVIOUS         _CFG_RS232(0x0A97) //T=u8, D=0, previous status of storage device 

/* data logging list */
#define CSID_C_RS232_DL_LIST_NAME		                _CFG_RS232(0x0B00) //T=str, data logging list name
#define CSID_C_RS232_DL_PROTOCOL_TYPE	                _CFG_RS232(0x0B40) //T=u8, 0:Modbus, 1:IEC  
#define CSID_C_RS232_DL_SLAVE_SERIAL_PORT		        _CFG_RS232(0x0B80) //T=u8, slave id location in slave serial port
#define CSID_C_RS232_DL_SLAVE_ID_RANDE_START		    _CFG_RS232(0x0BC0) //T=u8, from 1 to 247
#define CSID_C_RS232_DL_SLAVE_ID_RANDE_END	            _CFG_RS232(0x0C00) //T=u8, from 1 to 247
#define CSID_C_RS232_DL_PROXY_MODE_ENABLE		        _CFG_RS232(0x0C40) //T=u8, D=0, proxy mode enable
#define CSID_C_RS232_DL_MASTER_MONITORING_ITEM          _CFG_RS232(0x0C80) //T=u8, modbus master monitering item 1:IP Address 2: Local Serial Port
#define CSID_C_RS232_DL_MASTER_MONITORING_IP_ADDRESS    _CFG_RS232(0x0CC0) //T=ipv4, modbus master monitering IP address 
#define CSID_C_RS232_DL_MASTER_MONITORING_SERIAL_PORT   _CFG_RS232(0x0D00) //T=u8, modbus master monitering Local Serial Port
#define CSID_C_RS232_DL_PROXY_MODE_RULES_L              _CFG_RS232(0x0D40) //T=u32, rule name-1 to rule name-32 
#define CSID_C_RS232_DL_PROXY_MODE_RULES_H              _CFG_RS232(0x0D80) //T=u32, rule name-33 to rule name-64 
#define CSID_C_RS232_DL_LIST_ENABLE		            	_CFG_RS232(0x0DC0) //T=u8, D=0, data logging list enable
#define CSID_C_RS232_DL_FILE_SIZE	                    _CFG_RS232(0x0E00) //T=u32, file size (kilobyte) 
#define CSID_C_RS232_DL_LOG_FILE_NAME		            _CFG_RS232(0x0E40) //T=str, log file name
#define CSID_C_RS232_DL_SPLIT_FILE_ENABLE		        _CFG_RS232(0x0E80) //T=u8, D=0
#define CSID_C_RS232_DL_STORAGE_SPLIT_SIZE 	            _CFG_RS232(0x0EC0) //T=u32, D=200, data logging to storage split file size
#define CSID_C_RS232_DL_STORAGE_SPLIT_UNIT 	            _CFG_RS232(0x0F00) //T=u8, D=0, data logging to storage split file size unit, 0:KB, 1:MB 
/* proxy mode rule list */
#define CSID_C_RS232_DL_PROXY_NAME		                _CFG_RS232(0x0F40) //T=str, proxy mode rule list name
#define CSID_C_RS232_DL_PROXY_FUNCTION_NAME	            _CFG_RS232(0x0F80) //T=u8, read coils/discrete input/holding registers/input registers
#define CSID_C_RS232_DL_PROXY_START_ADDR                _CFG_RS232(0x0FC0) //T=u32, 0 to 65535 
#define CSID_C_RS232_DL_PROXY_NUMBERS                   _CFG_RS232(0x1000) //T=u32, 1 to 125 
#define CSID_C_RS232_DL_PROXY_POLL_TIME                 _CFG_RS232(0x1040) //T=u32, D=1000, 250 to 99999ms 

/* Multi serial port support */
#define CSID_C_RS232_VC_MULTI_OPERATION_MODE		    _CFG_RS232(0x1100)  //T=u8, 1:TCP Client 2:TCP Server 3:UDP 4:RFC-2217
#define CSID_C_RS232_VC_MULTI_SSL_ENABLE		    	_CFG_RS232(0x1110)  //T=u8, create ssl socket. 0:Disable, 1:Enable
#define CSID_C_RS232_VC_MULTI_LISTEN_PORT            	_CFG_RS232(0x1120)  //T=u32, D=4001, port number for virtual com port
#define CSID_C_RS232_VC_MULTI_TIP_ACCESS				_CFG_RS232(0x1130)  //T=u8, D=1, trusted ip access, 1:Allow All, 2:Specific IPs
#define CSID_C_RS232_VC_MULTI_CONNECTION_MAX			_CFG_RS232(0x1140)  //T=u8, D=1, maximum client connection (1-4) in TCP Server mode
#define CSID_C_RS232_VC_MULTI_CONNECTION_CTRL			_CFG_RS232(0x1150)  //T=u8, when mode is TCP Client, Always/On-Demand (default: Always)
#define CSID_C_RS232_VC_MULTI_IDLETIME	 				_CFG_RS232(0x1160)  //T=u32, D=0, Virtual COM connection idle time, unit is minute
#define CSID_C_RS232_VC_MULTI_KEEPALIVE					_CFG_RS232(0x1170)  //T=u8, D=0, TCP keepalive setting, unit is minute
#define CSID_C_RS232_VC_MULTI_TO_HOST_ITEM				_CFG_RS232(0x1180)  //T=u8, D=0, when mode is TCP Client, 0:IP Address, 1:Domain Name
#define CSID_C_RS232_VC_MULTI_TO_HOST_IP				_CFG_RS232(0x1190)  //T=ipv4, when mode is TCP Client, IP address of server
#define CSID_C_RS232_VC_MULTI_TO_HOST_DOMAIN			_CFG_RS232(0x11A0)  //T=str, when mode is TCP Client, domain name of server
#define CSID_C_RS232_VC_MULTI_TO_HOST_PORT         		_CFG_RS232(0x11B0)  //T=u32, D=4001, when mode is TCP Client, port number of server 
#define CSID_C_RS232_VC_MULTI_TO_HOST_SPORT_ENABLE 		_CFG_RS232(0x11C0)  //T=u32, when mode is TCP Client, serial port number enable
#define CSID_C_RS232_VC_MULTI_TO_HOST_ENABLE			_CFG_RS232(0x11D0)  //T=u8, D=0, when mode is TCP Client, definition enable
#define CSID_C_RS232_VC_MULTI_UDP_IP_ADDR_From     	 	_CFG_RS232(0x11E0)  //T=ipv4, virtual COM UDP mode IP ADDRESS from
#define CSID_C_RS232_VC_MULTI_UDP_IP_ADDR_To      	  	_CFG_RS232(0x11F0)  //T=ipv4, virtual COM UDP mode IP ADDRESS to
#define CSID_C_RS232_VC_MULTI_UDP_REMOTE_PORT     	  	_CFG_RS232(0x1200)  //T=u32, D=4001, port number connected to UDP server
#define CSID_C_RS232_VC_MULTI_UDP_SPORT_ENABLE 		    _CFG_RS232(0x1210)  //T=u32, when mode is UDP, serial port number enable
#define CSID_C_RS232_VC_MULTI_UDP_ENABLE				_CFG_RS232(0x1220)  //T=u8, D=0, when mode is UDP, definition enable
#define CSID_C_RS232_VC_MULTI_TCP_IP_ADDR_From      	_CFG_RS232(0x1230)  //T=ipv4, when mode is TCP Server, IP ADDRESS from
#define CSID_C_RS232_VC_MULTI_TCP_IP_ADDR_To        	_CFG_RS232(0x1240)  //T=ipv4, when mode is TCP Server, IP ADDRESS to	
#define CSID_C_RS232_VC_MULTI_TCP_SPORT_ENABLE			_CFG_RS232(0x1250)  //T=u32, when mode is TCP Server, serial port number enable
#define CSID_C_RS232_VC_MULTI_TCP_ENABLE				_CFG_RS232(0x1260)  //T=u8, D=0, when mode is TCP Server, definition enable
#define CSID_C_RS232_VC_MULTI_RFC_IP_ADDR_From       	_CFG_RS232(0x1270)  //T=ipv4, when mode is RFC-2217, IP ADDRESS from
#define CSID_C_RS232_VC_MULTI_RFC_IP_ADDR_To         	_CFG_RS232(0x1280)  //T=ipv4, when mode is RFC-2217, IP ADDRESS from
#define CSID_C_RS232_VC_MULTI_RFC_SPORT_ENABLE      	_CFG_RS232(0x1290)  //T=u32, when mode is RFC-2217, serial port number enable
#define CSID_C_RS232_VC_MULTI_RFC_ENABLE				_CFG_RS232(0x12A0)  //T=u8, D=0, when mode is RFC-2217, definition enable
#define CSID_C_RS232_MB_MULTI_LISTEN_PORT    	        _CFG_RS232(0x12B0)  //T=u32, D=502, port number for modbus tcp
#define CSID_C_RS232_MB_MULTI_TIP_SPORT_ENABLE 		    _CFG_RS232(0x12C0)  //T=u32, serial port number enable for modbus Trusted IP
#define CSID_C_RS232_IEC60870_MULTI_104_LISTEN_PORT		_CFG_RS232(0x12D0)  //T=u32, D=2404, IEC60870-5 104 Listen Port 
#define CSID_C_RS232_IEC60870_MULTI_104_DES_ADDR		_CFG_RS232(0x12E0)  //T=ipv4, IEC60870-5 104 Destination IP Address
#define CSID_C_RS232_IEC60870_MULTI_104_DES_PORT		_CFG_RS232(0x12F0)  //T=u32, D=2404, IEC60870-5 104 Destination Port
#define CSID_C_RS232_IEC60870_MULTI_TIP_SPORT_ENABLE 	_CFG_RS232(0x1300)  //T=u32, serial port number enable for IEC60870-5 Trusted IP
#define CSID_C_RS232_IEC60870_MULTI_104_ORI_ADDR		_CFG_RS232(0x1310) //T=u32, D=1, IEC60870-5 104 multi Originator Address
#define CSID_C_RS232_IEC60870_MULTI_101_LINK_ADDR		_CFG_RS232(0x1320) //T=u32, D=1, IEC60870-5 101 multi Link Address

/* Status Items */
#define CSID_S_RS232_ALTERED					_STA_RS232(0x0001)	//T=u32, ui altered
#define CSID_S_RS232_MODEM_STATUS				_STA_RS232(0x0002)	//T=u32, MODEM status 0:offline, 1:Ready.
#define CSID_S_RS232_DL_ALTERED					_STA_RS232(0x0003)	//T=u32, ui data logging altered
#define CSID_S_RS232_DL_MODBUS_ALTERED			_STA_RS232(0x0004)	//T=u32, data logging modbus altered
#define CSID_S_RS232_DL_IEC_ALTERED				_STA_RS232(0x0005)	//T=u32, data logging iec altered
#define CSID_S_RS232_DL_PASSWORD_ALTERED		_STA_RS232(0x0006)	//T=u32, data logging ftp password altered
#define CSID_S_RS232_DL_MODBUS_MULTI_ALTERED	_STA_RS232(0x0010)	//T=u32, data logging modbus altered for multi serial port(16)
#define CSID_S_RS232_IOMGT_MULTI_ALTERED		_STA_RS232(0x0020)	//T=u32, IO management altered for multi serial port(16)

#define CSID_S_RS232_CNT_TIME					_STA_RS232(0x0100)	//T=u32, connect time
#define CSID_S_RS232_IF_IP						_STA_RS232(0x0101)	//T=ipv4, dynamic interface ip
#define CSID_S_RS232_IF_NM						_STA_RS232(0x0102)	//T=ipv4, dynamic interface netmask
#define CSID_S_RS232_IF_GW						_STA_RS232(0x0103)	//T=ipv4, dynamic interface gateway
#define CSID_S_RS232_IF_PRIDNS					_STA_RS232(0x0104)	//T=ipv4, dynamic interface primary DNS
#define CSID_S_RS232_IF_SECDNS					_STA_RS232(0x0105)	//T=ipv4, dynamic interface secondary DNS
#define CSID_S_RS232_CNT_STATUS					_STA_RS232(0x0200)	//T=u16, connect status
#define CSID_S_RS232_ERR_CODE					_STA_RS232(0x0201)	//T=u16, connect fail error code
#define CSID_S_RS232_PKTIN_NUM					_STA_RS232(0x0300)	//T=u32, number of packet input
#define CSID_S_RS232_PKTOUT_NUM					_STA_RS232(0x0301)	//T=u32, number of packet output
#define CSID_S_RS232_PKTIN_NUM_VCP				_STA_RS232(0x0302)	//T=u32, number of packet input, using for Virtual Com Port
#define CSID_S_RS232_PKTOUT_NUM_VCP    			_STA_RS232(0x0303)	//T=u32, number of packet output, using for Virtual Com Port
#define CSID_S_RS232_ONDEMAND_STATUS			_STA_RS232(0x0400)	//T=u16, demend wan type status for script so it don't care for UI
#define CSID_S_RS232_AUTOBAK_STATUS				_STA_RS232(0x0500)	//T=u16, 3g failover status
#define CSID_S_RS232_SERIAL_ALTERED         	_STA_RS232(0x0600)  //T=u32, ui altered
#define CSID_S_RS232_SERIAL_BAUDRATE        	_STA_RS232(0x0700)  //T=u32, baudrate for virtual com port
#define CSID_S_RS232_SERIAL_PARITY_BIT      	_STA_RS232(0x0701)  //T=u32, parity for virtual com port
#define CSID_S_RS232_SERIAL_DATA_BIT        	_STA_RS232(0x0702)  //T=u32, data bit for virtual com port
#define CSID_S_RS232_SERIAL_STOP_BIT        	_STA_RS232(0x0703)  //T=u32, stop bit for virtual com port
#define CSID_S_RS232_SERIAL_PROTOCOL        	_STA_RS232(0x0704)  //T=u32, protocol for virtual com port, 1:TCP, 2:UDP, 3:RFC2217
#define CSID_S_RS232_SERIAL_PROTOCOL_MODE   	_STA_RS232(0x0705)  //T=u32, protocol mode for virtual com port, 1:SERVER, 2:ClIENT
#define CSID_S_RS232_SERIAL_SERVER_IP       	_STA_RS232(0x0706)  //T=ipv4, protocol mode client ip for virtual com port
#define CSID_S_RS232_SERIAL_BAUDRATE_IN     	_STA_RS232(0x0707)  //T=u32, incoming baudrate for virtual com port
#define CSID_S_RS232_SERIAL_BAUDRATE_OUT    	_STA_RS232(0x0708)  //T=u32, output baudrate for virtual com port
#define CSID_S_RS232_SERIAL_RFC2217_IDLETIME  	_STA_RS232(0x0709)  //T=u32, D=10, RFC2217 waiting data transmission idle time, unit is minute
#define CSID_S_RS232_SERIAL_PRE_BAUDRATE 		_STA_RS232(0x0710)  //T=u32, D=0, show present baudrate set by stty 
#define CSID_S_RS232_SERIAL_FLOWCONTROL_MODE	_STA_RS232(0x0711)  //T=u8,  D=0, set flow control, 0:None, 1: XON/XOFF, 2: RTS/CTS
#define CSID_S_RS232_I2C_GPOA_DATA    			_STA_RS232(0x0712)  //T=u32, D=0, MCP23017 I2C GPOA data
#define CSID_S_RS232_I2C_GPOB_DATA              _STA_RS232(0x0713)  //T=u32, D=0, MCP23017 I2C GPOB data
#define CSID_S_RS232_CMD_STATUS                 _STA_RS232(0x0714)  //T=u32, dialup command status
#define CSID_S_RS232_IOMGT_ALTERED              _STA_RS232(0x0715)  //T=u8, IO management altered
#define CSID_S_RS232_DL_SERVER_WORKING          _STA_RS232(0x0716)  //T=u8, Data logging server working status
#define CSID_S_RS232_DL_WRITE_DATA_DISABLE      _STA_RS232(0x0717)  //T=u8, D=0, 0:Enable write internal data 1:Disable write data/disk is full
#define CSID_S_RS232_MB_DI_STATUS               _STA_RS232(0x0720)  //T=u8, D=0, 16 groups of DI status for modbus slave, 0: OFF, 1:ON
#define CSID_S_RS232_MB_DO_STATUS               _STA_RS232(0x0730)  //T=u8, D=0, 16 groups of DO status for modbus slave, 0: OFF, 1:ON
#define CSID_S_RS232_IOMGT_MB_EVENT             _STA_RS232(0x0800)  //T=u8, IO management event from modbus
#define CSID_S_RS232_IOMGT_MB_ACTION            _STA_RS232(0x0900)  //T=u8, IO management action to modbus

#define CSID_S_RS232_SERIAL_PORT_NAME			_STA_RS232(0x0A00)	//T=str, serial device name
#define CSID_S_RS232_SERIAL_PORT_PROTOCOL		_STA_RS232(0x0A10)	//T=u8, 0:Both, 1:RS-232, 2:RS-485
#define CSID_S_RS232_SERIAL_PORT_MODE			_STA_RS232(0x0A20)	//T=u8, 0:Full, 1:Lite
#define CSID_S_RS232_SERIAL_PORT_MANUFACTURING	_STA_RS232(0x0A30)	//T=u8, 0:PL2303, 1:CPU_Uart, 2:CP210x
#define CSID_S_RS232_SERIAL_CP210X_INDEX		_STA_RS232(0x0A40)	//T=u8, CP210x serial index


#define CSID_S_RS232_WAN_IP						_STA_RS232(0x1000) //T=ipv4, wan ip
#define CSID_S_RS232_WAN_GW						_STA_RS232(0x2100) //T=ipv4, wan gateway
#define CSID_S_RS232_WAN_IF_NAME				_STA_RS232(0x2200) //T=str, the interface name of wan, ppp0, eth1, ixp1

#endif //ifndef _CSID_RS232_H
