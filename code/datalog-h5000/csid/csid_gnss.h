/*
 * @file Member ID Definition of Group GNSS.
 *
 * Moderator: ShirleyPan
 * Group ID: 0x00B60000/0x80B60000
 */

#ifndef _CSID_GNSS_H
#define _CSID_GNSS_H

#include "csid_gid.h"

#define _CFG_GNSS(x)         (CSID_GIDC_GNSS|(x))
#define _STA_GNSS(x)         (CSID_GIDS_GNSS|(x))

/* Configuration Items */
#define CSID_C_GNSS_ENABLE           _CFG_GNSS(0x0001) //T=u8, GNSS Enable, 0:No 1:Yes
#define CSID_C_GNSS_TYPE             _CFG_GNSS(0x0002) //T=u8, GNSS Type, 0:GPS
#define CSID_C_GNSS_INIT_STR         _CFG_GNSS(0x0003) //T=str, Initialization String
#define CSID_C_GNSS_GGA_MSG          _CFG_GNSS(0x0004) //T=u8, Enable GGA Message type, 0:Disable 1:Enable
#define CSID_C_GNSS_GLL_MSG          _CFG_GNSS(0x0005) //T=u8, Enable GLL Message type, 0:Disable 1:Enable
#define CSID_C_GNSS_GSA_MSG          _CFG_GNSS(0x0006) //T=u8, Enable GSA Message type, 0:Disable 1:Enable
#define CSID_C_GNSS_GSV_MSG          _CFG_GNSS(0x0007) //T=u8, Enable GSV Message type, 0:Disable 1:Enable
#define CSID_C_GNSS_RMC_MSG          _CFG_GNSS(0x0008) //T=u8, Enable RMC Message type, 0:Disable 1:Enable
#define CSID_C_GNSS_VTG_MSG          _CFG_GNSS(0x0009) //T=u8, Enable VTG Message type, 0:Disable 1:Enable
#define CSID_C_GNSS_OTHER_MSG        _CFG_GNSS(0x000a) //T=u8, Enable All Other Message type, 0:Disable 1:Enable

/* Assisted-GPS */
#define CSID_C_GNSS_AGPS_ENABLE      _CFG_GNSS(0x000b) //T=u8, AGPS Enable, 0:No 1:Yes
#define CSID_C_GNSS_AGPS_ALPFILE     _CFG_GNSS(0x000c) //T=u8, Select a Alp file name, 1/2/3/5/7/10/14 day Differential Almanac
#define CSID_C_GNSS_AGPS_ALP_IDX     _CFG_GNSS(0x000e) //T=u32, For alpserver, record upgrade index
#define CSID_C_GNSS_ALP_STARTTIME    _CFG_GNSS(0x000f) //T=u32, For upgrade alp file automatically, need to record last upgrade time

/* SBAS */
#define CSID_C_GNSS_SBAS_ENABLE      _CFG_GNSS(0x000d) //T=u8, SBAS Enable, 0:No 1:Yes

/* Data to Storage */
#define CSID_C_GNSS_DATA_ENABLE          _CFG_GNSS(0x0100) //T=u8, Data to Storage Enable, 0:No 1:Yes
#define CSID_C_GNSS_DATA_DEVICE          _CFG_GNSS(0x0101) //T=u8, Select a Device to store data, 0:Internal 1:External
#define CSID_C_GNSS_DATA_FORMAT          _CFG_GNSS(0x0102) //T=str, Data format, RAW/GPX
#define CSID_C_GNSS_DATA_FILENAME        _CFG_GNSS(0x0103) //T=str, Data file name, Default is XXX_yyyyMMddhhmm.log ;XXX depend on Data foramt field; ex:GPX
#define CSID_C_GNSS_DATA_SPLIT_ENABLE    _CFG_GNSS(0x0104) //T=u8, Enable to split data
#define CSID_C_GNSS_DATA_SPLIT_SIZE      _CFG_GNSS(0x0105) //T=u32, Split size, Default is 200KB
#define CSID_C_GNSS_DATA_SPLIT_UNIT      _CFG_GNSS(0x0106) //T=u8, Split size unit, 0:KB 1:MB

