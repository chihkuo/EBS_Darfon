#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>

#include <ifaddrs.h>
#include <netpacket/packet.h>


#include "mainproc.h"
#include "modbusdrv.h"

//Darfonlogger+
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include "iniparser-2.17/src/iniparser.h"

extern "C" {
   extern  void Start();
   extern  void MyStart();
}


typedef struct stMI_DATA {
    float  temperature;
    int    year, month, day;
    float  Eac1,Eac2,TEac;
    float  Pac1,Pac2;
    float  Vpv1,Ipv1,Ppv1, Vpv2,Ipv2,Ppv2;
    float  Vac, TIac, TPac,Fac;
    float  Iac1,Iac2;
    int    ErrorCode1, ErrorCode2;
    int    Pre1Code1, Pre1Code2,Pre2Code1, Pre2Code2;
} MI_DATA;

typedef struct stMI_INFO {
    char szSn[17];
    int  year,month,day;
} MI_INFO;

MI_INFO  g_miInfo;
MI_DATA  g_miData;

int parse_ini_file(char * ini_name);

void MyGetMIInfo();
void DumpMiInfo();
void DebugPrint(unsigned char *lpData,int size, char *lpHeader);
void getenvdata();

//global option
int   g_port=0;
char  g_comopt[32];
char  g_misn[16];
char  g_macaddr[18];
int   g_fetchtime=60;
char  g_internalIp[16];
char  g_externalIp[16];

//derfon
int   g_delay1; // unit; 1/1000 sec. for receive data delay
int   g_delay2; //  wait for write data
int   g_delay3;  // noise ??

unsigned char  g_plcid[32];


#include <setjmp.h>


#define LOGFILE	"./log/debug.log"     // all Log(); messages will be appended to this file

bool LogCreated;

extern int MyModbusDrvInit(char *port);
void writeLog (char *message);


extern void Init_mem_cMemRow(int MemRowindex, eType Type, unsigned int StartAddr, int Row, eFamily Family);
int WriteCommBlockModBus( unsigned int pStartAddress,int functioncode, char *pBuffer , int BytesToWrite, int SlaveID);



//#define UseRS232


//#define Demo
//#define disable_keepalivehttpd

#define wait_system_imit
//#define disable_write_csman
//#define write_all_data_to_the_same_csman
//#define enable_ping_test
//#define SaveHistoryData
//#define SUPPORT_ZIGBEE
//#define ONLYMONITOR
//#define disable_Darfonlogger
//#define disable_auto_reboot
#define enable_ping_test













///////
int newSize = 0;
int delSize = 0;
#define new(TYPE, n) malloc(n*sizeof(TYPE)); newSize+=n*sizeof(TYPE)
#define del(ptr, TYPE, n) free(ptr); delSize+=n*sizeof(TYPE)
#define mcheck() DEBUG1(printf("Memory:newSize=%d delSize=%d leakSize=%d\n", \
                        newSize, delSize, newSize-delSize));



#define MODBUS_DEBUG				2
#if(MODBUS_DEBUG>0)
#define DEBUG1(x)	x
#else
#define DEBUG1(x)
#endif
#if(MODBUS_DEBUG>1)
#define DEBUG2(x)	x
#else
#define DEBUG2(x)
#endif

//unsigned int writevaule[31];
unsigned int parameter[31];
unsigned int readparameter[31];
unsigned int DefaultSlaveID=0xf0;
//#define CONF_FILE         "/Temp/US.conf"

char* m_pModbusBuffer;
int oncenum;
int UserCodeSlaveID_For_Bootloader=0xFF;
int inBootloaderMode=0;
int stopAll=0;
int SlaveIDoffset=4;
int checkCommunication=1;
unsigned int UpgradeVersion=0;
unsigned int InverterFWVersion=0;
int VpvLimit=260;//30v
int PacLimit=600;//60w
unsigned int BootloaderVersion=0;
const int PageSize=48;
double UploadCloudDiffMax=600;//s 60s*10min=600
double WriteDataToCSMANROMDiffMax=120;//s 60s*2min=120
double WriteDataToCSMANRAMDiff=60;//s 20S

double UploadCloudDiffMin=120;//s 60s*2min=120
double InverterTimeoutDiff=300;//s 60s*5min=300
double InverterCheckPacTimeoutDiff=300;//s 60s*5min=300

double InverterUploadTimeoutDiff=86400;//s 60s*60mins*24hours=86400
double PingTestTimeoutDiff=3600;//s 60s*60mins=600
double AutoRebootTimeoutDiff=43200;//s 60s*60mins*12hours=43200

double CheckMemTimeoutDiff=43200;//s 60s*60mins*12hours=43200



double RegisterTimeoutDiff=300;//s 60s*5min=300
double RestartHttpdTimeoutDiff=14400;//s 60s*60mins*4hours=14400
double RestartHttpdTimeoutDiff2=3600;//s 60s*60mins*1hours=3600


uint DebuglogLineCount;
uint CheckAutoRebootCount=0;
uint readwritecsmanfailcount=0;




char strName[]= "Name" ;
char strEtotal[] = "Eac_Total" ;
char strInverterLastGetDatatTime[] = "LastGetDatatTime" ;
char stretotaltodaydate[] = "etotaltodaydate" ;
char stretotaltodaystartvalue[] = "etotaltodaystartvalue" ;









bool forceUploadCloud=false;
bool alreadytriggerReboot=false;
bool IfSaveDataToUsbStorage=false;

#define MI_SAVE_ITEMS_NUM_MAX				InverterEnergylen+3
#define WAITAFTERREQUEST				200000 //usleep 0.2s


bool csmanlock=false;



extern mem_cMemRow pMemory[PM_SIZE+EE_SIZE+CM_SIZE];

