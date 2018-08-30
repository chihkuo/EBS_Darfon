#ifndef __COMMANDER__
#define __COMMANDER__

#include <stdlib.h> 		// for system function
#include <stdio.h> 			// for acess file and string function
#include <signal.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h> 	// for inet_ntoa function
#include <arpa/inet.h> 		// for inet_ntoa function
#include <sys/socket.h> 	// for inet_ntoa and sucket function
#include <netdb.h> 			// for gethostbyname function
#include <net/if.h> 		// for struct ifreq
#include <linux/sockios.h> 	// for SIOCGMIIPHY and SIOCGMIIREG
//#include <errno.h> 		// for errno number
#include <time.h> 			// for ctime and asctime or other time function
#include <sys/timeb.h> 		// for ftime function
#include <sys/time.h> 		// for gettimeofday and settimeofday
#include <unistd.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <dirent.h>

#define RALINK_GPIO_E

#ifndef TC3262
#include "ralink_gpio.h"
#endif

#define GPIO_DEV "/dev/gpio"

#ifdef TC3262
#define Reset_BUTTON	            "/proc/tc3162/reset_button"
#define Wifi_BUTTON		            "/proc/tc3162/wlan_button"
#define WPS_BUTTON		            "/proc/tc3162/wps_button_stats"
#define LED_INET		            "/proc/tc3162/led_internet_stat"
#define LED_3G			            "/proc/tc3162/led_3g_stat"
#define LED_DEF			            "/proc/tc3162/led_def"

// led_no in led.conf	
#define LED_INTERNET_STATUS			33
#define LED_INTERNET_ACT_STATUS		34
#define LED_FXS_STATUS				49
#define LED_PWR_FLASH				59    
#endif

#include "csman.h"
#include "csid/csid_gid.h"
#include "csid/csid_local.h"
#include "csid/csid_http.h"
#include "csid/csid_dhcp.h"
#include "csid/csid_fix.h"
#include "csid/csid_pppoe.h"
#include "csid/csid_pptp.h"
#include "csid/csid_l2tp.h"
#include "csid/csid_ddns.h"
#include "csid/csid_dnsproxy.h"
#include "csid/csid_ntp.h"
#include "csid/csid_schedule.h"
#include "csid/csid_sendmail.h"
#include "csid/csid_routing.h"
#include "csid/csid_misc.h"
#include "csid/csid_spap.h"
#include "csid/csid_snmp.h"
#include "csid/csid_pkfilter.h"
#include "csid/csid_qos.h"
#include "csid/csid_urlblock.h"
#include "csid/csid_dmfilter.h"
#include "csid/csid_vcomputer.h"
#include "csid/csid_macctl.h"
#include "csid/csid_portfw.h"
#include "csid/csid_mrouting.h"
#include "csid/csid_sys.h"
#include "csid/csid_wlanap.h"
#include "csid/csid_wlanap2.h"
#include "csid/csid_wlanapcli.h"
#include "csid/csid_wlanapcli2.h"
#include "csid/csid_sdmz.h"
#include "csid/csid_3g.h"
#include "csid/csid_wlancli.h"
#include "csid/csid_local.h"
#include "csid/csid_green.h"
#include "csid/csid_xdsl.h"
#include "csid/csid_pptpserv.h"
#include "csid/csid_l2tp.h"
#include "csid/csid_l2tpserv.h"
#include "csid/csid_iburst.h"
#include "csid/csid_tr069.h"
#include "csid/csid_gre.h"
#include "csid/csid_wibro.h"
#include "csid/csid_wisp.h"
#include "csid/csid_vpn.h"
#include "csid/csid_flashofdm.h"
#include "csid/csid_storage.h"
#include "csid/csid_vlan.h"
#include "csid/csid_dect.h"
#include "csid/csid_ipv6.h"
#include "csid/csid_failover.h"
#include "csid/csid_sipserv.h"
#include "csid/csid_voip.h"
#include "csid/csid_voipvoice.h"
#include "csid/csid_vrrp.h"
#include "csid/csid_rs232.h"
#include "csid/csid_usb.h"
#include "csid/csid_x509.h"
#include "csid/csid_capwap.h"
#include "csid/csid_sslvpn.h"
#include "csid/csid_captive_portal.h"
#include "csid/csid_iomgt.h"
#include "csid/csid_external_server.h"
#include "csid/csid_eoip.h"
#include "csid/csid_ovpn.h"
#include "csid/csid_radius_user.h"
#include "csid/csid_radius_server.h"
#include "csid/csid_radius_group.h"
#include "csid/csid_ssl_vpn_user.h"
#include "csid/csid_grouping.h"
#include "csid/csid_webui.h"
#include "csid/csid_wanbonding.h"
#include "csid/csid_telephony.h"
#include "csid/csid_packet_analyser.h"

