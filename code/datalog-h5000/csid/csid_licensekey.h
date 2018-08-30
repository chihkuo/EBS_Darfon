/*
 * @file Member ID Definition of Group LicenseKey.
 *
 * Moderator: JamieKuo
 * Group ID: 0x00B90000/0x80B90000
 */

#ifndef _CSID_LICENSEKEY_H
#define _CSID_LICENSEKEY_H

#include "csid_gid.h"

#define _CFG_LICENSEKEY(x)         (CSID_GIDC_LICENSEKEY|(x))
#define _STA_LICENSEKEY(x)         (CSID_GIDS_LICENSEKEY|(x))

/* Configuration Items */
#define CSID_C_LK_DK		_CFG_LICENSEKEY(0x0001) //T=str, Device Key
#define CSID_C_LK_LKN		_CFG_LICENSEKEY(0x0002) //T=str, License Key Number

/* Status Items */
#define CSID_S_LK_ALTERED		_STA_LICENSEKEY(0x0001) //T=u8, ui altered

#define CSID_S_LK_FEATURE		_STA_LICENSEKEY(0x0010) //T=str, Feature
#define CSID_S_LK_VERSION		_STA_LICENSEKEY(0x0020)	//T=str, Version
#define CSID_S_LK_REGISTIME		_STA_LICENSEKEY(0x0030)	//T=str, Registration Time
#define CSID_S_LK_STATUS_INFO	_STA_LICENSEKEY(0x0040)	//T=str, Status and Information

#define CSID_S_LK_IEC_RES		_STA_LICENSEKEY(0x0100) //T=u8, IEC 60870-5 Result 0:FALSE 1:TRUE
#define CSID_S_LK_MB_RES		_STA_LICENSEKEY(0x0101) //T=u8, Modbus Result 0:FALSE 1:TRUE

#define CSID_S_LK_ENABLE		_STA_LICENSEKEY(0x0200)	//T=u8,D=0, License Key Enable 0:FALSE 1:TRUE

#endif //ifndef _CSID_LICENSEKEY_H