sDevice Device[] =
{
    {"dsPIC30F2010",      0x040, 1, dsPIC30F},
    {"dsPIC30F2011",      0x0C0, 1, dsPIC30F},
    {"dsPIC30F2011",      0x240, 1, dsPIC30F},
    {"dsPIC30F2012",      0x0C2, 1, dsPIC30F},
    {"dsPIC30F2012",      0x241, 1, dsPIC30F},
    {"dsPIC30F3010",      0x1C0, 1, dsPIC30F},
    {"dsPIC30F3011",      0x1C1, 1, dsPIC30F},
    {"dsPIC30F3012",      0x0C1, 1, dsPIC30F},
    {"dsPIC30F3013",      0x0C3, 1, dsPIC30F},
    {"dsPIC30F3014",      0x160, 1, dsPIC30F},
    {"dsPIC30F4011",      0x101, 1, dsPIC30F},
    {"dsPIC30F4012",      0x100, 1, dsPIC30F},
    {"dsPIC30F4013",      0x141, 1, dsPIC30F},
    {"dsPIC30F5011",      0x080, 1, dsPIC30F},
    {"dsPIC30F5013",      0x081, 1, dsPIC30F},
    {"dsPIC30F5015",      0x200, 1, dsPIC30F},
    {"dsPIC30F5016",      0x201, 1, dsPIC30F},
    {"dsPIC30F6010",      0x188, 1, dsPIC30F},
    {"dsPIC30F6010A",     0x281, 1, dsPIC30F},
    {"dsPIC30F6011",      0x192, 1, dsPIC30F},
    {"dsPIC30F6011A",     0x2C0, 1, dsPIC30F},
    {"dsPIC30F6012",      0x193, 1, dsPIC30F},
    {"dsPIC30F6012A",     0x2C2, 1, dsPIC30F},
    {"dsPIC30F6013",      0x197, 1, dsPIC30F},
    {"dsPIC30F6013A",     0x2C1, 1, dsPIC30F},
    {"dsPIC30F6014",      0x198, 1, dsPIC30F},
    {"dsPIC30F6014A",     0x2C3, 1, dsPIC30F},
    {"dsPIC30F6015",      0x280, 1, dsPIC30F},

    {"dsPIC33FJ64GP206",  0xC1, 3, dsPIC33F},
    {"dsPIC33FJ64GP306",  0xCD, 3, dsPIC33F},
    {"dsPIC33FJ64GP310",  0xCF, 3, dsPIC33F},
    {"dsPIC33FJ64GP706",  0xD5, 3, dsPIC33F},
    {"dsPIC33FJ64GP708",  0xD6, 3, dsPIC33F},
    {"dsPIC33FJ64GP710",  0xD7, 3, dsPIC33F},
    {"dsPIC33FJ128GP206", 0xD9, 3, dsPIC33F},
    {"dsPIC33FJ128GP306", 0xE5, 3, dsPIC33F},
    {"dsPIC33FJ128GP310", 0xE7, 3, dsPIC33F},
    {"dsPIC33FJ128GP706", 0xED, 3, dsPIC33F},
    {"dsPIC33FJ128GP708", 0xEE, 3, dsPIC33F},
    {"dsPIC33FJ128GP710", 0xEF, 3, dsPIC33F},
    {"dsPIC33FJ256GP506", 0xF5, 3, dsPIC33F},
    {"dsPIC33FJ256GP510", 0xF7, 3, dsPIC33F},
    {"dsPIC33FJ256GP710", 0xFF, 3, dsPIC33F},
    {"dsPIC33FJ64MC506",  0x89, 3, dsPIC33F},
    {"dsPIC33FJ64MC508",  0x8A, 3, dsPIC33F},
    {"dsPIC33FJ64MC510",  0x8B, 3, dsPIC33F},
    {"dsPIC33FJ64MC706",  0x91, 3, dsPIC33F},
    {"dsPIC33FJ64MC710",  0x97, 3, dsPIC33F},
    {"dsPIC33FJ128MC506", 0xA1, 3, dsPIC33F},
    {"dsPIC33FJ128MC510", 0xA3, 3, dsPIC33F},
    {"dsPIC33FJ128MC706", 0xA9, 3, dsPIC33F},
    {"dsPIC33FJ128MC708", 0xAE, 3, dsPIC33F},
    {"dsPIC33FJ128MC710", 0xAF, 3, dsPIC33F},
    {"dsPIC33FJ256MC510", 0xB7, 3, dsPIC33F},
    {"dsPIC33FJ256MC710", 0xBF, 3, dsPIC33F},

    {"dsPIC33FJ12GP201", 0x802, 3, dsPIC33F},
    {"dsPIC33FJ12GP202", 0x803, 3, dsPIC33F},
    {"dsPIC33FJ12MC201", 0x800, 3, dsPIC33F},
    {"dsPIC33FJ12MC202", 0x801, 3, dsPIC33F},

    {"dsPIC33FJ32GP204", 0xF0F, 3, dsPIC33F},
    {"dsPIC33FJ32GP202", 0xF0D, 3, dsPIC33F},
    {"dsPIC33FJ16GP304", 0xF07, 3, dsPIC33F},
    {"dsPIC33FJ32MC204", 0xF0B, 3, dsPIC33F},
    {"dsPIC33FJ32MC202", 0xF09, 3, dsPIC33F},
    {"dsPIC33FJ16MC304", 0xF03, 3, dsPIC33F},

    {"dsPIC33FJ128GP804", 0x62F, 3, dsPIC33F},
    {"dsPIC33FJ128GP802", 0x62D, 3, dsPIC33F},
    {"dsPIC33FJ128GP204", 0x627, 3, dsPIC33F},
    {"dsPIC33FJ128GP202", 0x625, 3, dsPIC33F},
    {"dsPIC33FJ64GP804",  0x61F, 3, dsPIC33F},
    {"dsPIC33FJ64GP802",  0x61D, 3, dsPIC33F},
    {"dsPIC33FJ64GP204",  0x617, 3, dsPIC33F},
    {"dsPIC33FJ64GP202",  0x615, 3, dsPIC33F},

    {"dsPIC33FJ64GS606.",  0x4003, 3, dsPIC33F},//MIKE+

    {"dsPIC33FJ32GP304",  0x607, 3, dsPIC33F},
    {"dsPIC33FJ32GP302",  0x605, 3, dsPIC33F},
    {"dsPIC33FJ128MC804", 0x62B, 3, dsPIC33F},
    {"dsPIC33FJ128MC802", 0x629, 3, dsPIC33F},
    {"dsPIC33FJ128MC204", 0x623, 3, dsPIC33F},
    {"dsPIC33FJ128MC202", 0x621, 3, dsPIC33F},
    {"dsPIC33FJ64MC804",  0x61B, 3, dsPIC33F},
    {"dsPIC33FJ64MC802",  0x619, 3, dsPIC33F},
    {"dsPIC33FJ64MC204",  0x613, 3, dsPIC33F},
    {"dsPIC33FJ64MC202",  0x611, 3, dsPIC33F},
    {"dsPIC33FJ32MC304",  0x603, 3, dsPIC33F},
    {"dsPIC33FJ32MC302",  0x601, 3, dsPIC33F},

    {"dsPIC33FJ06GS101",  0xC00, 3, dsPIC33F},
    {"dsPIC33FJ06GS102",  0xC01, 3, dsPIC33F},
    {"dsPIC33FJ06GS202",  0xC02, 3, dsPIC33F},
    {"dsPIC33FJ16GS402",  0xC04, 3, dsPIC33F},
    {"dsPIC33FJ16GS404",  0xC06, 3, dsPIC33F},
    {"dsPIC33FJ16GS502",  0xC03, 3, dsPIC33F},
    {"dsPIC33FJ16GS504",  0xC05, 3, dsPIC33F},

    {"PIC24HJ64GP206",    0x41, 3, PIC24H},
    {"PIC24HJ64GP210",    0x47, 3, PIC24H},
    {"PIC24HJ64GP506",    0x49, 3, PIC24H},
    {"PIC24HJ64GP510",    0x4B, 3, PIC24H},
    {"PIC24HJ128GP206",   0x5D, 3, PIC24H},
    {"PIC24HJ128GP210",   0x5F, 3, PIC24H},
    {"PIC24HJ128GP306",   0x65, 3, PIC24H},
    {"PIC24HJ128GP310",   0x67, 3, PIC24H},
    {"PIC24HJ128GP506",   0x61, 3, PIC24H},//pv inverter
    {"PIC24HJ128GP510",   0x63, 3, PIC24H},
    {"PIC24HJ256GP206",   0x71, 3, PIC24H},
    {"PIC24HJ256GP210",   0x73, 3, PIC24H},
    {"PIC24HJ256GP610",   0x7B, 3, PIC24H},

    {"PIC24HJ12GP201", 0x80A, 3, PIC24H},
    {"PIC24HJ12GP202", 0x80B, 3, PIC24H},

    {"PIC24HJ32GP204", 0xF1F, 3, PIC24H},
    {"PIC24HJ32GP202", 0xF1D, 3, PIC24H},
    {"PIC24HJ16GP304", 0xF17, 3, PIC24H},

    {"PIC24HJ128GP504", 0x67F, 3, PIC24H},
    {"PIC24HJ128GP502", 0x67D, 3, PIC24H},
    {"PIC24HJ128GP204", 0x667, 3, PIC24H},
    {"PIC24HJ128GP202", 0x665, 3, PIC24H},
    {"PIC24HJ64GP504",  0x677, 3, PIC24H},
    {"PIC24HJ64GP502",  0x675, 3, PIC24H},
    {"PIC24HJ64GP204",  0x657, 3, PIC24H},
    {"PIC24HJ64GP202",  0x655, 3, PIC24H},
    {"PIC24HJ32GP304",  0x647, 3, PIC24H},
    {"PIC24HJ32GP302",  0x645, 3, PIC24H},

    {"PIC24FJ64GA006",    0x405, 3, PIC24F},
    {"PIC24FJ64GA008",    0x408, 3, PIC24F},
    {"PIC24FJ64GA010",    0x40B, 3, PIC24F},
    {"PIC24FJ96GA006",    0x406, 3, PIC24F},
    {"PIC24FJ96GA008",    0x409, 3, PIC24F},
    {"PIC24FJ96GA010",    0x40C, 3, PIC24F},
    {"PIC24FJ128GA006",   0x407, 3, PIC24F},
    {"PIC24FJ128GA008",   0x40A, 3, PIC24F},
    {"PIC24FJ128GA010",   0x40D, 3, PIC24F},
    {NULL, 0, 0}
};

//Darfonlogger+
s_InverterTable  CurrentUsedglobesetting;
float commandtimeinterval=2.5;//s
unsigned int rbuffer[30];
int csmanfd;
char UploadFilePath[40]="/tmp/dfupload.txt";
char version[20]="00000014.DL200";
char WanGW[50]="";;
char UsbStorageFolderPath[40]="/var/usb/C";
char CSVFileMainFolderPath[40]="/var/usb/C/data";
char systemtmpPath[40]="/tmp";
/*
#ifdef MI_ReadDebugData
d
#else

s_EnergyData 	DefaultEnergyData[InverterEnergylen] =
{
    {"Temperature",	0,	"C",		"0"},
	{"Eac_Today",	0,	"KWH",		"0"},
	{"Vpv",			0,	"V",		"0"},
	{"Ipv",			0,	"A",		"0"},
	{"Ppv",			0,	"W",		"0"},
	{"Vac",			0,	"V",		"0"},
	{"Iac",			0,	"A",		"0"},
	{"Pac",			0,	"W",		"0"},
	{"Fac",			0,	"Hz"		"0"},
	{"Eac_Total",	0,	"KWH",		"0"},
	{"Error_COD1",	0,	"",			"0000"},
	{"Error_COD2",	0,	"",			"0000"},
	{"Error_COD3",	0,	"",			"0000"},
	{"Error_COD4",	0,	"",			"0000"},
	{"Error_COD5",	0,	"",			"0000"},
	{"Error_COD6",	0,	"",			"0000"}

};


#define TitleCSVDataNum 11
#define TitleCSVErrorNum 3

char CsvDataTitle[TitleCSVDataNum][20] = {"Time","Temperature","Eac_Today","Vpv","Ipv","Ppv","Vac","Iac","Pac","Fac","Eac_Total"};
char CsvErrorTitle[TitleCSVErrorNum][20] = {"Time","Error_Code","Error_Message"};
#endif
*/
pthread_t tSaveHistoryData;



static sigjmp_buf jmpbuf;

static void alarm_func()
{
	 siglongjmp(jmpbuf, 1);
}