#include "board_config.h"
#include "unilog.h"

#include "md5_sha1/key.h"

/*MultiWAN define*/
#define SUCCESS                         1
#define ERROR                           0

#define ENABLE                          1
#define DISABLE                         0

#define DHCP_NUM 3

#define BASIC_WAN_NUM 					1
#define MULTI_WAN_NUM 					16
#define TOTAL_WAN_NUM 					17 //BASIC_WAN_NUM+MULTI_WAN_NUM

#define START_INDEX_ETHER 				1
#define MAX_ETHER_WAN_NUM 				5
#define START_INDEX_ADSL 				6
#define MAX_ADSL_WAN_NUM 				8
#define START_INDEX_3G 					14
#define MAX_3G_WAN_NUM 					3

#define BASIC_WAN_INDEX 				99
#define DEFAULT_POLICY 					100

// Keep those index the same with glue/etc.rt_tables.
#define IP_RULE_PRIO_POLICY_WAN 		29
#define IP_RULE_PRIO_POLICY_MULTIWAN 	30
#define IP_RULE_PRIO_LAN 				20
#define IP_RULE_PRIO_WAN 				199
#define IP_RULE_PRIO_MULTIWAN 			200
#define IP_RULE_PRIO_WANDEF 			222
/*End of MultiWAN define*/

#define STRBUF 					        256
//Joe expand it to 16 for VLAN have maximum size of 4094
#define IF_NAME_SIZE 			        16
#define IP_ADDR_SIZE                    16
#define MAC_ADDR_LEN                    6

#define JOB_STATE_NORMAL 		        0
#define JOB_STATE_FORK 			        1

#define NOTIFIER_LIST_NUM 				47
#define LAN_ALTERED_LIST 				0
#define WAN_TYPE_SET_ALTERED_LIST 		1
#define ROUTING_ALTERED_LIST 			2
#define WAN_CONNECTED_LIST 				3
#define WAN_DISCONNECTED_LIST 			4
#define JOB_REGULAR_ROUTINE				5
#define DHCPSRV_ALTERED_LIST 			6
#define WLANAP_ALTERED_LIST				7
#define MACCTL_ALTERED_LIST				8
#define WLANAP_ATE_ALTERED_LIST			9
#define WAN_WAIT_TRAFFIC_LIST 			10
#define WLANCLI_ALTERED_LIST 			11
#define ALIAS_ALTERED_LIST 				12
#define PPTPSRV_ALTERED_LIST 			13
#define L2TPSRV_ALTERED_LIST 			14
#define NAT_ALTERED_LIST 				15
#define REBOOT_ALTERED_LIST				16
#define STORAGE_USER_ALTERED_LIST 		17
#define FAILOVER_SET_ALTERED_LIST 		18
#define WIBRO_ALTERED_LIST 				19
#define XDSL_ALTERED_LIST 				20
#define IPSEC_ALTERED_LIST 				21
#define IPV6_WAN_TYPE_SET_ALTERED_LIST 	22
#define IPV6_WAN_CONNECTED_LIST 		23
#define IPV6_WAN_DISCONNECTED_LIST 		24
//#define MULTIWAN_CONNECTED_LIST		    22
//#define MULTIWAN_TYPE_SET_ALTERED_LIST    23
//#define MULTIWAN_DISCONNECTED_LIST        24
//#define FAILOVER_ALTERED_LIST 			25
#define IOMGT_EVENT_DETECTED_LIST 	25
#define VLAN_ALTERED_LIST 				26  // multi lan
#define VOIP_ALTERED_LIST 				27
#define IOMGT_RULE_ALTERED_LIST 	28
//#define VRRP_ALTERED_LIST 				28
#define PORT_ALTERED_LIST 				29
#define RS232_ALTERED_LIST          	30