/* RITI Protocol */
#define CSID_C_GNSS_RITI_ENABLE          _CFG_GNSS(0x0110) //T=u8, Enable RITI Protocol to send data to their server
#define CSID_C_GNSS_RITI_SERVER_IP       _CFG_GNSS(0x0111) //T=ipv4, IP of RITI Server IP
#define CSID_C_GNSS_RITI_SERVER_PORT     _CFG_GNSS(0x0112) //T=u32, IP of RITI Server port
#define CSID_C_GNSS_RITI_CID             _CFG_GNSS(0x0113) //T=str, Call Identification, Range:0000~FFFF

/* Remote Host*/
#define CSID_C_GNSS_RM_RULEENABLE    _CFG_GNSS(0x0010) //T=u8, 0:Disable 1:Enable
#define CSID_C_GNSS_RM_HOSTNAME      _CFG_GNSS(0x0020) //T=str, Remote Host Name
#define CSID_C_GNSS_RM_HOSTIP        _CFG_GNSS(0x0030) //T=ipv4, Remote Host IP
#define CSID_C_GNSS_RM_PROTOCOL      _CFG_GNSS(0x0040) //T=u8, Transmission protocol, 0:TCP 1:UDP
#define CSID_C_GNSS_RM_PORTNUM       _CFG_GNSS(0x0050) //T=u32, Transmission port number, 1~65535
#define CSID_C_GNSS_RM_INTERVAL      _CFG_GNSS(0x0060) //T=u32, Transmission interval, min is 1 second.
#define CSID_C_GNSS_RM_PREFIX_MSG    _CFG_GNSS(0x0070) //T=str, Prefix Message of the packet
#define CSID_C_GNSS_RM_SUFFIX_MSG    _CFG_GNSS(0x0080) //T=str, Suffix Message of the packet


/* Status Items */
#define CSID_S_GNSS_ALTERED                  _STA_GNSS(0x0001) //T=u8, ui altered
#define CSID_S_GNSS_DATA_FORMAT_ALTERED      _STA_GNSS(0x0002) //T=u8, Based on data to storage function, 0:RAW 1:GPX
#define CSID_S_GNSS_DATA_DOWNLOAD_FILE       _STA_GNSS(0x0003) //T=str, Download file name
#define CSID_S_GNSS_DATA_DOWNLOAD_FILE_LK    _STA_GNSS(0x0004) //T=str, Download file name(record link file)
#define CSID_S_GNSS_DATA_DEVICE_ALTERED      _STA_GNSS(0x0005) //T=u8, Select which device to download log. 0:Internal 1:External

/* GNSS Status */
#define CSID_S_GNSS_STA_UI_ALTERED           _STA_GNSS(0x0006) //T=u8, ui altered for reflash status page
#define CSID_S_GNSS_STA_GPS_FIX              _STA_GNSS(0x0007) //T=u8, 0:No Fix 1:Fixed(Lat, Long is ready)
#define CSID_S_GNSS_STA_AGPS_FIX             _STA_GNSS(0x0008) //T=u8, 0:No Alpdata 1:Alpdata is ready
#define CSID_S_GNSS_STA_UTC                  _STA_GNSS(0x0009) //T=str, UTC time from RMC(hhmmss)
#define CSID_S_GNSS_STA_POSITION             _STA_GNSS(0x000a) //T=str, Lat&Long from RMC
#define CSID_S_GNSS_STA_ALTITUDE             _STA_GNSS(0x000b) //T=str, Altitude from GGA(meters)
#define CSID_S_GNSS_STA_TC                   _STA_GNSS(0x000c) //T=str, True Course from RMC(degrees)
#define CSID_S_GNSS_STA_SPEED                _STA_GNSS(0x000d) //T=str, Ground Speed from RMC(km/h)
#define CSID_S_GNSS_STA_SATELLITES_NUM       _STA_GNSS(0x000e) //T=u8, Satellites number from GSV
#define CSID_S_GNSS_STA_SATELLITES_ID        _STA_GNSS(0x0010) //T=str, ID of each Satellites from GSV
#define CSID_S_GNSS_STA_SATELLITES_SIGNAL    _STA_GNSS(0x0020) //T=str, Signal strength of each Satellites from GSV

#endif //ifndef _CSID_GNSS_H