/*

//FLAG
#define CSID_C_DARFONLOGGER_ROM_RULE_MODE  	_CFG_DARFONLOGGER(0x0000) //0:MI, 1:HYB
#define CSID_C_DARFONLOGGER_ROM_RULE_CLEAR_DEVICE_TABLE  	_CFG_DARFONLOGGER(0x0001) //0:NO, 1:YES


//
#define CSID_C_DARFONLOGGER_ROM_RULE_Temperature  	_CFG_DARFONLOGGER(0x0100) //Temperature
#define CSID_C_DARFONLOGGER_ROM_RULE_Eac_Today  	_CFG_DARFONLOGGER(0x0200) //Eac_Today
#define CSID_C_DARFONLOGGER_ROM_RULE_Vpv_A  		_CFG_DARFONLOGGER(0x0300) //Vpv_A
#define CSID_C_DARFONLOGGER_ROM_RULE_Ipv_A  		_CFG_DARFONLOGGER(0x0400) //Ipv_A
#define CSID_C_DARFONLOGGER_ROM_RULE_Ppv_A  		_CFG_DARFONLOGGER(0x0500) //Ppv_A
#define CSID_C_DARFONLOGGER_ROM_RULE_Vac_A  		_CFG_DARFONLOGGER(0x0600) //Vac_A
#define CSID_C_DARFONLOGGER_ROM_RULE_Iac_A  		_CFG_DARFONLOGGER(0x0700) //Iac_A
#define CSID_C_DARFONLOGGER_ROM_RULE_Pac_A  		_CFG_DARFONLOGGER(0x0800) //Pac_A
#define CSID_C_DARFONLOGGER_ROM_RULE_Fac_A  		_CFG_DARFONLOGGER(0x0900) //Fac_A
#define CSID_C_DARFONLOGGER_ROM_RULE_Eac_Total  	_CFG_DARFONLOGGER(0x0A00) //Eac_Total
#define CSID_C_DARFONLOGGER_ROM_RULE_Error_COD1  	_CFG_DARFONLOGGER(0x0B00) //Error_COD1
#define CSID_C_DARFONLOGGER_ROM_RULE_Error_COD2  	_CFG_DARFONLOGGER(0x0C00) //Error_COD2
#define CSID_C_DARFONLOGGER_ROM_RULE_Error_COD3  	_CFG_DARFONLOGGER(0x0D00) //Error_COD3
#define CSID_C_DARFONLOGGER_ROM_RULE_Error_COD4  	_CFG_DARFONLOGGER(0x0E00) //Error_COD4
#define CSID_C_DARFONLOGGER_ROM_RULE_Error_COD5  	_CFG_DARFONLOGGER(0x0F00) //Error_COD5
#define CSID_C_DARFONLOGGER_ROM_RULE_Error_COD6  	_CFG_DARFONLOGGER(0x1000) //Error_COD6
#define CSID_C_DARFONLOGGER_ROM_RULE_MI_DATA  		_CFG_DARFONLOGGER(0x1100) //Error_COD6
*/



/*
typedef struct
{
    char   	name[20];
	double 	value[4];
	char   	UnitName[20];
	char 	csman_value[4];

	unsigned int csidindex;

} s_EnergyData, *ps_EnergyData;

*/


static void _mymkdir(const char *dir) {
        char tmp[256];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, S_IRWXU);
                        *p = '/';
                }
        mkdir(tmp, S_IRWXU);
}

int DebugLog(int clear, char* info)
  {
	  FILE *out;
	  int i;
	  char confpath[100];

	  sprintf (confpath, "/tmp/DarfonLoggerDebugLog");



	  if(DebuglogLineCount>500)
	  {
		DebuglogLineCount=0;
		clear=1;
	  }

	  if(clear==1)
	  {
		  if((out = fopen(confpath, "w+")) == NULL)
		  {
			  fprintf(stderr, "***> Open error on output file %s", confpath);
			  return 0;
		  }
		  //return 1;
	  }

	  if((out = fopen(confpath, "a+")) == NULL)
	  {
		  fprintf(stderr, "***> Open error on output file %s", confpath);
		  return 0;
	  }

	  time_t now = time (0);
	  char datetimestr[100];
      strftime (datetimestr, 100, "%Y-%m-%d %H:%M:%S ==>", localtime (&now));

	  if(strlen(info)!=0)
	  {
		  fprintf(out, "%s%s\n",datetimestr,info);
		  DebuglogLineCount++;
	  }
	  fclose(out);


//usb
#ifdef MI_ReadDebugData

	  sprintf (confpath, "%s/DarfonLoggerDebugLog",CSVFileMainFolderPath);

	  if((out = fopen(confpath, "a+")) == NULL)
	  {
		  fprintf(stderr, "***> Open error on output file %s", confpath);
		  return 0;
	  }

	  now = time (0);
      strftime (datetimestr, 100, "%Y-%m-%d %H:%M:%S ==>", localtime (&now));

	  if(strlen(info)!=0)
	  {
		  fprintf(out, "%s%s\n",datetimestr,info);
	  }
	  fclose(out);
#endif
//


  }


///////////////rtc
char *rtc_dev_name;

static int rtc_dev_fd = -1;

static void close_rtc(void)
{
	if (rtc_dev_fd != -1)
		close(rtc_dev_fd);
	rtc_dev_fd = -1;
}

static int open_rtc(void)
{
	char *fls[] = {
#ifdef __ia64__
		"/dev/efirtc",
		"/dev/misc/efirtc",
#endif
		"/dev/rtc",
		"/dev/rtc0",
		"/dev/misc/rtc",
		NULL
	};
	char **p;

	if (rtc_dev_fd != -1)
		return rtc_dev_fd;

	/* --rtc option has been given */
	if (rtc_dev_name)
		rtc_dev_fd = open(rtc_dev_name, O_RDONLY);
	else {
		for (p = fls; *p; ++p) {
			rtc_dev_fd = open(*p, O_RDONLY);

			if (rtc_dev_fd < 0
			    && (errno == ENOENT || errno == ENODEV))
				continue;
			rtc_dev_name = *p;
			break;
		}
		if (rtc_dev_fd < 0)
			rtc_dev_name = *fls;	/* default for error messages */
	}

	if (rtc_dev_fd != 1)
		atexit(close_rtc);
	return rtc_dev_fd;
}

///////////////





















void split(char **arr, char *str, const char *del)
{
	char *s = strtok(str, del);
	//printf("split1=   (%s)\n",s);

	while(s != NULL) {
	*arr++ = s;
	s = strtok(NULL, del);
    //if(s != NULL)
    //printf("split2=   (%s)\n",s);
	}

 }



char Timeformat[40]="%Y-%m-%d %H:%M:%S";


void PrintTime(time_t time)
{
	char datetime[20];
	Convert_time_t_to_Str(time,datetime);
	DEBUG2(printf("PrintTime %s\n",datetime));
}

void Convert_time_t_to_Str(time_t time, char* timestr)
{
	//2014-11-26 10:50:08
	strftime (timestr, 100, Timeformat, localtime (&time));
    //printf ("%s\n", timestr);
}

time_t Convert_Str_to_time_t(char* timestr)
{
	printf("Convert_Str_to_time_t from %s\n",timestr);
	struct tm lt;
	strptime(timestr, Timeformat, &lt);
	return mktime(&lt);
	//*time= mktime(&lt);
	//memcpy(time, mktime(&lt), sizeof(time_t));

}





int DeleteFile(char* file_name)
{
	int status;
	//char file_name[25];

	//printf("Enter the name of file you wish to delete\n");
	//gets(file_name);

	status = remove(file_name);

	if( status == 0 )
	  printf("%s file deleted successfully.\n",file_name);
	else
	{
	  printf("Unable to delete the file\n");
	  //perror("Error");
	}

	return 0;
}





void CheckUSBStorageAlive()
{
    char cmd[50]="", line[100];
    char *test;
    FILE *fp;

    if(IfSaveDataToUsbStorage==true)
        return;

    memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"df | grep '%s' | awk '{print $6}'>%s",UsbStorageFolderPath, DarfonloggerUSBStoragePathFile);
	LanchCMD(cmd);


    if((fp=fopen(DarfonloggerUSBStoragePathFile,"r"))==NULL)
    {
        printf("CheckUSBStorageAlive: file not exits\n");
		IfSaveDataToUsbStorage=false;
        return;
    }

    while(!feof(fp))
    {
        memset(line,0,sizeof(line));
        fgets(line,100,fp);
        DEBUG2(printf("CheckUSBStorageAlive: line=%s\n",line));
        test = strstr(line, UsbStorageFolderPath);
        //DEBUG2(printf("CheckUSBStorageAlive: test=%s\n",test));
        if(test!=NULL)
            IfSaveDataToUsbStorage=true;
        else
            IfSaveDataToUsbStorage=false;
        break;
    }
    fclose(fp);
}



bool CheckReadResponseFunctionCodeandSlaveID(ps_Inverter inverter)
{
	if(inverter->Address!=mrPacket.mData[0] || mrPacket.mData[1]!=0x03)
		return false;
	else
		return true;
}


/*
unsigned int =0x0200;
unsigned int =0x0207;
unsigned int =0x020A;
unsigned int =0x020E;
unsigned int =0x020F;
unsigned int =0x0210;
unsigned int =0x0211;
unsigned int =0x0213;
unsigned int =0x0214;
unsigned int =0x0215;
unsigned int =0x0217;
unsigned int =0x0218;
unsigned int =0x0219;
unsigned int =0x021A;
unsigned int =0x021B;
unsigned int =0x021C;
unsigned int =0x021D;

*/