#define WIFI_VLAN_ID_ALTERED_LIST    	31
#define AP_LAN_ALTERED_LIST          	32
#define RADIUS_USER_ALTERED_LIST        33
#define RADIUS_SERVER_ALTERED_LIST      34
#define X509_ALTERED_LIST               35
#define SSLVPN_ALTERED_LIST             36
#define SSL_VPN_USER_ALTERED_LIST       37

#define GROUPING_ALTERED_LIST           39

#define HANDLE_ROUTING_ALTERED_LIST     40
#define CAPTIVE_PORTAL_ALTERED_LIST     41
#define RADIUS_GROUP_ALTERED_LIST       42
#define SSLVPN_CLIENT_ALTERED_LIST      43
#define SSLVPN_CISCO_ALTERED_LIST       44
#define SCEP_ALTERED_LIST       		45
#define OPENVPN_SERVER_ALTERED_LIST		46

#define FAKE_IP                         "10.64.64.64"
#define FAKE_GW                         "10.112.112.112"

#define MAX_LAN_SVR                     12 // define how many DHCP server can be start
#define MAC_ADDR_LEN	                6

#if	defined(CID688NC)
#define VOIPWANNUM 3
#elif defined(CIW562AM)
#define VOIPWANNUM 2
#else
#define VOIPWANNUM 1
#endif

#if	defined(BIG_BTG_MIX)
#define VoIP_Vo3G_SWITCH
#endif

typedef struct CMD_JOBS CMD_JOBS_T;

struct CMD_JOBS
{
	char    *name;
	int     (*init)(int fd, CMD_JOBS_T *job);
	int     (*phase_two)(int fd, CMD_JOBS_T *job);
	int     pid;
	char    state;	
};

typedef int (*notify_func) __P((void *, int, void *));

struct notifier {
    struct notifier *next;
    notify_func	    func;
    void	        *arg;
    CMD_JOBS_T	    *job;
};

struct reg_param {
    int             csm_fd;
    void            *arg;
    notify_func     regular_func;
    CMD_JOBS_T      *job;
};

int _3g_sch_flag; // flag write by handle_schchk.c

int iomgt_call_handler(int fd, int num);
int notify(int notifierList, int val, void *arg);
void remove_notifier(int notifierList , notify_func func, void *arg);
int add_notifier(int notifierList , notify_func func, void *arg, CMD_JOBS_T *job);
void gpio_set_led(unsigned int gpio, unsigned int on_time, unsigned int off_time,
		                unsigned int blinks, unsigned int rests, unsigned int times);
void exec_cmd(char* fmt, ...);		                
void set_login_prompt(int csman_fd);

/* handle_lan */
int init_lan(int fd, CMD_JOBS_T *job);

/* handle stp */
int init_stp(int fd, CMD_JOBS_T *job);

/* hanlde_wan */
int wan_phase_two(int fd, CMD_JOBS_T *job);
int init_wan(int fd, CMD_JOBS_T *job);

/*handle_ondemand*/
int init_ondemand(int fd, CMD_JOBS_T *job);
int init_multi_ondemand(int fd, CMD_JOBS_T *job);

/*handle_nat*/
int init_nat(int fd, CMD_JOBS_T *job);

/*handle_qos*/
int init_qos(int fd, CMD_JOBS_T *job);

/*handle_upnp*/
int init_upnp(int fd, CMD_JOBS_T *job);

/*handle_mini_upnpd*/
int init_mupnpd(int fd, CMD_JOBS_T *job);

/*handle_routing*/
int init_routing(int fd, CMD_JOBS_T *job);

/*handle_route_protocol*/
int init_route_protocol(int fd, CMD_JOBS_T *job);

/*handle_load_balance*/
int init_load_balance(int fd, CMD_JOBS_T *job);

/* handle_wanbonding */
int init_wanbonding(int fd, CMD_JOBS_T *job);

/*handle_snmp*/
int init_snmp(int fd, CMD_JOBS_T *job);

