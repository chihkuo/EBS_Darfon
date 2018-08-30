/*
 * @file Member ID Definition of Group I/O Management.
 *
 * Moderator: Mike
 * Group ID: 0x003B0000/0x803B0000
 */

#ifndef _CSID_IOMGT_H
#define _CSID_IOMGT_H

#include "csid_gid.h"

#define _CFG_IOMGT(x)			(CSID_GIDC_IOMGT|(x))
#define _STA_IOMGT(x)			(CSID_GIDS_IOMGT|(x))

#define IOMGT_EVENT_NONE 	0
#define IOMGT_EVENT_DI 		1
#define IOMGT_EVENT_SMS 	2
#define IOMGT_EVENT_PWRCHG 	3
#define IOMGT_EVENT_MODBUS 	4

#define IOMGT_HANDLER_NONE 	0
#define IOMGT_HANDLER_DO 	1
#define IOMGT_HANDLER_SMS 	2
#define IOMGT_HANDLER_SYSLOG 	3
#define IOMGT_HANDLER_SNMP 	4
#define IOMGT_HANDLER_EMAIL 	5
#define IOMGT_HANDLER_REBOOT 	6
#define IOMGT_HANDLER_MODBUS 	7

#define MODBUS_CMP_NONE 	0
#define MODBUS_CMP_GT 		1
#define MODBUS_CMP_EQ 		2
#define MODBUS_CMP_LT 		3

#define IOMGT_RULE_NUM 		16

/* Configuration Items */
#define CSID_C_IOMGT_ENABLE 			_CFG_IOMGT(0x0001) //T=u8, D=0, enable event handler
#define CSID_C_IOMGT_NOTIFYING_EVENTS_ENABLE        _CFG_IOMGT(0x0002) //T=u8, D=0, Notifying Events Enable
#define CSID_C_IOMGT_MANAGING_EVENTS_ENABLE         _CFG_IOMGT(0x0003) //T=u8, D=0, Managing Events Enable

/* DI & DO & VIN Source GPIO Definition */
#define CSID_C_IOMGT_DI_SOURCE_GPIO                 _CFG_IOMGT(0x0010) //T=u32, D=0, The DI Source of GPIO 
#define CSID_C_IOMGT_DO_SOURCE_GPIO                 _CFG_IOMGT(0x0020) //T=u32, D=0, The DO Source of GPIO
#define CSID_C_IOMGT_VIN_SOURCE_GPIO                _CFG_IOMGT(0x0030) //T=u32, D=0, The VIN Source of GPIO

#define CSID_C_IOMGT_RULE_ENABLE 		_CFG_IOMGT(0x0100) //T=u8, D=0, control each rule of event handler
#define CSID_C_IOMGT_EVENT_TYPE 		_CFG_IOMGT(0x0200) //T=u8, D=0, event type, default none
#define CSID_C_IOMGT_HANDLER_TYPE 		_CFG_IOMGT(0x0300) //T=u8, D=0, handler type, default none
#define CSID_C_IOMGT_RULE_SCHE_NO 		_CFG_IOMGT(0x0400) //T=u16, number of schedule rule


#define CSID_C_IOMGT_EVENT_SMS_MSG 		_CFG_IOMGT(0x3000) //T=str, D="", message to trigger handler

/* Not used */
#define CSID_C_IOMGT_EVENT_MODBUS_DEV 		_CFG_IOMGT(0x3100) //T=u32, D=0, device ID for comparison
#define CSID_C_IOMGT_EVENT_MODBUS_REG 		_CFG_IOMGT(0x3200) //T=u32, D=0, register for comparison
#define CSID_C_IOMGT_EVENT_MODBUS_CMP 		_CFG_IOMGT(0x3300) //T=u8, D=0, logic comparator for comparison
#define CSID_C_IOMGT_EVENT_MODBUS_THR 		_CFG_IOMGT(0x3400) //T=u32, threshold for comparison

/* Notifying Events Configuration */ 
#define CSID_C_IOMGT_NOTIFYING_EVTS_GROUPING_MEM_EVENT 	    _CFG_IOMGT(0x4000) //T=str, Notifying Events Grouping Member [Event, Ex: "@DI-1|On--> Off@DI-2|Off--> On"]
#define CSID_C_IOMGT_NOTIFYING_EVTS_GROUPING_MEM_HANDLERS    _CFG_IOMGT(0x4100) //T=str, Notifying Events Grouping Member [Handlers, Ex: "@DO|1@SMS|2"]
#define CSID_C_IOMGT_NOTIFYING_EVTS_GROUPING_MEM_RULE_ENABLE _CFG_IOMGT(0x4200) //T=u32, Notifying Events Grouping Member [Rule Enable]