int CheckSN( char* SN)
{
    int i;



    if(strlen(SN)==16)
    {
        if(strncmp(SN,"0000000000000000",16)==0 ||
           strncmp(SN,"FFFFFFFFFFFFFFFF",16)==0)
                return 0;

        for(i=0;i<16;i++)
        {
            if(!isxdigit(SN[i]))
            {
                printf("Wrong SN %s wrong char = %c\n",SN,SN[i]);
                return 0;
            }
        }
        return 1;
    }
    else
    {
         printf("Wrong SN %s len=%d\n",SN,strlen(SN));
    }
    return 0;
}

void BuildMessage(int SlaveID, unsigned char FunctionCode,int StartAddress,int RegisterQuantity)
{
    unsigned short crc;
    txbuffer[0] = SlaveID;
    txbuffer[1] = FunctionCode;
    txbuffer[2]= (unsigned char) (StartAddress >> 8);
    txbuffer[3]= (unsigned char) (StartAddress);
    txbuffer[4]= (unsigned char) (RegisterQuantity >> 8);
    txbuffer[5]= (unsigned char) (RegisterQuantity);
    crc=CalculateCRC(&(txbuffer[0]), txsize-2);
    txbuffer[txsize - 2]= (unsigned char) (crc >> 8);
    txbuffer[txsize - 1]= (unsigned char) (crc&0x00ff);
}



void PrintTxBuffer()
{
    int i;
    printf("TX size=%d\n",txsize);
    printf("-> ");
    for(i=0;i<txsize;i++)
    {
        printf("%02x ",txbuffer[i]);
    }
    printf("\n");
}

void PrintRxPackage()
{
    int i;
    printf("<- ");

    for(i=0;i<(2+mrLen+2);i++)
    {
        printf("%02x ",mrPacket.mData[i]);
    }
    printf("\n");
}

void PrintArray(char* a[],int size)
{
    int i;
    for(i=0;i<size;i++)
    {
        printf("%s ",a[i]);
    }
    printf("\n");
}

void PrintArrayInt(int a[],int size)
{
    int i;
    for(i=0;i<size;i++)
    {
        printf("%d ",a[i]);
    }
    printf("\n");
}



int power(int x,int n)
{
    int i;
    int num = 1;
    for(i=1;i<=n;i++)
        num*=x;
    return num;
}

