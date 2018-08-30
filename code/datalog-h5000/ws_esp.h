#ifndef _WS_ESP_H_
#define _WS_ESP_H_

#include "unilog.h"
#include "wsif.h"
#include "asif.h"
#include "csman.h"
#include "csid/csid_sys.h"
#include "csid/csid_local.h"
#include "csid/csid_dhcp.h"
#include "csid/csid_fix.h"
#include "csid/csid_l2tp.h"
#include "csid/csid_pptp.h"
#include "csid/csid_pppoe.h"
#include "csid/csid_3g.h"
#include "csid/csid_iburst.h"
#include "csid/csid_wlanap.h"
#include "csid/csid_webui.h"
#include "csid/csid_xdsl.h"
#include "csid/csid_failover.h"

#ifdef _MULTI_LAN
#include "csid/csid_wlanap2.h"
#endif

#ifdef _AMIT_WISP
#include "csid/csid_wlanapcli.h"
#endif
#ifdef _AMIT_DDNS
#include "csid/csid_ddns.h"
#endif

//** Define your weblet name and attribute here!
#define EZ_DBG	1
#define WEBLET_NAME			ws_esp
#define MY_WS_ATTRIBUTE		(WSF_CACHE_PAGE)
#define EZ_CONFIG_SIZE  4096

// for forward reference
extern weblet_t	WEBLET_NAME;

/* Place functions to help this web service here */

//static unsigned char *get_buf;
//static unsigned char *put_buf;
typedef struct {
    unsigned char   *data;
    int             size;
    int             offset;
}ezConfig;

typedef struct {
    unsigned long   id;
    unsigned short  size;
    unsigned char   *value;
}ezItem;

typedef struct {
	ezConfig ezCfg;
	int	chip_size;
	int 	chip_offset;
}esp_state_t;

#define EZ_STATE(x)  ((esp_state_t *)((x)->state.scratch_pad)) //用來保存靜態變數,因為weblet是用fork的方式

#define CLOSE_CSID  0x00000000

#define LOCAL_WAN_SERVICE_DHCP      0x01
#define LOCAL_WAN_SERVICE_PPPOE     0x02
#define LOCAL_WAN_SERVICE_3G        0x04
#define START_INDEX_3G              0x0d

int check_WAN_link_status(page_blk_t *page);
int add_item(ezConfig *config,unsigned long id,unsigned short size, unsigned char *data);
int add_close_item(ezConfig *config);
int discover_dhcpserver(unsigned char *mac);
int discover_pppoeserver(char *wan_if);
int check_WAN_service(page_blk_t *page);
int WLANConfig_to_ezConfig(page_blk_t *page);
int Apply_ezConfig(page_blk_t *page);
ezItem *get_item(ezConfig *config);

#ifdef _AMIT_GREEN
int GreenConfig_to_ezConfig(page_blk_t *page);
int GreenService_to_ezConfig(page_blk_t *page);
int SaveGreenConfig_to_CSID( int fd, unsigned int csid, ezItem* item);
#endif
#ifdef _AMIT_TIMEZONE
int TimeZone_to_ezConfig(page_blk_t *page);
int SaveTZConfig_to_CSID( int fd, unsigned int csid, ezItem* item);
#endif
#ifdef _AMIT_DDNS
int DDNS_to_ezConfig(page_blk_t *page);
#endif
#ifdef _AMIT_WISP
int WISPConfig_to_ezConfig(page_blk_t *page);
int SaveWISPConfig_to_CSID( int fd, unsigned int csid, ezItem* item);
#endif
#ifdef _AMIT_MULTI_WAN
int SaveMultiWANConfig_to_CSID( int fd, unsigned int csid, ezItem* item);
#endif

#endif