/*handle_ddns*/
int init_ddns(int fd, CMD_JOBS_T *job);

/*handle_ddnsv6*/
int init_ddnsv6(int fd, CMD_JOBS_T *job);

/*handle_mail*/
int init_mail(int fd, CMD_JOBS_T *job);

/*handle_spap*/
int init_spap(int fd, CMD_JOBS_T *job);

/*handle_time*/
int init_time(int fd, CMD_JOBS_T *job);

/*handle_rbydom*/
int init_rbydom(int fd, CMD_JOBS_T *job);

/*handle_rbyip*/
int init_rbyip(int fd, CMD_JOBS_T *job);

/* hanlde_dhcpsrv*/
int init_dhcpsrv(int fd, CMD_JOBS_T *job);

/* handle_wlanap*/
int init_wlanap(int fd, CMD_JOBS_T *job);
int wlan_phase_two(int fd, CMD_JOBS_T *job);
int init_wpa(int fd, CMD_JOBS_T *job);

/* handle_password*/
int init_password(int fd, CMD_JOBS_T *job);

/* handle_system*/
int init_system(int fd, CMD_JOBS_T *job);

/* handle_wps_status */
int init_wps_status(int fd, CMD_JOBS_T *job);

/* handle_wlancli*/
int init_wlancli(int fd, CMD_JOBS_T *job);

/* handle_wlanap*/
int init_dnsrelay(int fd, CMD_JOBS_T *job);

/* handle_wireless_mode */
int init_wireless_mode(int fd, CMD_JOBS_T *job);

/*handle_sdmz*/
int init_sdmz(int fd, CMD_JOBS_T *job);

/*handle_autobak*/
int init_autobak(int fd, CMD_JOBS_T *job);

/* handle_serial */
int init_serial(int fd, CMD_JOBS_T *job);

/* handle_igmp */
int init_igmp(int fd, CMD_JOBS_T *job);

/* handle_half_bridge */
int init_half_bridge(int fd, CMD_JOBS_T *job);

/* handle_green */
int init_green(int fd, CMD_JOBS_T *job);

/* handle_gpio */
int init_gpio(int fd, CMD_JOBS_T *job);

/* handle_adsl */
int init_adsl(int fd, CMD_JOBS_T *job);

/* handle wlan ap ate */
int init_wlanap_ate(int fd, CMD_JOBS_T * job);

/* handle pptp server */
int init_pptpsrv(int fd, CMD_JOBS_T * job);

/* handle pptp client */
int init_pptp(int fd, CMD_JOBS_T * job);

/* handle l2tp server */
int init_l2tpsrv(int fd, CMD_JOBS_T * job);

/* handle l2tp client */
int init_l2tp(int fd, CMD_JOBS_T * job);

/* handle_ipsec */
int init_ipsec(int fd, CMD_JOBS_T * job);

/* handle_vpn_failover */
int init_vpn_failover(int fd, CMD_JOBS_T * job);

/* handel_eoip */
int init_eoip_tunnel(int fd, CMD_JOBS_T * job);

/* handle_russian_p1wan */
int init_russian_p1wan(int fd, CMD_JOBS_T *job);

/* handle tr069 */
int init_tr069(int fd, CMD_JOBS_T * job);

/* handle_keep_alive */
int init_keep_alive(int fd, CMD_JOBS_T * job);

/* handle wibro */
int init_wibro(int fd, CMD_JOBS_T * job);

/* handle_3g_status */
int init_3g_status(int fd, CMD_JOBS_T * job);

/*handle_3g_setting*/
int init_3g_setting(int fd, CMD_JOBS_T *job); 

/* handle_flashofdm_keep_alive */
int init_flashofdm_keep_alive(int fd, CMD_JOBS_T * job);

/* handle_gre_tunnel */
int init_gre_tunnel(int fd, CMD_JOBS_T * job); 

/* handle_sip_alg */
int init_sip_alg(int fd, CMD_JOBS_T * job); 

/* handle_ftp_alg */
int init_ftp_alg(int fd, CMD_JOBS_T * job);

/* handle_reboot */
int init_reboot(int fd, CMD_JOBS_T * job);