int transfer_string_to_hex(unsigned char *str_name)
{
    char string[]="0123456789ABCDEF";
    int number[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    int i = 0;
    int j = 0;
    int str_number = 0;
    for(i=0; i<sizeof(str_name); i++)
    {
        for(j=0; j<sizeof(string); j++)
        {
            if(toupper(str_name[i]) == string[j])
            {
                str_number += power(16, (sizeof(str_name)-1-i))* number[j];
                break;
            }
        }
    }
    return str_number;
}
 void substr(char *dest, const char* src, unsigned int start, unsigned int cnt)
 {
    strncpy(dest, src + start, cnt);
    dest[cnt] = 0;
 }
/*
int AssignAddress(const char* ID)
{
    int errorcount=0;
    while(errorcount<3)
    {
        SendRemoveAllRegisterQuery();
        usleep(500000);//0.5s
        SendRemoveAllRegisterQuery();
        usleep(500000);//0.5s
        SendRemoveAllRegisterQuery();
        usleep(500000);//0.5s

        MClearRX();
        char* UniIDchar[8];
        int i;
        for(i=0;i<8;i++)
        {
            //UniIDchar[i]=malloc( 3 * sizeof(char) );
            UniIDchar[i]=(char *)new(char,3);
            memset(UniIDchar[i],0,3);
        }
        char* UniIDchar16[8];
        for(i=0;i<8;i++)
        {
            //UniIDchar16[i]=malloc( 5 * sizeof(char) );
            UniIDchar16[i]=(char *)new(char,5);
            memset(UniIDchar16[i],0,5);
        }

        byte UniID[8];

        //PrintArray(UniIDchar,8);
        DEBUG2(printf("%s\n",ID));

        strncpy(UniIDchar[0], ID, 2);
        strncpy(UniIDchar[1], ID+2, 2);
        strncpy(UniIDchar[2], ID+4, 2);
        strncpy(UniIDchar[3], ID+6, 2);
        strncpy(UniIDchar[4], ID+8, 2);
        strncpy(UniIDchar[5], ID+10, 2);
        strncpy(UniIDchar[6], ID+12, 2);
        strncpy(UniIDchar[7], ID+14, 2);

        sprintf (UniIDchar16[0], "0x%s",UniIDchar[0]);
        sprintf (UniIDchar16[1], "0x%s",UniIDchar[1]);
        sprintf (UniIDchar16[2], "0x%s",UniIDchar[2]);
        sprintf (UniIDchar16[3], "0x%s",UniIDchar[3]);
        sprintf (UniIDchar16[4], "0x%s",UniIDchar[4]);
        sprintf (UniIDchar16[5], "0x%s",UniIDchar[5]);
        sprintf (UniIDchar16[6], "0x%s",UniIDchar[6]);
        sprintf (UniIDchar16[7], "0x%s",UniIDchar[7]);
        //PrintArray(UniIDchar16,8);
        UniID[0]=transfer_string_to_hex (UniIDchar16[0]);
        UniID[1]=transfer_string_to_hex (UniIDchar16[1]);
        UniID[2]=transfer_string_to_hex (UniIDchar16[2]);
        UniID[3]=transfer_string_to_hex (UniIDchar16[3]);
        UniID[4]=transfer_string_to_hex (UniIDchar16[4]);
        UniID[5]=transfer_string_to_hex (UniIDchar16[5]);
        UniID[6]=transfer_string_to_hex (UniIDchar16[6]);
        UniID[7]=transfer_string_to_hex (UniIDchar16[7]);
        //PrintArrayInt(UniID,8);
        SendAllocatedAddress(UniID, DefaultSlaveID);

        for(i=0;i<8;i++)
            del(UniIDchar[i],char,3);
        for(i=0;i<8;i++)
            del(UniIDchar16[i],char,3);


        if(CheckResponse(2)==0)
          	errorcount++;
        else
    	       return 1;

        //MClearRX();
    }
    return 0;
}
*/

/*
void SendSyncOffLineQuery()
{
    writeLog("SendSyncOffLineQuery gogogo!");
    MClearRX();
    txsize=8;
    waitFCode = 0x00;
    BuildMessage(0, 0x00, 0x05, CurrentUsedglobesetting.RegisterMODValue);
    MStartTX();
    writeLog("SendSyncOffLineQuery ououou!");
}
*/
/*
void SyncOffLineQuery()
{
	printf("Darfonlogger:SyncOffLineQuery\n");
    SendSyncOffLineQuery();
	//ModbusResponseStatus rError, rSuccess, rNoresponse

	ModbusResponseStatus status=CheckResponse(commandtimeinterval);
	if(status==rError)
  	{
  		CurrentUsedglobesetting.RegisterResponseFailCount++;
	}
	else if(status==rSuccess)
	{
		CurrentUsedglobesetting.RegisterResponseSuccessCount++;
		AddMItoInveterlist();
		//SendAddressToInverters();
	}
	else if(status==rNoresponse)
	{
		CurrentUsedglobesetting.RegisterNoResponseCount++;
	}
	//PrintInverterlist("SyncOffLineQuery",false);
}
*/
/*
SendOffLineQuery()
{
    MClearRX();
    txsize=8;
    waitFCode = 0x00;
    BuildMessage(0, 0x00, 0x00, 4);
    MStartTX();
}
*/
/*
OffLineQuery()
{
	//printf("Darfonlogger:OffLineQuery\n");
    SendOffLineQuery();
	ModbusResponseStatus status=CheckResponse(commandtimeinterval);
	if(status==rError)
  	{
  		CurrentUsedglobesetting.RegisterResponseFailCount++;
	}
	else if(status==rSuccess)
	{
		CurrentUsedglobesetting.RegisterResponseSuccessCount++;
		AddMItoInveterlist();
	}
	else if(status==rNoresponse)
	{
		CurrentUsedglobesetting.RegisterNoResponseCount++;
	}
}
*/




void SendRemoveAllRegisterQuery()
{
    printf("SendRemoveAllRegisterQuery enter ");
    MClearRX();
    txsize=8;
    waitFCode = 0xFE;
    BuildMessage(0, 0xFE, 0x02, 0);
    DEBUG2(printf("SendRemoveAllRegisterQuery "));
    MStartTX();
    printf("SendRemoveAllRegisterQuery exit");
}
void SendAllocatedAddress(byte UniID[], unsigned int AllocatedAddress)
{
    byte  buf[1024];
    byte  messageZigbee[19+16];
    MClearRX();
    txsize=19;
    waitAddr=0x00;
    waitFCode = 0xff;
    unsigned short crc;
    txbuffer[0] = 0x00;
    txbuffer[1] = 0xff;
    txbuffer[2]= 0x00;
    txbuffer[3]= 0x01;
    txbuffer[4]= 0x00;
    txbuffer[5]= 0x05;
    txbuffer[6]= 0x0a;
    int i;
    for (i = 0; i < 8; i++)
    {
        txbuffer[i + 7] = UniID[i];
    }
    txbuffer[txsize - 4]= (unsigned char) (AllocatedAddress);
    txbuffer[txsize - 3]= (unsigned char) (AllocatedAddress);
    crc=CalculateCRC(&(txbuffer[0]), txsize-2);
    txbuffer[txsize - 2]= (unsigned char) (crc >> 8);
    txbuffer[txsize - 1]= (unsigned char) (crc&0x00ff);
    DEBUG2(printf("SendAllocatedAddress "));
    printf("SendAllocatedAddress\n");

    //printf(txbuffer);
    MStartTX();
}




ModbusResponseStatus CheckResponse(float waittime_s)
{

	DEBUG2(printf("\nCheckResponse\n"));
	ModbusResponseStatus status=rNoresponse;
    unsigned long i=0;
    unsigned long totalus=waittime_s*1000000;
    unsigned long gap=10000;//10ms 0.01s
    while( TRC_RECEIVED==0 && i<=(totalus/gap))
    {
        if(TRC_RXERROR==1)
            break;
        //lySleep(gap);//0.01s
        usleep(10000);//0.01s
        i++;
    }

    float time;
    time=(0.01)*i;
    if(TRC_RXERROR==1)
    {
        DEBUG2(printf("CheckResponse Error %.2f s\n",time));
		status=rError;
   	}
    else if(TRC_RECEIVED==0)
    {
        DEBUG2(printf("CheckResponse No Response %.2f s\n",time));
		status=rNoresponse;
   	}
    else
    {
        DEBUG2(printf("CheckResponse success,cost  %.2f s\n",time));
		status=rSuccess;

   	}

    return status;
}



int ReadParameterFromFile(unsigned int parameter[],const char *file, int Length)
{
    FILE *in;
    char buffer[200];
    char *delim = ",";
    char * pch;
    if (!(in = fopen(file, "r")))
    {
        printf("unable to open config file: %s", file);
        return 0;
    }
    while (fgets(buffer, 200, in))
    {
         //printf(buffer);
        //printf ("Splitting string \"%s\" into tokens:\n",buffer);
        pch = strtok(buffer,delim);
        int i=0;
        while (pch != NULL)
        {
            parameter[i]=atoi(pch);
            i++;
            //printf ("%s\n",pch);
            pch = strtok (NULL, delim);
        }
        if(i!=Length)
        {
            printf("Read config file: %s fail i=%d\n", file,i);
            return 0;
        }
        else
        {
            DEBUG2(printf("Read config file: %s success\n", file));
            return 1;
        }
    }
}

int AddSuccessNote(const char* ID, int success, int clear, char* info)
{
    FILE *out;
    int i;
    char confpath[1024];
    char cwd[1024]="";
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        sprintf (confpath, "%s/report",cwd);
    }

    if(clear==1)
    {
        if((out = fopen(confpath, "w+")) == NULL)
        {
            fprintf(stderr, "***> Open error on output file %s", confpath);
            return 0;
        }
        return 1;
    }

    if((out = fopen(confpath, "a+")) == NULL)
    {
        fprintf(stderr, "***> Open error on output file %s", confpath);
        return 0;
    }
    if(strlen(info)!=0)
        fprintf(out, "%s %d %s\n", ID,success,info);
    else
        fprintf(out, "%s %d\n", ID,success);

    fclose(out);

}
/*
int SendHexFile( FILE * pFile, eFamily Family)
{
    char Buffer[BUFFER_SIZE];
    int  ExtAddr = 0;

    int Row;
    for(Row = 0; Row < PM_SIZE; Row++)
    {
        Init_mem_cMemRow(Row, Program, 0x000000, Row, Family);//mike*ppMemory[Row] = new mem_cMemRow(mem_cMemRow::Program, 0x000000, Row, Family);
    }

    for(Row = 0; Row < EE_SIZE; Row++)
    {
        Init_mem_cMemRow(Row + PM_SIZE, EEProm, 0x7FF000, Row, Family);
    }

    for(Row = 0; Row < CM_SIZE; Row++)
    {
        Init_mem_cMemRow(Row + PM_SIZE + EE_SIZE, Configuration, 0xF80000, Row, Family);
    }

    printf("\nReading HexFile");

    while(fgets(Buffer, sizeof(Buffer), pFile) != NULL)
    {
        int ByteCount;
        int Address;
        int RecordType;

        sscanf(Buffer+1, "%2x%4x%2x", &ByteCount, &Address, &RecordType);

        if(RecordType == 0)
        {
            Address = (Address + ExtAddr) / 2;
            int CharCount;
            for(CharCount = 0; CharCount < ByteCount*2; CharCount += 4, Address++)
            {
                int bInserted = 0;

                for(Row = 0; Row < (PM_SIZE + EE_SIZE + CM_SIZE); Row++)
                {
                    if((bInserted = InsertData(Row, Address, Buffer + 9 + CharCount)) == 1)
                    {
                        break;
                    }
                }

                if(bInserted != 1)
                {
                    printf("Bad Hex file: 0x%xAddress out of range\n", Address);
                }
            }
        }
        else if(RecordType == 1)
        {
        }
        else if(RecordType == 4)
        {
            sscanf(Buffer+9, "%4x", &ExtAddr);

            ExtAddr = ExtAddr << 16;
        }
        else
        {
            printf("\n[Error]:Unknown hex record type");
            return 0;
        }
    }


    {
        char Data[32];
        int          RowSize;

        if(Family == dsPIC30F)
        {
        	RowSize = PM30F_ROW_SIZE;
        }
        else
        {
        	RowSize = PM33F_ROW_SIZE;
        }

        //read pm{

        char  ModbusPMResponse[1536+8];
        memset(ModbusPMResponse, 0, 1544);
        int errorcount=0;
        int getresponse=0;

        while(getresponse==0)
        {
        	if(BootloaderVersion==0)
			{
	            printf("\nReading Target 0x%X\n",0);
	            usleep(300000);
	            WriteCommBlockModBus(0, COMMAND_READ_PM, Buffer, 0, UserCodeSlaveID_For_Bootloader);

	            if(CheckResponse(10)==0)
	                errorcount++;
	            else
	            {
	                getresponse=1;
					memcpy(ModbusPMResponse, mrPacket.mData, 1544);
	            }
        	}
			else//new bootloader
			{
				printf("\nReading Target 0x%X\n",0);
				getresponse=DarfonNewProtocolReadPMBlock(0, ModbusPMResponse+6 , UserCodeSlaveID_For_Bootloader,Family);
				if(getresponse==0)
					errorcount++;

			}

            if(errorcount>3)
            {
                printf("\n[Error]:Check CRC Fail[COMMAND_READ_PM]");
                return 0;
            }
    	}




        int modbusdataoffset=6;
        sprintf(Data, "%02x%02x%02x00%02x%02x%02x00",     ModbusPMResponse[2+modbusdataoffset] & 0xFF,
                                                                                            ModbusPMResponse[1+modbusdataoffset] & 0xFF,
                                                                                            ModbusPMResponse[0+modbusdataoffset] & 0xFF,
                                                                                            ModbusPMResponse[5+modbusdataoffset] & 0xFF,
                                                                                            ModbusPMResponse[4+modbusdataoffset] & 0xFF,
                                                                                            ModbusPMResponse[3+modbusdataoffset] & 0xFF);

        InsertData(0,0x000000, Data);
        InsertData(0,0x000001, Data + 4);
        InsertData(0,0x000002, Data + 8);
        InsertData(0,0x000003, Data + 12);
    }

    for(Row = 0; Row < (PM_SIZE + EE_SIZE + CM_SIZE); Row++)
    {
        FormatData(Row);
    }

    printf("\nProgramming Device\n");

    for( Row = 0; Row < (PM_SIZE + EE_SIZE + CM_SIZE); Row++)
    {
        if(SendData(Row )==0)
            return 0;
    }

    for(Row = 0; Row < (PM_SIZE + EE_SIZE + CM_SIZE); Row++)
    {
        free(pMemory[Row].m_pBuffer);
    }

    usleep(100000);
    if(BootloaderResetInverterModbus(10)==0) //mike+
        return 0;
    usleep(100000);

    return 1;
}
*/

void StopAndShowMessage( char * pMessage,int showstop)
{
    printf("\n%s",pMessage);
    if(showstop==1)
        printf("\nStop");

}

