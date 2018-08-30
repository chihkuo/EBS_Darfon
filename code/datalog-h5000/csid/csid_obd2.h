/*
 * @file Member ID Definition of Group OBD2.
 *
 * Moderator: JamieKuo
 * Group ID: 0x00B70000/0x80B70000
 */

#ifndef _CSID_OBD2_H
#define _CSID_OBD2_H

#include "csid_gid.h"

#define _CFG_OBD2(x)         (CSID_GIDC_OBD2|(x))
#define _STA_OBD2(x)         (CSID_GIDS_OBD2|(x))

/* Configuration Items */
#define CSID_C_OBD2_ENABLE			_CFG_OBD2(0x0001) //T=u8, OBD2 Enable, 0:No 1:Yes


/* selected MODE and PID */
#define CSID_C_OBD2_MODE			_CFG_OBD2(0x0010) //T=u32, OBD2 MODE
#define CSID_C_OBD2_MODE1_PID		_CFG_OBD2(0x0011) //T=str, OBD2 MODE1 PID
#define CSID_C_OBD2_MODE2_PID		_CFG_OBD2(0x0012) //T=str, OBD2 MODE2 PID
#define CSID_C_OBD2_MODE9_PID		_CFG_OBD2(0x0013) //T=str, OBD2 MODE9 PID
#define CSID_C_OBD2_RM_HOST_ENABLE	_CFG_OBD2(0x0014) //T=u32, Remote Host Enable


/* Remote Host*/
#define CSID_C_OBD2_RM_HOSTNAME      _CFG_OBD2(0x0020) //T=str, Remote Host Name
#define CSID_C_OBD2_RM_HOSTIP        _CFG_OBD2(0x0030) //T=ipv4, Remote Host IP
#define CSID_C_OBD2_RM_PROTOCOL      _CFG_OBD2(0x0040) //T=u8, Transmission protocol, 0:TCP 1:UDP
#define CSID_C_OBD2_RM_PORTNUM       _CFG_OBD2(0x0050) //T=u32, Transmission port number, 1~65535
#define CSID_C_OBD2_RM_INTERVAL      _CFG_OBD2(0x0060) //T=u32, Transmission interval, min is 1 second.
#define CSID_C_OBD2_RM_PREFIX_MSG    _CFG_OBD2(0x0070) //T=str, Prefix Message of the packet
#define CSID_C_OBD2_RM_SUFFIX_MSG    _CFG_OBD2(0x0080) //T=str, Suffix Message of the packet

/* Data to Storage */
#define CSID_C_OBD2_DATA_ENABLE          _CFG_OBD2(0x0100) //T=u8, Data to Storage Enable, 0:No 1:Yes
#define CSID_C_OBD2_DATA_DEVICE          _CFG_OBD2(0x0101) //T=u8, Select a Device to store data, 0:Internal 1:External
#define CSID_C_OBD2_DATA_FORMAT          _CFG_OBD2(0x0102) //T=str, Data format, RAW GPX
#define CSID_C_OBD2_DATA_FILENAME        _CFG_OBD2(0x0103) //T=str, Data file name, Default is XXX_yyyyMMddhhmm.log ;XXX depend on Data foramt field; ex:GPX
#define CSID_C_OBD2_DATA_SPLIT_ENABLE    _CFG_OBD2(0x0104) //T=u8, Enable to split data, 0:Disable 1:Enable
#define CSID_C_OBD2_DATA_SPLIT_SIZE      _CFG_OBD2(0x0105) //T=u32, Split size, Default is 200KB
#define CSID_C_OBD2_DATA_SPLIT_UNIT      _CFG_OBD2(0x0106) //T=u8, Split size unit, 0:KB 1:MB


/* Status Items */
#define CSID_S_OBD2_ALTERED          _STA_OBD2(0x0001) //T=u8, ui altered

#endif //ifndef _CSID_OBD2_H