int init_hwnat(int fd, CMD_JOBS_T *job);

/* handle_samba */
int init_samba(int fd, CMD_JOBS_T *job);

/* handle_ftpd */
int init_ftpd(int fd, CMD_JOBS_T *job);

/* handle_dl */
int init_dl(int fd, CMD_JOBS_T *job);

/* handle_disk */
int init_disk(int fd, CMD_JOBS_T *job);

/* handle user */
int init_user(int fd,CMD_JOBS_T *job);

/* handle_usb_device*/
int init_usb_device(int fd, CMD_JOBS_T *job);

/* handle_usb_green */
int init_usb_green(int fd, CMD_JOBS_T *job);

/* handle_daap */
int init_daap(int fd, CMD_JOBS_T *job);

/* handle_hotplug */
int init_hotplug(int fd, CMD_JOBS_T *job);

/* handle_dlna */
#ifdef HAVE_DLNA
int init_dlna(int fd, CMD_JOBS_T *job);
#endif

/* handle_bt */
int init_bt(int fd, CMD_JOBS_T *job);

/* handle_alias */
int init_alias(int fd, CMD_JOBS_T *job);

/* handle_altddns */
int init_altddns(int fd, CMD_JOBS_T *job);

/* handle_webHDact */
int init_webHDact(int fd, CMD_JOBS_T *job);

/* handle_watchdog */
int init_watchdog(int fd, CMD_JOBS_T *job);

/* handle ipv6 */
int init_ipv6(int fd, CMD_JOBS_T *job);

/* handle_schudle_check */
int init_schedule(int fd, CMD_JOBS_T *job);

/* handle v6filter */
int init_v6filter(int fd, CMD_JOBS_T *job);

/* handle vlan */
int init_vlan(int fd, CMD_JOBS_T *job);

/* handle multiwan*/
int multiwan_phase_two(int fd, CMD_JOBS_T *job);
int init_multiwan(int fd, CMD_JOBS_T *job);

/* handle wan_alias */
int init_wan_alias(int fd, CMD_JOBS_T *job);

/* hanlde_failover */
int init_failover(int fd, CMD_JOBS_T *job);

#ifdef _AMIT_ENABLE_VOIP
int init_voip(int fd, CMD_JOBS_T *job);
#endif

#ifdef _AMIT_ENABLE_WEB_REBINDING
/*handle_webrb*/
int init_webrb(int fd, CMD_JOBS_T *job);
#endif

#ifdef _AMIT_ENABLE_SSHD
int init_sshd(int fd, CMD_JOBS_T *job);
#endif

#ifdef _AMIT_ENABLE_VRRP
/* handle_vrrp  */
int init_vrrp(int fd, CMD_JOBS_T *job);
#endif

#ifdef _AMIT_ENABLE_VDSL
/* handle vdsl */
int init_vdsl(int fd, CMD_JOBS_T *job);
#endif

int init_rtsp_alg(int fd, CMD_JOBS_T *job);
#endif

int init_shareport(int fd, CMD_JOBS_T * job);

int init_bonjour(int fd, CMD_JOBS_T * job);

int init_host_resolution(int fd, CMD_JOBS_T *job);

/* handle hnap */
int init_hnap(int fd, CMD_JOBS_T * job);

/* handle mydlink */
int init_mydlink(int fd, CMD_JOBS_T * job);

/* handle quota */
int init_quota(int fd, CMD_JOBS_T * job);

int init_port(int fd, CMD_JOBS_T *job);

int init_bty_led(int fd, CMD_JOBS_T *job);

/* handle_wps_btn */
int init_wps_btn(int fd, CMD_JOBS_T * job);

/* handle_reset_btn */
int init_reset_btn(int fd, CMD_JOBS_T * job);

int init_capwap(int fd, CMD_JOBS_T * job);

/* handle_ate */
int init_ate(int fd, CMD_JOBS_T *job);

/* handle_mbss */
int init_mbss(int fd, CMD_JOBS_T *job);

#if defined(ENABLE_APONLY)
int init_ap_lan(int fd, CMD_JOBS_T *job);
#endif

#if defined(ENABLE_REPEATER_WPS) 
/* handle_repeater_wps */
int init_repeater_wps(int fd, CMD_JOBS_T *job);
#endif