int WriteCommBlockModBus( unsigned int pStartAddress,int functioncode, char *pBuffer , int BytesToWrite, int SlaveID)
{
    int TxNum=0;
    int TxCount=0;
    int modbusmessagesize = BytesToWrite+8;

    if(functioncode==COMMAND_READ_ID || functioncode==COMMAND_READ_PM ||functioncode==COMMAND_READ_PM_PAGE || functioncode==COMMAND_READ_BOOTLOADER_VERSION)
        modbusmessagesize = 8;
	if(functioncode==COMMAND_WRITE_PM_FLUSH)
		modbusmessagesize = 9;
	if(functioncode==COMMAND_WRITE_PM_PAGE)
		modbusmessagesize = 8+PageSize;
    if(functioncode==COMMAND_READ_PM || functioncode==COMMAND_WRITE_PM || functioncode==COMMAND_WRITE_CM)
        BytesToWrite = 0xff;


    m_pModbusBuffer   = (char *)malloc(1544);
    memset(m_pModbusBuffer, 0, 1544);
    unsigned int Address = pStartAddress;
    m_pModbusBuffer[0] = SlaveID;
    m_pModbusBuffer[1] = functioncode;
    m_pModbusBuffer[2] = (Address)       & 0xFF;
    m_pModbusBuffer[3] = (Address >> 8)  & 0xFF;
    m_pModbusBuffer[4] = (Address >> 16) & 0xFF;
    m_pModbusBuffer[5] =BytesToWrite;//BytesToWrite;max 255 so can not enter 1536



    if(functioncode==COMMAND_WRITE_PM )
        memcpy(m_pModbusBuffer+6, pBuffer, 1536);
	else if(functioncode==COMMAND_WRITE_PM_PAGE )
		memcpy(m_pModbusBuffer+6, pBuffer, PageSize);
	else if(functioncode==COMMAND_WRITE_PM_FLUSH )
		memcpy(m_pModbusBuffer+5, pBuffer, 2);
	else if(functioncode==COMMAND_READ_PM_PAGE )
		m_pModbusBuffer[5] =BytesToWrite;
    else
        memcpy(m_pModbusBuffer+6, pBuffer, BytesToWrite);


    unsigned short crc;
    crc=CalculateCRC(m_pModbusBuffer, modbusmessagesize-2);
    m_pModbusBuffer[modbusmessagesize-2] = (unsigned char)(crc>>8);
    m_pModbusBuffer[modbusmessagesize-1] = (unsigned char)(crc&0xFF);



    int index;
    int packagecount=0;
    for(index=0;index<modbusmessagesize; index+=oncenum)
    {
        DEBUG2(printf("."));
        MClearRX();
        if((index+oncenum) < modbusmessagesize)
        {
            TxCount+=oncenum;
            txsize=oncenum;
            //printf("\nFc0x10txsize=%d\n",txsize);
            waitFCode = functioncode;
            waitAddr=SlaveID;
            memcpy(txbuffer, m_pModbusBuffer+index, oncenum);
            MStartTX();
            if(oncenum==180)
                usleep(150000);//mike*800
            else
                usleep(150000);//mike*400
        }
        else
        {
            TxCount+=(modbusmessagesize-index);
            txsize=modbusmessagesize-index;
            //printf("\nFc0x10txsize=%d\n",txsize);
            waitFCode = functioncode;
            waitAddr=SlaveID;
            memcpy(txbuffer, m_pModbusBuffer+index, modbusmessagesize-index);
            MStartTX();

            if(oncenum==180)
                usleep(150000);//mike*800
            else
                usleep(150000);//mike*400
        }
        packagecount++;
    }
    DEBUG2(printf("\npackagecount=%d\n",packagecount));
    free(m_pModbusBuffer);
    return 1;
}
//mike+}

int BootloaderResetInverterModbus(int count)
{
    char Buffer[1]={0};
    int errorcount=0;
    while(errorcount<count)
    {
        printf("Reset\n");
        WriteCommBlockModBus( 0xF8, COMMAND_RESET, Buffer, 0, UserCodeSlaveID_For_Bootloader);
        char CheckACKBuffer[9] = {1,1,1,1,1,1,1,1,1};
        if(CheckResponse(1)==0)
        {
            DEBUG2(printf("\nNo ACK\n"));
            errorcount++;
        }
        else
        {
            memset(CheckACKBuffer, 0, 9);
            memcpy(CheckACKBuffer, mrPacket.mData, 9);
            DEBUG2(printf("\nGot ACK\n"));
            return 1;
        }
    }
    printf("\nReset fail\n");
    return 0;
}

eFamily ReadIDModbus(unsigned int *version)
{
    char                Buffer[1];
    unsigned short int  DeviceId = 0;
    unsigned short int  ProcessId = 0;

    WriteCommBlockModBus(0xff, COMMAND_READ_ID, Buffer, 8, UserCodeSlaveID_For_Bootloader);
	//if(checkresponse==0)
		//return Unknow;

	if(CheckResponse(1)==0)
    {
        *version=0;
        return Unknow;
    }
    int modbusdataoffset=6;
    DeviceId  = ((mrPacket.mData[1+modbusdataoffset] << 8)&0xFF00) | (mrPacket.mData[0+modbusdataoffset]&0x00FF);
    ProcessId = (mrPacket.mData[5+modbusdataoffset] >> 4) & 0x0F;

    int Count         = 0;
    int bDeviceFound = 0;

    while(bDeviceFound != 1)
    {
        if(Device[Count].pName == NULL)
        {
            break;
        }

        if((Device[Count].Id == DeviceId) && (Device[Count].ProcessId == ProcessId))
        {
            bDeviceFound = 1;
            break;
        }

        Count++;
    }

    if(bDeviceFound != 1)
    {
    	*version=((mrPacket.mData[5+modbusdataoffset] << 8)&0xFF00) | (mrPacket.mData[4+modbusdataoffset]&0x00FF);
    	printf("Found Bootloader Version: 0x%04x\n", *version);
        return Unknow;
    }

    printf("Found %s (ID: 0x%04x)\n", Device[Count].pName, DeviceId);

    return(Device[Count].Family);
}

/*
ReadRegisterByModbus(int SlaveID,int StartAddress,int RegisterQuantity)
{
    byte ID[8], messageZigbee[24];
    MClearRX();
    txsize=8;
    waitFCode = 3;
    waitAddr=SlaveID;
    BuildMessage(SlaveID, waitFCode, StartAddress, RegisterQuantity);
    DEBUG2(printf("ReadRegisterByModbus "));

    if(CurrentUsedglobesetting.CommunicationBoxType==Zigbee)
    {
        GetIDByAddress(SlaveID,ID);
        if(ToUniqMIMessageZigbee(ID, txbuffer, txsize, messageZigbee,24))
        {
            memcpy(txbuffer, messageZigbee, 24);
            txsize=24;
            MStartTX();
        }
    }
    else
    {
        MStartTX();
    }
}
*/
/*

int CheckCommunication()
{
    int startaddres=0x200;
    int parameterCount=30;
    int SlaveID=DefaultSlaveID;

    int count=0;
    int lostcount=0;
    DEBUG2(printf("CheckPackageLost\n"));
    if(checkCommunication==0)
        return 1;
    while(count<100)
    {
        ReadRegisterByModbus(SlaveID,startaddres,parameterCount);
        if(CheckResponse(1)==0)
        {
            lostcount++;
        }
        else
        {
            ///
            int i;
            unsigned int readvalues[parameterCount];
            for (i = 0; i < parameterCount; i++)
            {
                readvalues[i] = mrPacket.mData[i*2+3] <<8;
                readvalues[i] += mrPacket.mData[i*2+3+1];
                int address=i+startaddres;
                if(address==0x020F)//"Vpv_A","labVpv_A",0x020F,10,(decimal )0.1M,"V","Vpv"
                {
                    if(readvalues[i]<VpvLimit)
                    {
                        float value;
                        value=readvalues[i]*0.1;
                        printf("CheckCommunication error Vpv=%.2f\n",value);
                        return 0;
                    }
                }
                else if(address==0x0215)//Pac_A","labPac_A",0x0215,1,(decimal )0.1M,"W","Pac",0,1),
                {
                    if(readvalues[i]<PacLimit)
                    {
                        float value;
                        value=readvalues[i]*0.1;
                        printf("CheckCommunication error Pac_A=%.2f\n",value);
                        return 0;
                    }
                }

            }
            ///
        }
        if(lostcount>5)
        {
            printf("Package lost count>5\n");
            return 0;
        }
        count++;
    }
    return 1;
}
*/




StringToLower(char *f)
{
    int i=0;
    for(i=0;i<strlen(f);i++)
    {
        *(f+i)=tolower(* (f+i));
    }

}








int getpidbyName(char *name)  //켹⑥?�役{㎸?Ωず①?
{
    printf("getpidbyName start\n");
    char line[1000],*key,*t_name;
	t_name=name;
	char cmd[1000];
	memset(cmd,'\x00',sizeof(cmd));

	DeleteFile(temptxtpath);

	//sprintf(cmd,"ps |grep %s >temp.txt",t_name);
	sprintf(cmd,"ps |grep '%s' | grep -v grep | awk '{print $1}'>%s",t_name,temptxtpath);
	LanchCMD(cmd);
    DebugLog(0,cmd);
    FILE *fp;
    int count;
    count=0;
    if((fp=fopen(temptxtpath,"r"))==NULL)
    {
        printf("file not exits\n");
		return 0;
     }

     while(!feof(fp))
     {
     	memset(line,'\x00',sizeof(line));
        fgets(line,100,fp);
        if(strlen(line)>0)
    	{
            count++;
            //printf("getpidbyNo %s",line);
    	}


     }

     return(count);
 }


void killProcessByName(char *name)  //켹⑥?�役{㎸?Ωず①?
{
    char line[1000],*key,*t_name;
	t_name=name;
	char cmd[1000];
	memset(cmd,'\x00',sizeof(cmd));
	//sprintf(cmd, "ps  |grep '%s'>temp.txt",t_name);
	//
	//DeleteFile(DarfonhttppostkillProcesspath);
	sprintf(cmd,"ps |grep '%s' | grep -v grep | awk '{print $1}'>%s",t_name,temptxtpath);
	sprintf(cmd,"ps |grep '%s' | grep -v grep | awk '{print $1}'>%s",t_name, DarfonloggerkillProcesspath);
	LanchCMD(cmd);
    FILE *fp;
    int count;
    count=0;
    if((fp=fopen(temptxtpath,"r"))==NULL)
    {
        printf("file not exits\n");
        //fclose(fp);
		return;
     }
     while(!feof(fp))
     {
     	memset(line,'\x00',sizeof(line));
        fgets(line,100,fp);
       	if(strlen(line)>0)
    	{
            count++;
            printf("%s",line);
            char * pch;
            char killcmd[20]="";
            sprintf(killcmd,"/bin/kill -1 %s",line);
			LanchCMD(killcmd);
			printf("Darfonlogger kill cmd:%s\n",killcmd);
			sleep(2);
			sprintf(killcmd,"/bin/kill -9 %s",line);
			LanchCMD(killcmd);
            printf("Darfonlogger kill cmd:%s\n",killcmd);

        }

     }
     fclose(fp);



 }