/* Managing Events Configuration */
#define CSID_C_IOMGT_MANAGING_EVTS_GROUPING_MEM_EVENT 	    _CFG_IOMGT(0x4800) //T=str, Managing Events Grouping Member [Event, Ex: "@DI|1@SMS|Test"]
#define CSID_C_IOMGT_MANAGING_EVTS_GROUPING_MEM_HANDLERS     _CFG_IOMGT(0x4900) //T=str, Managing Events Grouping Member [Handlers, Ex: "@DO|1@SMS|2"]
#define CSID_C_IOMGT_MANAGING_EVTS_GROUPING_MEM_RESPONSE	    _CFG_IOMGT(0x4A00) //T=str, Managing Events Grouping Member [Response, Ex: "@None@DO|1"]
#define CSID_C_IOMGT_MANAGING_EVTS_GROUPING_MEM_RULE_ENABLE  _CFG_IOMGT(0x4B00) //T=u32, Managing Events Grouping Member [Rule Enable]


/* SMS Account Handler Configuration */
#define CSID_C_IOMGT_HANDLER_SMS_PNO 		_CFG_IOMGT(0x6000) //T=str, D="", phone number
#define CSID_C_IOMGT_HANDLER_SMS_MSG 		_CFG_IOMGT(0x6100) //T=str, D="", message

#define CSID_C_IOMGT_HANDLER_SYSLOG_SVR 	_CFG_IOMGT(0x6200) //T=u32, D=0, server
#define CSID_C_IOMGT_HANDLER_SYSLOG_MSG 	_CFG_IOMGT(0x6300) //T=str, D="", message

#define CSID_C_IOMGT_HANDLER_SNMP_TRIP_INDEX 	_CFG_IOMGT(0x7000) //T=u32, D=0, snmp trap receiver, get IP from CSID_C_SNMP_TRIP_IP, value is from 1, 2, 3, 4
#define CSID_C_IOMGT_HANDLER_SNMP_MSG 		_CFG_IOMGT(0x6400) //T=str, D="", snmp trap message

/* Email Service Handler Configuration */
#define CSID_C_IOMGT_HANDLER_EMAIL_SVR 		_CFG_IOMGT(0x6500) //T=u32, D=0, server
#define CSID_C_IOMGT_HANDLER_EMAIL_ADD 		_CFG_IOMGT(0x6600) //T=str, D="", address
#define CSID_C_IOMGT_HANDLER_EMAIL_MSG 		_CFG_IOMGT(0x6700) //T=str, D="", message

/* Modbus Handler Definition Configuration */
#define CSID_C_IOMGT_HANDLER_MODBUS_DEV 	_CFG_IOMGT(0x6800) //T=u32, D=1, device ID (Range: 1 ~ 247)
#define CSID_C_IOMGT_HANDLER_MODBUS_REG 	_CFG_IOMGT(0x6900) //T=u32, D=0, register (Range: 0 ~ 65535)
#define CSID_C_IOMGT_HANDLER_MODBUS_VAL 	_CFG_IOMGT(0x6A00) //T=u32, D=0, value used to set (Range: 0 ~ 65535)
#define CSID_C_IOMGT_HANDLER_MODBUS_CMP     _CFG_IOMGT(0x6B00) //T=u32, D=1, Logic Comparator (1 ~ 5 : > , < , = , >= , <=)
#define CSID_C_IOMGT_HANDLER_MODBUS_NAME    _CFG_IOMGT(0x6C00) //T=str, Modbus Name
#define CSID_C_IOMGT_HANDLER_MODBUS_MODE    _CFG_IOMGT(0x6D00) //T=u32, D=0, Modbus Mode, 0: Serial, 1: TCP
#define CSID_C_IOMGT_HANDLER_MODBUS_IP      _CFG_IOMGT(0x6E00) //T=ipv4, IP Address for TCP Modbus Mode
#define CSID_C_IOMGT_HANDLER_MODBUS_PORT    _CFG_IOMGT(0x6F00) //T=u32, Port for TCP Modbus Mode
#define CSID_C_IOMGT_HANDLER_MODBUS_READ_FUNCTION   _CFG_IOMGT(0x7100) //T=u32, D=3, Read Function code for Modbus
#define CSID_C_IOMGT_HANDLER_MODBUS_WRITE_FUNCTION  _CFG_IOMGT(0x7200) //T=u32, D=6, Write Function code for Modbus

