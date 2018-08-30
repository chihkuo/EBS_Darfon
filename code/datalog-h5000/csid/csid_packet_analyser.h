/*  le Member ID Definition of Group External Server.
 *
 * Moderator:
 * Group ID: 0x00B40000/0x80B40000
 */

#ifndef _CSID_PACKET_ANALYSER_H_
#define _CSID_PACKET_ANALYSER_H_

#include "csid_gid.h"

#define _CFG_PACKET_ANALYSER(x) (CSID_GIDC_PACKET_ANALYSER|(x))
#define _STA_PACKET_ANALYSER(x) (CSID_GIDS_PACKET_ANALYSER|(x))

/* Configuration Items */
#define CSID_C_PACKET_ANALYSER_ENABLE               _CFG_PACKET_ANALYSER(0x0000)   //T=u8, 0:disable 1:enable
#define CSID_C_PACKET_ANALYSER_FILENAME             _CFG_PACKET_ANALYSER(0x0001)   //T=str, filename of saving pcap file
#define CSID_C_PACKET_ANALYSER_SPLIT_ENABLE         _CFG_PACKET_ANALYSER(0x0002)   //T=u8, 0:disable 1:enable
#define CSID_C_PACKET_ANALYSER_SPLIT_SIZE           _CFG_PACKET_ANALYSER(0x0003)   //T=u32, file size of each split file
#define CSID_C_PACKET_ANALYSER_FILTER_ENABLE        _CFG_PACKET_ANALYSER(0x0004)   //T=u8, 0:disable 1:enable
#define CSID_C_PACKET_ANALYSER_SRC_MAC              _CFG_PACKET_ANALYSER(0x0005)   //T=str, filter option of source mac
#define CSID_C_PACKET_ANALYSER_SRC_IP               _CFG_PACKET_ANALYSER(0x0006)   //T=str, filter option of source ip
#define CSID_C_PACKET_ANALYSER_SRC_PORT             _CFG_PACKET_ANALYSER(0x0007)   //T=str, filter option of source ports
#define CSID_C_PACKET_ANALYSER_DST_MAC              _CFG_PACKET_ANALYSER(0x0008)   //T=str, filter option of destination mac
#define CSID_C_PACKET_ANALYSER_DST_IP               _CFG_PACKET_ANALYSER(0x0009)   //T=str, filter option of destination ip
#define CSID_C_PACKET_ANALYSER_DST_PORT             _CFG_PACKET_ANALYSER(0x0010)   //T=str, filter option of destination ports
#define CSID_C_PACKET_ANALYSER_INTERFACE_ALL        _CFG_PACKET_ANALYSER(0x0011)   //T=u8, 0:disable 1:enablei
#define CSID_C_PACKET_ANALYSER_TCPDUMP_NUM          _CFG_PACKET_ANALYSER(0x0012)   //T=u8, num of TCPDump daemon should be run
#define CSID_C_PACKET_ANALYSER_SIZE_TYPE            _CFG_PACKET_ANALYSER(0x0013)   //T=u8, 0:KB, 1:MB
#define CSID_C_PACKET_ANALYSER_MULTI_INTERFACE_WAN  _CFG_PACKET_ANALYSER(0x0020)   //T=u8, 0:disable 1:enable
#define CSID_C_PACKET_ANALYSER_MULTI_INTERFACE_VAP  _CFG_PACKET_ANALYSER(0x0030)   //T=u8, 0:disable 1:enable
#define CSID_C_PACKET_ANALYSER_MULTI_INTERFACE_ASY  _CFG_PACKET_ANALYSER(0x0040)   //T=u8, 0:disable 1:enable
#define CSID_C_PACKET_ANALYSER_MULTI_INTERFACE_VAP_5G  _CFG_PACKET_ANALYSER(0x0050)   //T=u8, 0:disable 1:enable
#define CSID_C_PACKET_ANALYSER_MULTI_ASY_PORT       _CFG_PACKET_ANALYSER(0x0060)   //T=u32, D=6666, port number for ASY packet capture

/* Status Items*/
#define CSID_S_PACKET_ANALYSER_ALTER                _STA_PACKET_ANALYSER(0x0000)   //T=u8, 0:disable 1:enable
#define CSID_S_PACKET_ANALYSER_LOCAL_STATUS         _STA_PACKET_ANALYSER(0x0001)   //T=u8, 0:not running 1:running
#define CSID_S_PACKET_ANALYSER_MULTI_STATUS         _STA_PACKET_ANALYSER(0x0010)   //T=u8, 0:not running 1:running
#define CSID_S_PACKET_ANALYSER_VAP_STATUS           _STA_PACKET_ANALYSER(0x0020)   //T=u8, 0:not running 1:running
#define CSID_S_PACKET_ANALYSER_ASY_STATUS           _STA_PACKET_ANALYSER(0x0030)   //T=u8, 0:not running 1:running

#endif