int Popen(const char* cmd_line)
{
    FILE* fp_ = popen(cmd_line,"r");
    //assert(fp_);
    char buf_[4096];
    fread(buf_,sizeof(buf_),1,fp_);
    return pclose(fp_);
}



int  LanchCMD(char* cmd)
{
/*
    sig_t savesig;
    FILE *f;

    savesig = signal(SIGCHLD, SIG_DFL);
    f = popen(cmd, "r");

    if (f == NULL)
    {
        signal(SIGCHLD, savesig);
        return false;
    }
    pclose(f);
    return true;
*/
    /*
        system(cmd);
    */

    int ret = 0;
    __sighandler_t old_handler;
    old_handler = signal(SIGCHLD, SIG_DFL);
    ret = Popen(cmd);
    signal(SIGCHLD, old_handler);
    return ret;

}

bool pingurl(char* url)
{
	int processcount=0;
	char pingcmd[50];
	char result[10];


	sprintf(pingcmd,"ping -c 1 %s | grep received | awk '{print $4}'",url);


	printf("Ping %s....\n",pingcmd);

	signal(SIGALRM, alarm_func);//mike+
	printf("pingurl debug1\n");
	if(sigsetjmp(jmpbuf, 1) != 0)
	{
	    printf("pingurl debug2\n");
		alarm(0);//timout
		signal(SIGALRM, SIG_IGN);
        printf("pingurl debug3\n");
		printf("Ping timeout\n");
		////////////////
		processcount=getpidbyName(url);
		if(processcount>0)
		{
			DebugLog(0,"kill multi process(PING)1");
			killProcessByName(url);
		}
	   	return false;
	}
	alarm(3);//setting alarm
	//mike20141204+}
	printf("pingurl debug4\n");
	FILE *ls = popen(pingcmd, "r");


	while (fgets(result, sizeof(result), ls) != NULL) {

		 printf("ping result=%s size=%d\n", result,sizeof(result));
		 break;
	}
    printf("pingurl debug5\n");
	pclose(ls);
    printf("pingurl debug6\n");
	signal(SIGALRM, SIG_IGN);///mike20141204+
	printf("Ping %s...end\n",url);
	////////////////
	processcount=getpidbyName(url);
	if(processcount>0)
	{
		DebugLog(0,"kill multi process(PING)2");
		killProcessByName(url);
	}
	if(strncmp(result,"1",1)==0)
		return true;
	else
		return false;

 }


 void RemoveAllRegister( uint count)
{
	int i=0;
	for(i=0;i<count;i++)
	{
		SendRemoveAllRegisterQuery();
		usleep(500000);
		//CheckResponse(commandtimeinterval);
	}
}



void MakeReadDataCRC(unsigned char *pbuf,int tsize)
{
    unsigned short crc;
    crc=CalculateCRC(pbuf, tsize-2);
    *(pbuf+tsize-2)= (unsigned char) (crc >> 8);
    *(pbuf+tsize-1)= (unsigned char) (crc&0x00ff);

}


// for G320, G640
void MyGetMIInfo()
{
	int i,i2;
	byte *lpdata;
	char sEtotal[] = "Eac_Total" ;
	char tempData[100],MIROMData[1000],LastError[50],CommunicationStatus[50];
    char csmanvalue[20];
    char  szXX[]={0x03, 0x03, 0x00, 0x01, 0x00, 0x07, 0x00, 0x00};

    MakeReadDataCRC(szXX,8);
    txsize = 8;
    memcpy(txbuffer, szXX, 8);
    //while (1) {
	  MStartTX();
	  usleep(1000000);
	  lpdata = GetRespond(19, 200);
	  if (lpdata) {
	    DumpMiInfo(lpdata);
        free(lpdata);
	  }
     // usleep(300000);
    //}

}

// for G320, G640
void MyWriteAllMIDataToRAM(bool force, bool UpdateAll)
{
	int i,i2;
	byte *lpdata;
	char sEtotal[] = "Eac_Total" ;
	char tempData[100],MIROMData[1000],LastError[50],CommunicationStatus[50];
    char csmanvalue[20];

    char  szMIinfo[]={0x03, 0x03, 0x00, 0x01, 0x00, 0x07, 0x00, 0x00};
    MakeReadDataCRC(szMIinfo,8);

    char  szXX[]={0x03, 0x03, 0x02, 0x00, 0x00, 0x21, 0xc5, 0x98};

    MakeReadDataCRC(szXX,8);
    txsize = 8;


    while (1) {
      memcpy(txbuffer, szMIinfo, 8);
	  MStartTX();
	  usleep(1000000);
	  lpdata = GetRespond(19, 200);
	  if (lpdata) {
	    DumpMiInfo(lpdata, &g_miInfo);
        free(lpdata);
	  } else {
	    break; // error
	  }
      memcpy(txbuffer, szXX, 8);
	  MStartTX();
	  usleep(1000000);

	  lpdata = GetRespond(71, 200);
	  /*
	  if (lpdata) {
	    Dumpdata(lpdata, &g_miInfo);
        free(lpdata);
	  } else {
	    break; // error
	  }
	  */
	//  PostData(lpdata);
      usleep(g_fetchtime*1000000);
    }

}
int MyAssignAddress(const char* ID)
{
    byte  *pdata;
    int errorcount=0;
    while(errorcount<3)
    {
        MClearRX();

        DEBUG2(printf("%s\n",ID));

        DefaultSlaveID=3;
        SendAllocatedAddress(ID, DefaultSlaveID);
        usleep(1000000);
        pdata = GetRespond(8, 200);
        if (!pdata) {
            //free(pdata);
            errorcount++;
            return 1;
        }
    }
    return 0;
}

int MyStartRegisterProcess(bool ckecktimeout)
{
	int RegisterLoopCount = 0;
    int DefaultMODValue=20;
	int i,Synctime;



    writeLog("MyStartRegisterProcess: enter! ");

    char szbuf[8]={0x00,0x05,0x00,0x01,0x00,0x00,0x00,0xFF};
    int trytimes=0;
    while (trytimes <3) {
       //writeLog("MyAssignAddress: enter!\n ");
printf(g_plcid);
       if (MyAssignAddress(g_plcid)==1) {
            break;
       }
       trytimes++;
       if (trytimes >= 3) {
        return 0;
       }
    }
	//SendAddressToInverters(1);
	//MyGetMIInfo();
	MyWriteAllMIDataToRAM(false,false);

}

void MyLogger(char *port )
{

    char szbuf[32];
    sprintf(szbuf,"port = %s \n",port);
    printf(szbuf);
    getchar();
   // getchar();
    MyModbusDrvInit(port);
   // getchar();


    writeLog("RemoveAllRegister");
    RemoveAllRegister(3);

    writeLog("MyStartRegisterProcess\n");
    MyStartRegisterProcess(false);

	ModbusDrvDeinit();

}


char   *szPort[]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2","/dev/ttyS3","/sys/bus/usb/devices/usb2"};
void MyStart()
{

	//InitCurrentUsedglobesetting();
	//ReadRegisterMode(true);
	getenvdata();
	parse_ini_file("config.ini");

	g_port = 4;
    MyLogger(szPort[g_port]);
}

void writeLog (char *message)
{

    printf("debug: %s \n",message);
    /*
	FILE *file;

	if (!LogCreated) {
		file = fopen(LOGFILE, "w");
		LogCreated = true;
	}
	else
		file = fopen(LOGFILE, "a");

	if (file == NULL) {
		if (LogCreated)
			LogCreated = false;
		return;
	}
	else
	{
		fputs(message, file);
		fclose(file);
	}
unsigned short crc;
    txbuffer[0] = 0xff;
    txbuffer[1] = 0xfe;
    txbuffer[2]= type;
    txbuffer[3]= index;

    txbuffer[txsize - 4]= 0xfe;
    txbuffer[txsize - 3]= 0xff;
    crc=CalculateCRC(&(txbuffer[0]), txsize-2);
    txbuffer[txsize - 2]= (unsigned char) (crc >> 8);
    txbuffer[txsize - 1]= (unsigned char) (crc&0x00ff);
	if (file)
		fclose(file);
    */
}

