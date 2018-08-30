/*
 * @file Member ID Definition of Group PDUMG.
 *
 * Moderator: Andy_Gu
 * Group ID: 0x00B50000/0x80B50000
 */

#ifndef _CSID_PDUMG_H
#define _CSID_PDUMG_H

#include "csid_gid.h"

#define _CFG_PDUMG(x)		(CSID_GIDC_PDUMG|(x))
#define _STA_PDUMG(x)		(CSID_GIDS_PDUMG|(x))

/* Configuration Items */
#define CSID_C_PDUMG_ENABLE						_CFG_PDUMG(0x0001) //T=u8, D=0, enable pdu management, 1:enable, 0:disable
#define CSID_C_PDUMG_OUTLET_COUNT				_CFG_PDUMG(0x0002) //T=u8, D=2, pdu outlet count
#define CSID_C_PDUMG_OUTLET_FOR_GATEWAY			_CFG_PDUMG(0x0003) //T=u8, D=1, pdu outlet for gateway, 0:NONE, 1:OUTLET1, 2:OUTLET2
#define CSID_C_PDUMG_DATASTORAGE_ENABLE			_CFG_PDUMG(0x0004) //T=u8, D=0, enable pdu scheduling action result to storage, 1:enable, 0:disable 
#define CSID_C_PDUMG_DATASTORAGE_DEVICE			_CFG_PDUMG(0x0005) //T=u8, D=0, pdu scheduling action result storage device, 0:NONE, 1:external 

#define CSID_C_PDUMG_OUTLET_PING_IP 		_CFG_PDUMG(0x0010) //T=ipv4, pdu outlet Auto-Restart ping IP address

#define CSID_C_PDUMG_OUTLET_AUTORESTART_ENABLE_STATUS		_CFG_PDUMG(0x0040) //T=u8, pdu outlet Auto-Restart enable status, 1:enable, 0:disable

#define CSID_C_PDUMG_OUTLET_AUTORESTART_ENABLE		_CFG_PDUMG(0x0050) //T=u8, D=0, pdu outlet Auto-Restart enable, 1:enable, 0:disable

#define CSID_C_PDUMG_OUTLET_ACTION				_CFG_PDUMG(0x0060) //T=u8, D=1, pdu outlet action, 1:ON, 0:OFF, 2:RESTART

#define CSID_C_PDUMG_OUTLET_ACTION_ENABLE		_CFG_PDUMG(0x0070) //T=u8, D=0, pdu outlet action enable, 1:enable, 0:disable



#define CSID_C_PDUMG_OUTLET_SCHEDULING_RULE_NAME		_CFG_PDUMG(0x0080) //T=str, pdu outlet scheduling rule name

#define CSID_C_PDUMG_OUTLET_SCHEDULING_RULE		_CFG_PDUMG(0x00C0) //T=u32, number of using rule for pdu outlet scheduling

#define CSID_C_PDUMG_OUTLET_SCHEDULING_SELECT		_CFG_PDUMG(0x0100) //T=u8, bit-mapped outlet of select for pdu outlet scheduling

#define CSID_C_PDUMG_OUTLET_SCHEDULING_ASSIGN_ACTION	_CFG_PDUMG(0x0140) //T=u8, pdu outlet scheduling assign action, 1:ON, 0:OFF, 2:RESTART, 3:ON-OFF, 4:OFF-ON

#define CSID_C_PDUMG_OUTLET_SCHEDULING_ENABLE		_CFG_PDUMG(0x0180) //T=u8, pdu outlet scheduling rule enable, 1:enable, 0:disable

#define CSID_C_PDUMG_OUTLET_SCHEDULING_IN_SCH_STATUS		_CFG_PDUMG(0x0200) //T=u8, pdu outlet scheduling rule in_sch status, 0:not in schedule, 1:in schedule

/* Status Items */
#define CSID_S_PDUMG_ALTERED             	_STA_PDUMG(0x0001) //T=u8, pdu management ui altered
#define CSID_S_PDUMG_OUTLET_STATUS             	_STA_PDUMG(0x0002) //T=u8, pdu outlet status, 1:ON, 0:OFF, -1:not available, -2:powr lost or Breaker triggered
#define CSID_S_PDUMG_OUTLET_SCHEDULING_STATUS		_STA_PDUMG(0x0010) //T=u8, pdu outlet scheduling rule status, 0:none, 1:start time, 2:end time
#endif //ifndef _CSID_PDUMG_H
