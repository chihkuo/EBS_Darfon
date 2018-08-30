//#This section is for RELEASE IN SDK COPY
/*
 * @file Member ID Definition of Group DARFONLOGGER
 * 
 * Moderator: 
 * Group ID: 0x00A10000/0x80A10000
 */

#ifndef _CSID_DARFONLOGGER_H
#define _CSID_DARFONLOGGER_H

#include "csid_gid.h"

#define _CFG_DARFONLOGGER(x)     (CSID_GIDC_DARFONLOGGER|(x))	//save in ROM
#define _STA_DARFONLOGGER(x)     (CSID_GIDS_DARFONLOGGER|(x))	//save in RAM

#define CSID_S_DARFONLOGGER_RAM_ALTERED  							_STA_DARFONLOGGER(0x0000) //T=u8,0:none, 1:alter
#define CSID_S_DARFONLOGGER_RAM_ERROR_CHECK_INTERNET_CONNECTION 	_STA_DARFONLOGGER(0x0001) //T=u8, Internet Connection Error, 1=error, 0=nothing
#define CSID_S_DARFONLOGGER_RAM_ERROR_CHECK_RS485_CONNECTION 		_STA_DARFONLOGGER(0x0002) //T=u8, RS485 Error, 1=error, 0=nothing
#define CSID_S_DARFONLOGGER_RAM_SYSTEM_ONLINE_COUNT 				_STA_DARFONLOGGER(0x0003) //T=str, D="Scanning", str with default
#define CSID_S_DARFONLOGGER_RAM_SYSTEM_OFFLINE_COUNT 				_STA_DARFONLOGGER(0x0004) //T=str, D="Scanning", str with default
#define CSID_S_DARFONLOGGER_RAM_SYSTEM_ERROR_COUNT 					_STA_DARFONLOGGER(0x0005) //T=str, D="Scanning", str with default
#define CSID_S_DARFONLOGGER_RAM_SYSTEM_PAC  						_STA_DARFONLOGGER(0x0006) //T=str, D="Scanning", str with default
#define CSID_S_DARFONLOGGER_RAM_SYSTEM_ETOTAL  						_STA_DARFONLOGGER(0x0007) //T=str, D="Scanning", str with default
#define CSID_S_DARFONLOGGER_RAM_SYSTEM_ETODAY  						_STA_DARFONLOGGER(0x0008) //T=str, D="Scanning", str with default
#define CSID_S_DARFONLOGGER_RAM_SYSTEM_CO2  						_STA_DARFONLOGGER(0x0009) //T=str, D="Scanning", str with default
#define CSID_S_DARFONLOGGER_RAM_SYSTEM_MONEYSAVE  					_STA_DARFONLOGGER(0x000A) //T=str, D="Scanning", str with default
#define CSID_S_DARFONLOGGER_RAM_SYSTEM_TOTAL_NUM 					_STA_DARFONLOGGER(0x000B) //T=str, D="Scanning", str with default
#define CSID_S_DARFONLOGGER_RAM_MITABLE_CHANGE_ALTERED				_STA_DARFONLOGGER(0x000C) //T=u32,0:none, 1:alter
#define CSID_S_DARFONLOGGER_RAM_REGISTERMODE_CHANGE_ALTERED			_STA_DARFONLOGGER(0x000D) //T=u32,0:none, 1:alter



/* For SN, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_SN  								_STA_DARFONLOGGER(0x1100) //T=str
/* For Display Data, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_SHOW_DATA  						_STA_DARFONLOGGER(0x1200) //T=str
/* For ERROR MESSAGE, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_INVERTER_LASTERROR  				_STA_DARFONLOGGER(0x1300) //T=str
/* For COMMUNICATIONSTATUS, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_INVERTER_COMMUNICATIONSTATUS 	_STA_DARFONLOGGER(0x1400) //T=str
/* For MI_STATUS, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_STATUS  							_STA_DARFONLOGGER(0x1500) //T=u32,0:offline, 1:online, 2:error
/* For MI_PAC, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_PAC  							_STA_DARFONLOGGER(0x1600) //T=str
/* For MI_ETODAY, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_ETODAY  							_STA_DARFONLOGGER(0x1700) //T=str
/* For MI_ETOTAL, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_ETOTAL  							_STA_DARFONLOGGER(0x1800) //T=str
/* For MI_CO2, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_CO2  							_STA_DARFONLOGGER(0x1900) //T=str
/* For MI_MONEYSAVE, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_MONEYSAVE  						_STA_DARFONLOGGER(0x1A00) //T=str
/* For MI_EHOURS, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_EHOURS  							_STA_DARFONLOGGER(0x1B00) //T=str
/* For MI_LASTSAVEDATETIME, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_LASTSAVEDATETIME  				_STA_DARFONLOGGER(0x1C00) //T=str
/* For MI_SLAVEID, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_MI_SLAVEID  						_STA_DARFONLOGGER(0x1D00) //T=str

/* For Hybrid BMS Display Data, reserved 256 items */
#define CSID_S_DARFONLOGGER_RAM_HYRBRID_SHOW_BMS_DATA  				_STA_DARFONLOGGER(0x5000) //T=str









#define CSID_C_DARFONLOGGER_ROM_RULE_MODE  					_CFG_DARFONLOGGER(0x0001) //T=u8,0:MI, 1:HYB
#define CSID_C_DARFONLOGGER_ROM_RULE_CLEAR_DEVICE_TABLE  	_CFG_DARFONLOGGER(0x0002) //T=u8, D=0
#define CSID_C_DARFONLOGGER_ROM_MI_REGISTER_MODE  			_CFG_DARFONLOGGER(0x0003) //T=u8, 1:yes 0:no
#define CSID_C_DARFONLOGGER_ROM_MI_UPLOAD_MODE  			_CFG_DARFONLOGGER(0x0004) //T=u8, 1:low datausage 0:normal

/*#define CSID_C_DARFONLOGGER_ROM_DATALOGGER_SN  				_CFG_DARFONLOGGER(0x0005) //T=str, datalogger sn*/


/* For EnergyData, reserved 256 items */
#define CSID_C_DARFONLOGGER_ROM_RULE_MI_DATA  				_CFG_DARFONLOGGER(0x1100) //T=str, D="Default String", str with default
/* For MI_SN, reserved 256 items */
#define CSID_C_DARFONLOGGER_ROM_MI_SN 						_CFG_DARFONLOGGER(0x1200) //T=str, D="", str with default
/* For MI_ETOTAL, reserved 256 items */
#define CSID_C_DARFONLOGGER_ROM_MI_ETOTAL  					_CFG_DARFONLOGGER(0x1300) //T=str
/* For MI_EHOURS, reserved 256 items */
#define CSID_C_DARFONLOGGER_ROM_MI_EHOURS  					_CFG_DARFONLOGGER(0x1400) //T=str
/* For MI_LASTSAVEDATETIME, reserved 256 items */
#define CSID_C_DARFONLOGGER_ROM_MI_LASTSAVEDATETIME  		_CFG_DARFONLOGGER(0x1500) //T=str















#endif //ifndef _CSID_DARFONLOGGER_H