#if defined(ENABLE_X509) 
int init_x509(int fd, CMD_JOBS_T *job);
int init_scep(int fd, CMD_JOBS_T *job);
#endif

#if defined(ENABLE_SSLVPN) 
int init_sslvpn(int fd, CMD_JOBS_T *job);
#endif

#if defined(ENABLE_SSLVPN_CISCO) 
int init_sslvpn_cisco(int fd, CMD_JOBS_T *job);
#endif

#if defined(ENABLE_SSLVPN_CLIENT) 
int init_sslvpn_client(int fd, CMD_JOBS_T *job);
#endif

#if defined(ENABLE_OPENVPN_SERVER) 
int init_openvpn_server(int fd, CMD_JOBS_T *job);
#endif

#if defined(ENABLE_DATA_USAGE) 
int init_data_usage(int fd, CMD_JOBS_T *job);
#endif

/*handle_ddp*/
int init_ddp(int fd, CMD_JOBS_T *job);

/*handle_dip_switch*/
int init_dip_switch(int fd, CMD_JOBS_T *job);

/*handle_wan_led*/
int init_wan_led(int fd, CMD_JOBS_T *job);

/*handle_recent*/
int init_recent(int fd, CMD_JOBS_T *job);

#if defined(ENABLE_EXTERNAL_SERVER)
/*handle_external_server*/
int init_external_server(int fd, CMD_JOBS_T *job);
#endif

int init_radius_server(int fd, CMD_JOBS_T *job);
int init_radius_user(int fd, CMD_JOBS_T *job);
int init_ssl_vpn_user(int fd, CMD_JOBS_T *job);
int init_radius_group(int fd, CMD_JOBS_T *job);
int init_grouping(int fd, CMD_JOBS_T *job);
int init_captive_portal(int fd, CMD_JOBS_T *job);
int init2_radius_group(int fd, CMD_JOBS_T *job);
int init3_radius_group(int fd, CMD_JOBS_T *job);

int captive_portal_action(int fd, CMD_JOBS_T *job);
	
int WAN_ready_captive_portal(void *arg, int fd, void *arg1);

int SEC_handle_radius_user(int fd, CMD_JOBS_T *job);
int SEC_handle_ssl_vpn_user(int fd, CMD_JOBS_T *job);
int SEC_radius_group(int fd, CMD_JOBS_T *job);
int SEC_handle_grouping(int fd, CMD_JOBS_T *job);


#if defined(BDP77AAM_001) || defined(APC761AM_P03)|| defined(APC772AM_P01) || defined(APW771AM_P01)||defined(APC77BAM_P01) || defined(APW77BAM_P01) || defined(APP761AM_P03) || defined(APO771AM_P01)  || defined(OAP77AAM_P01)
int init_AP_mode_change(int fd, CMD_JOBS_T *job);
#endif

#if defined(APC761AM_P03) || defined(APP761AM_P03) || defined(APO771AM_P01)  || defined(OAP77AAM_P01)
int apControl(int fd, CMD_JOBS_T *job);
#endif


#if defined(ENABLE_JFFS2) 
/* handle_jffs2 */
int init_storage(int fd, CMD_JOBS_T *job);
#endif

#if defined(ENABLE_DYNAMIC_LANIP)
int init_dynamic_lanip(int fd, CMD_JOBS_T *job);
#endif 

//This section is for RELEASE IN SDK COPY
#ifdef SDK_RELEASE
extern CMD_JOBS_T cmd_jobs_sdk[];
//int init_helloworld(int fd, CMD_JOBS_T *job);
/* darfonlogger */
int init_darfonlogger(int fd, CMD_JOBS_T *job);//miketest

#endif

 /* handle_wlan_btn */
int init_wlan_btn(int fd, CMD_JOBS_T * job);

int init_iom(int fd, CMD_JOBS_T *job);

#if defined(ENABLE_STATIC_ARP_MAPPING)
int init_static_arp(int fd, CMD_JOBS_T *job);
#endif

/* tools/getlink.c */
int switch_init();
int reg_read(int esw_fd,int offset, int *value);
void switch_fini(int fd);