void Dumpdata(char *lpdata, MI_INFO *pInfo)
{
    unsigned char *data;
    char  szbuf[512];
    sprintf(szbuf,"SleveID=%d, FuncCode=%d, ByteCount=%d \n",lpdata[0], lpdata[1],lpdata[2]);
    printf(szbuf);
      DebugPrint(lpdata, 1, "slaveID:");
      DebugPrint(lpdata+1, 1, "Func Code:");
      DebugPrint(lpdata+2, 1, "Byte count:");

      data = lpdata+3;
    g_miData.temperature = (float)data[0]; // 0.1 プ
    g_miData.year = data[1];
    g_miData.month = data[2];
    g_miData.day = data[3];

    sprintf(szbuf,"Temp=%d, date=%d, Hour=%d, Minute =%d \n",data[0], data[1],data[2],data[3]);
    printf(szbuf);

    g_miData.Eac1 = (float)data[4]*100+(float)data[5]/100;
    g_miData.Eac2 = (float)data[7]*100+(float)data[8]/100;
    g_miData.TEac = (float)data[10]*100+(float)data[11]/100;

    sprintf(szbuf,"Eac 1=%.2f, Eac2=%.2f TEac=%f \n",
            (float)data[4]*100+(float)data[5]/100,
            (float)data[7]*100+(float)data[8]/100,
            (float)data[10]*100+(float)data[11]/100 );
    printf(szbuf);

    g_miData.Pac1 = (float)data[13];  // 0.1 W
    g_miData.Pac2 = (float)data[14];  // 0.1 W


   sprintf(szbuf,"Pac 1=%.1f W, Pac 2=%.1f W \n",
            (float)data[13]/10,
            (float)data[14]/10);


    printf(szbuf);

    g_miData.Vpv1 = (float)data[15];  // 0.1 V
    g_miData.Ipv1 = (float)data[16];  // 0.01 A
    g_miData.Ppv1 = (float)data[17];  // 0.1 W
    g_miData.Vac = (float)data[19];  // 0.1 V




    sprintf(szbuf,"Vpv 1=%.1f V, Ipv 1%.1f A Ppv1=%.1f V  Vac=%.1f\n",
            (float)data[15]/10,
            (float)data[16]/10,
            (float)data[17]/10,
            (float)data[19]/10);
    printf(szbuf);

    g_miData.TIac = (float)data[20];  // 0.001 A
    g_miData.TPac = (float)data[21];  // 0.1 W
    g_miData.Fac = (float)data[22];  // 0.01 Hz
    g_miData.Vac = (float)data[19];  // 0.1 V

    sprintf(szbuf,"Iac 1=%.3f A, Pac 1%.1f W Fac=%.2f Hz  Vac=%.1f\n",
            (float)data[20]/1000,
            (float)data[21]/10,
            (float)data[23]/100);

    printf(szbuf);


    g_miData.Vpv2 = (float)data[30];  // 0.1 V
    g_miData.Ipv2 = (float)data[31];  // 0.01 A
    g_miData.Ppv2 = (float)data[32];  // 0.1 W
    g_miData.Iac1 = g_miData.Pac1/g_miData.Vac; // 0.001 A
    g_miData.Iac2 = g_miData.Pac2/g_miData.Vac;  // 0.001 A
    char  outf[2048];
    char  szparam[512];

    char curl[] = "curl -L -v -d \"%s\" http://60.251.36.232/data/PostDarfonData.php";
    //char struri[] = "http://60.251.36.232/data/PostDarfonData.php";

    sprintf(szparam,"logger_sn=00050001000000FF&internal_ip=192.168.0.8&external_ip=123.45.67.89&firmware_version=00000015.DL200&logger_mac=%s&logger_status=ACTIVE&invcount=1&inverter1_model=0001&inverter1_sn=%s&inverter1_address=3&inverter1_etotal=%.2f&inverter1_pac=%.2f&inverter1_Temperature=%.1f&inverter1_vpv=%.2f&inverter1_ipv=%.2f&inverter1_vac=%.2f&inverter1_iac=%.2f",
         g_macaddr,pInfo->szSn, g_miData.TEac,g_miData.Pac1,g_miData.temperature, g_miData.Vpv1, g_miData.Ipv1, g_miData.Vac,g_miData.Iac1);

   // char curl[] = "curl http://60.251.36.232/data/PostDarfonData.php?%s";

//printf(szparam);
//getchar();
    sprintf(outf,curl, szparam );

printf(outf);
getchar();

    system(outf);

    /*
      DebugPrint(data+20, 1, "Total Iac:");
      DebugPrint(data+21, 1, "Total Pac:");
      DebugPrint(data+23, 1, "Fac:");
      DebugPrint(data+24, 2, "Error code:");
      DebugPrint(data+26, 2, "Pre code:");
      DebugPrint(data+30, 1, "Vpv 2:");
      DebugPrint(data+31, 1, "Ipv 2:");
      DebugPrint(data+32, 1, "Ppv 2:");
      */
      /*
      char buf[80];
      char  cfv = data+13;
      char  cvac = data+19;
      float fv = (float)cfv;
      float fvac = (float)cvac;
      sprintf(buf,"Iac 1 : %.3f", fv / fvac );
      cfv = data+14;
      fv = (float)cfv;
      sprintf(buf,"Iac 2 : %.3f", fv / fvac );
      */




}

void DumpMiInfo(char *lpdata, MI_INFO *pInfo)
{
    unsigned char *data;
    char  szbuf[512];
    sprintf(szbuf,"SleveID=%d, FuncCode=%d, ByteCount=%d \n",lpdata[0], lpdata[1],lpdata[2]);
    printf(szbuf);

    data = (unsigned char *)lpdata+3;
    //unsigned short customer,model,y,m,d;

    pInfo->year = (unsigned short)data[8]<<8 | data[9];
    pInfo->month = (unsigned short)data[10]<<8 | data[11];
    pInfo->day = (unsigned short)data[12]<<8 | data[13];
    sprintf(pInfo->szSn,"%02x%02x%02x%02x%02x%02x%02x%02x",data[0], data[1],data[2],data[3],data[4], data[5],data[6],data[7] );
    sprintf(szbuf,"SN =%s ,Year=%d, Month=%d, Day=%d\n",pInfo->szSn ,pInfo->year,pInfo->month, pInfo->day);
    printf(szbuf);



   // getchar();

}


void DebugPrint(unsigned char *lpData, int size, char *lpHeader)
{
    int len,i;
    unsigned char *p,*q;
    unsigned char szbuf[2048];
    memset(szbuf, 0x00, 2048);
    p = szbuf;

    if (lpHeader && *lpHeader) {
        len = strlen(lpHeader);
        strcpy(p, lpHeader);
        p += len;
        strcat(p, " : ");
        p += 3;
    } else {
        strcpy(p, "undef : ");
        p += 8;
    }

    q = lpData;
    for (i=0; i<size; i++, q++) {
        sprintf(p, " 0x%02X", *q);
        p += 5;
    }
    strcat(szbuf, "\n");
    printf(szbuf);

}

int parse_ini_file(char * ini_name)
{
    dictionary  *   ini ;

    /* Some temporary variables to hold query results */
    int             b ;
    int             i ;
    double          d ;
    unsigned char  *   s ;

    ini = iniparser_load(ini_name);
    if (ini==NULL) {
        fprintf(stderr, "cannot parse file: %s\n", ini_name);
        return -1 ;
    }
    iniparser_dump(ini, stderr);

    g_port = iniparser_getint(ini, "device:port", 0);
    printf("Port:  %d\n", g_port);

    s = iniparser_getstring(ini, "devide:opt", "9600,8,n,1");
    strcpy(g_comopt, s);
    printf("baudrate: %s \n", g_comopt);

    // get dealy time for darfon
    g_delay1 = iniparser_getint(ini, "darfon:delay1", 1000 );
    g_delay2 = iniparser_getint(ini, "darfon:delay2", 500 );
    g_delay3 = iniparser_getint(ini, "darfon:delay3", 100 );
    strcpy(g_comopt, s);
    printf("delay: %d \n", g_delay1);

    s = iniparser_getstring(ini, "darfon:plcid", "0x00,0x05,0x00,0x01,0x00,0x00,0x00,0xFF");
    unsigned int xbuf[8];
    sscanf(s, "%x,%x,%x,%x,%x,%x,%x,%x",&xbuf[0],&xbuf[1],&xbuf[2],&xbuf[3],&xbuf[4],&xbuf[5],&xbuf[6],&xbuf[7] );
    for (i=0; i<8; i++) {
       g_plcid[i] = (unsigned char)(xbuf[i]&0x00ff);
    }


    g_fetchtime = iniparser_getint(ini, "darfon:fetchtime", 60 );  // seconds
    s = iniparser_getstring(ini, "darfon:internal_ip", "192.168.1.1");
    strcpy(g_internalIp, s);
    s = (unsigned char *)iniparser_getstring(ini, "darfon:external_ip", "168.95.1.1");
    strcpy(g_externalIp, s);




    iniparser_freedict(ini);
    return 0 ;
}


void getenvdata()
{
    struct ifaddrs *ifaddr=NULL;
    struct ifaddrs *ifa = NULL;
    int i = 0;

    if (getifaddrs(&ifaddr) == -1)
    {
         perror("getifaddrs");
    }
    else
    {
         for ( ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
         {
             if ( (ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_PACKET) )
             {
                  struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
                  if (s->sll_addr[0]|s->sll_addr[1]|s->sll_addr[2]|s->sll_addr[3]|s->sll_addr[4]|s->sll_addr[5]) {
                    sprintf(g_macaddr,"%02x:%02x:%02x:%02x:%02x:%02x",
                    s->sll_addr[0],s->sll_addr[1],s->sll_addr[2],s->sll_addr[3],s->sll_addr[4],s->sll_addr[5]);
                    break;
                  }

             }
         }
         freeifaddrs(ifaddr);
    }



    printf(g_macaddr);

}






