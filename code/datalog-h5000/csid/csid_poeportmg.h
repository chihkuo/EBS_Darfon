/*
 * @file Member ID Definition of Group POEPORTMG.
 *
 * Moderator: Andy_Gu
 * Group ID: 0x00B80000/0x80B80000
 */

#ifndef _CSID_POEPORTMG_H
#define _CSID_POEPORTMG_H

#include "csid_gid.h"

#define _CFG_POEPORTMG(x)		(CSID_GIDC_POEPORTMG|(x))
#define _STA_POEPORTMG(x)		(CSID_GIDS_POEPORTMG|(x))

/* Configuration Items */
#define CSID_C_POEPORTMG_ENABLE						_CFG_POEPORTMG(0x0001) //T=u8, D=0, enable PoE port management, 1:enable, 0:disable
#define CSID_C_POEPORTMG_PORT_COUNT					_CFG_POEPORTMG(0x0002) //T=u8, PoE port count
#define CSID_C_POEPORTMG_DATASTORAGE_ENABLE			_CFG_POEPORTMG(0x0003) //T=u8, D=0, enable PoE port scheduling action result to storage, 1:enable, 0:disable 
#define CSID_C_POEPORTMG_DATASTORAGE_DEVICE			_CFG_POEPORTMG(0x0004) //T=u8, D=0, PoE port scheduling action result storage device, 0:NONE, 1:external 

#define CSID_C_POEPORTMG_PORT_PING_IP 		_CFG_POEPORTMG(0x0010) //T=ipv4, PoE port Auto-Restart ping IP address

#define CSID_C_POEPORTMG_PORT_TIME_INTERVAL 		_CFG_POEPORTMG(0x0020) //T=u32, PoE port Auto-Restart ping time interval in seconds

#define CSID_C_POEPORTMG_PORT_FAIL_THRESHOLD 		_CFG_POEPORTMG(0x0030) //T=u32, PoE port Auto-Restart ping fail threshold 

#define CSID_C_POEPORTMG_PORT_AUTORESTART_ENABLE		_CFG_POEPORTMG(0x0040) //T=u8, D=0, PoE port Auto-Restart enable, 1:enable, 0:disable

#define CSID_C_POEPORTMG_PORT_ACTION				_CFG_POEPORTMG(0x0050) //T=u8, PoE port action, 1:ON, 0:OFF, 2:RESTART

#define CSID_C_POEPORTMG_PORT_ACTION_ENABLE		_CFG_POEPORTMG(0x0060) //T=u8, D=0, PoE port action enable, 1:enable, 0:disable

#define CSID_C_POEPORTMG_PORT_BEFORE_REBOOT_STATUS		_CFG_POEPORTMG(0x0070) //T=u8, PoE port before reboot status, 1:ON, 0:OFF

#define CSID_C_POEPORTMG_PORT_SCHEDULING_RULE_NAME		_CFG_POEPORTMG(0x0080) //T=str, PoE port scheduling rule name

#define CSID_C_POEPORTMG_PORT_SCHEDULING_RULE		_CFG_POEPORTMG(0x00C0) //T=u32, number of using rule for PoE port scheduling

#define CSID_C_POEPORTMG_PORT_SCHEDULING_SELECT		_CFG_POEPORTMG(0x0100) //T=u8, bit-mapped port of select for PoE port scheduling

#define CSID_C_POEPORTMG_PORT_SCHEDULING_ASSIGN_ACTION	_CFG_POEPORTMG(0x0140) //T=u8, PoE port scheduling assign action, 1:ON, 0:OFF, 2:RESTART, 3:ON-OFF, 4:OFF-ON

#define CSID_C_POEPORTMG_PORT_SCHEDULING_ENABLE		_CFG_POEPORTMG(0x0180) //T=u8, PoE port scheduling rule enable, 1:enable, 0:disable

/* Status Items */
#define CSID_S_POEPORTMG_ALTERED             	_STA_POEPORTMG(0x0001) //T=u8, PoE port management ui altered
#define CSID_S_POEPORTMG_PORT_STATUS             	_STA_POEPORTMG(0x0002) //T=u8, PoE port status, 1:ON, 0:OFF
#define CSID_S_POEPORTMG_PORT_SCHEDULING_IN_SCH_STATUS		_STA_POEPORTMG(0x0010) //T=u8, PoE port scheduling rule in_sch status, 0:not in schedule, 1:in schedule
#endif //ifndef _CSID_POEPORTMG_H