/* DI Handler Configuration */
#define CSID_C_IOMGT_HANDLER_DI_PROFILE_NAME     _CFG_IOMGT(0x8000) //T=str, DI Profile Name
#define CSID_C_IOMGT_HANDLER_DI_SOURCE           _CFG_IOMGT(0x8100) //T=u32, D=0, DI Source (1 ~ n : DI-1 ~ DI-n)
#define CSID_C_IOMGT_HANDLER_DI_NORMAL_LEVEL     _CFG_IOMGT(0x8200) //T=u32, DI Normal Level (0 = Low, 1 = High)
#define CSID_C_IOMGT_HANDLER_DI_SIG_ACTIVE_TIME  _CFG_IOMGT(0x8300) //T=u32, DI Signal Active Time (1~ 10s), by default, 1 second

/* DO Handler Configuration */
#define CSID_C_IOMGT_HANDLER_DO_PROFILE_NAME     _CFG_IOMGT(0x8800) //T=str, DO Profile Name
#define CSID_C_IOMGT_HANDLER_DO_SOURCE           _CFG_IOMGT(0x8900) //T=u32, D=0, DO Source (1 ~ n : DO-1 ~ DO-n)
#define CSID_C_IOMGT_HANDLER_DO_NORMAL_LEVEL     _CFG_IOMGT(0x8A00) //T=u32, DO Normal Level (0 = Low, 1 = High)
#define CSID_C_IOMGT_HANDLER_DO_TOTAL_SIG_PERIOD _CFG_IOMGT(0x8B00) //T=u32, DO Total Signal Period (10~ 10000 ms)
#define CSID_C_IOMGT_HANDLER_DO_REPEAT_ENABLE    _CFG_IOMGT(0x8C00) //T=u32, DO Repeat Enable, 0: disable, 1: enable
#define CSID_C_IOMGT_HANDLER_DO_DUTY_CYCLE       _CFG_IOMGT(0x8D00) //T=u32, DO Duty Cycle, value = 1~100 (%)
#define CSID_C_IOMGT_HANDLER_DO_REPEAT_COUNTER   _CFG_IOMGT(0x8E00) //T=u32, D=0, DO Repeat Counter (0 ~ 9999) 

/* Modbus Handler Configuration */

/* SMS/Email/DI/DO/Modbus Handler Rule Enable */
#define CSID_C_IOMGT_HANDLER_SMS_RULE_ENABLE    _CFG_IOMGT(0xA000) //T=u32, SMS Account Rule Enable, 0: disable, 1: enable
#define CSID_C_IOMGT_HANDLER_EMAIL_RULE_ENABLE  _CFG_IOMGT(0xA100) //T=u32, Email Service Rule Enable, 0: disable, 1: enable
#define CSID_C_IOMGT_HANDLER_DI_PROFILE_ENABLE  _CFG_IOMGT(0xA200) //T=u32, DI Profile Enable, 0: disable, 1: enable
#define CSID_C_IOMGT_HANDLER_DO_PROFILE_ENABLE  _CFG_IOMGT(0xA300) //T=u32, DO Profile Enable, 0: disable, 1: enable
#define CSID_C_IOMGT_HANDLER_MODBUS_DEF_ENABLE  _CFG_IOMGT(0xA400) //T=u32, Modbus Definition Enable, 0: disable, 1: enable

/* SMS/Email/Modbus Handler Application Type */
#define CSID_C_IOMGT_HANDLER_SMS_APP_TYPE       _CFG_IOMGT(0xB000) //T=u32, D=0, SMS Account with Application Type, 0: none, 1: Managing Evts., 2: Notifying Evts., 3: Both
#define CSID_C_IOMGT_HANDLER_MODBUS_APP_TYPE    _CFG_IOMGT(0xB100) //T=u32, D=0, Modbus with Application Type, 0: none, 1: Managing Evts., 2: Notifying Evts., 3: Both


/* Status Items */
#define CSID_S_IOMGT_ALTERED             	_STA_IOMGT(0x0001) //T=u8, ui altered
#define CSID_S_IOMGT_CUR_POWER                  _STA_IOMGT(0x0011)  //T=u32, D=0, current power. 1: power 1; 2: power 2


#endif //ifndef _CSID_IOMGT_H
