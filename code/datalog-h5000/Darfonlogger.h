#include "mem.h"
#include <sys/time.h>
#include "DarfonloggerGlobe.h"






//typedef unsigned char byte;
//typedef enum { false, true } bool;
//typedef enum { rError, rSuccess, rNoresponse } ModbusResponseStatus;
//typedef enum { Auto, OnlyRegister, OnlyMonitor } MIRegisterMode;
//typedef enum { offline, online, error } MIstatus;





char temptxtpath[200]="/tmp/Darfonlogger_temp.txt";
char DarfonloggerkillProcesspath[50]="/tmp/DarfonloggerkillProcess.txt";
char DarfonloggerMemSizepath[50]="/tmp/MemSize.txt";
char DarfonloggerUSBStoragePathFile[50]="/tmp/USBStoragePath.txt";


char pingTestURL1[20]="google.com";
char pingTestURL2[20]="yahoo.com";
char pingTestURL3[30]="portal.darfonsolar.com";

int pingfailcount=0;
int pingsuccesscount=0;

int pingtestcount=0;

const int maxInvertercount=200;





char* SysA_Error_COD1StringArray_MI_240_300[16]=
{
	//Bit 0:
	"Fac over safety frequency value",
	//Bit 1:
	"Fac under safety frequency value",
	//Bit 2:
	"Islanding Detection",
	//"Bit 3:
	"Vac over maximum safety limit value",
	//"Bit 4:
	"Vac under minimum safety limit value",
	//Bit 5:
	"Vac over safety trip value",
	//Bit 6:
	"Vac under safety trip value",
	//Bit 7:
	"Non AC exist",
	//Bit 8:
	"Detect GFDI",
	//Bit 9:
	"Iac over limit value",
	//Bit A:
	"Output power over limit value",
	//Bit B:
	"Vpv over maximum limit value",
	//Bit C:
	#if (ShowMoreErrorForRD)
	"Vpv under minimum limit value",//no for user
	#else
	"",
	#endif
	//"",
	//Bit D:
	"Ipv1 over maximum limit value",
	//Bit E:
	"Ipv2 over maximum limit value",
	//Bit F:
	"Ipv over maximum limit value",
};

char* SysA_Error_COD2StringArray_MI_240_300[16]=
{

	//Bit 0:
	"",//Read EEPROM Fail
	//Bit 1:
	"",//Vbus over maximum limit value
	//Bit 2:
	"",//Current command too smaller
	//Bit 3:
	"Software protection",
	//Bit 4:
	"",
	//Bit 5:
	#if (ShowMoreErrorForRD)
	"Temporal Protection too frequently",//no for user
	#else
	"",
	#endif
	//Bit 6:
	#if (ShowMoreErrorForRD)
	"Zero duty",//no for user
	#else
	"",
	#endif
	//Bit 7:
	"",
	//Bit 8:
	"",
	//Bit 9:
	"",
	//Bit A:
	"",
	//Bit B:
	"",
	//Bit C:
	"",
	//Bit D:
	"",
	//Bit E:
	"",
	//Bit F:
	""
};

char* SysA_Error_COD1StringArray_MI_320[16] =
{
            //Bit 0:
            "Fac over safety frequency value",
			//Bit 1:
            "Fac under safety frequency value",
			//Bit 2:
            "Islanding Detection",
			//Bit 3:
            "Vac over maximum safety limit value",
			//Bit 4:
            "Vac under minimum safety limit value",
			//Bit 5:
            "Vac over safety trip value",
			//Bit 6:
            "Vac under safety trip value",
			//Bit 7:
            "Non AC exist",
			//Bit 8:
            "Detect GFDI",
			//Bit 9:
            "Iac over limit value",
			//Bit A:
            "Output power over limit value",
			//Bit B:
            "Vpv over maximum limit value",
			//Bit C:
            "Vpv under minimum limit value",
			//Bit D:
            "Ipv1 over maximum limit value",
			//Bit E:
            "Ipv2 over maximum limit value",
			//Bit F:
            "Ipv over maximum limit value",
        };

 char* SysA_Error_COD2StringArray_MI_320[16] =
 {
            //Bit 0:
            "Read EEPROM Fail",
			//Bit 1:
            "Vbus over maximum limit value",
			//Bit 2:
            "Current command too smaller",
			//Bit 3:
            "Software protection. For example: abnormal zero-crosing signal",
            //Bit 4:
            "Stop by Force Coil through communication",
			//Bit 5:
            "Temperature over maximum limit",
			//Bit 6:
            "RISO Fault",
			//Bit 7:
            "Instantaneous frequency variation over than current frequency",
			//Bit 8:
            "Instantaneous frequency variation under than current frequency",
			//Bit 9:
            "Instantaneous Vac variation over than current voltage",
			//Bit A:
            "Instantaneous Vac variation under than current voltage",
			//Bit B:
            "AC Grid Abnormal Remain on the Circuit",
			//Bit C:
            "Transformer Short Detection",
			//Bit D:
            "",
			//Bit E:
            "",
			//Bit F:
            "",
        };

/*

new MonitorComInverterType("","labTemperature",,30,(decimal )0.1M,"C","",0,1),
                new MonitorComInverterType("EpvA-Total H","labEpvA-Total",,2,(decimal )100M,"KWHr","-Total",0,2),
                new MonitorComInverterType("EpvA-Total L","labEpvA-Total",,1,(decimal )0.01M,"KWHr","-Total",0,1),
                new MonitorComInverterType("Eac A-Total H","labEac A-Total",,2,(decimal )100M,"KWHr","",0,2),
                new MonitorComInverterType("Eac A-Total L","labEac A-Total",,1,(decimal )0.01M,"KWHr","",0,1),
                new MonitorComInverterType("Eac_Today","labEac_Today",,4,(decimal )0.01M,"KWHr","",0,1),
                new MonitorComInverterType("Vpv_A","labVpv_A",,1,(decimal )0.1M,"V","",0,1),
                new MonitorComInverterType("Ipv_A","labIpv_A",,1,(decimal )0.01M,"A","",0,1),
                new MonitorComInverterType("Ppv_A","labPpv_A",,1,(decimal )0.1M,"W","",0,1),
                new MonitorComInverterType("Vac_A","labVac_A",,5,(decimal )0.1M,"V","",0,1),
                new MonitorComInverterType("Iac_A","labIac_A",,1,(decimal )0.001M,"A","",0,1),
                new MonitorComInverterType("Pac_A","labPac_A",,1,(decimal )0.1M,"W","",0,1),
                new MonitorComInverterType("Fac_A","labFac_A",,1,(decimal )0.01M,"Hz","",0,1),
                //error
                new MonitorComInverterType("SysA Error_COD1","labSysA Error_COD1",,6,(byte)1,"","",0,1),
                new MonitorComInverterType("SysA Error_COD2","labSysA Error_COD2",0x0219,1,(byte)1,"","Error_COD2",0,1),
                new MonitorComInverterType("SysA Error_COD3","labSysA Error_COD3",0x021A,1,(byte)1,"","Error_COD3",0,1),
                new MonitorComInverterType("SysA Error_COD4","labSysA Error_COD4",0x021B,1,(byte)1,"","Error_COD4",0,1),
                new MonitorComInverterType("SysA Error_COD5","labSysA Error_COD5",0x021C,1,(byte)1,"","Error_COD5",0,1),
                new MonitorComInverterType("SysA Error_COD6","labSysA Error_COD6",0x021D,1,(byte)1,"","Error_COD6",0,1)

*/





#define StartAddress_Temperature 0x0200
#define StartAddress_Epv_Total 0x0207
#define StartAddress_Eac_Total 0x020A
#define StartAddress_Eac_Today 0x020E
#define StartAddress_Vpv 0x020F
#define StartAddress_Ipv 0x0210
#define StartAddress_Ppv 0x0211
#define StartAddress_Vac 0x0213
#define StartAddress_Iac 0x0214
#define StartAddress_Pac 0x0215
#define StartAddress_Fac 0x0217
#define StartAddress_Error_COD1 0x0218
#define StartAddress_Error_COD2 0x0219
#define StartAddress_Error_COD3 0x021A
#define StartAddress_Error_COD4 0x021B
#define StartAddress_Error_COD5 0x021C
#define StartAddress_Error_COD6 0x021D

//#define InverterEnergylen 16



typedef enum
{
	SysA_Error_COD1,
    SysA_Error_COD2,
    SysA_Error_COD3,
    SysA_Error_COD4,
    SysA_Error_COD5,
    SysA_Error_COD6,
    None
} ErrorCodeType;


struct       rtc_time {
      int         tm_sec;
      int         tm_min;
      int         tm_hour;
      int         tm_mday;
      int         tm_mon;
      int         tm_year;
      int         tm_wday; /* unused */
      int         tm_yday; /* unused */
      int         tm_isdst;/* unused */
  };

struct linux_rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};


# define RTC_RD_TIME	_IOR('p', 0x09, struct linux_rtc_time)
# define RTC_SET_TIME	_IOW('p', 0x0a, struct linux_rtc_time)


/*
typedef enum
{
    PLC,
    Zigbee
}CommunicationType;



typedef struct
{
    char   	name[20];
	double 	value;
	char   	UnitName[20];
	char 	csman_value[20];

	//unsigned int csidindex;

} s_EnergyData, *ps_EnergyData;





typedef struct{
	bool 			registered;
    bool 			CsmanDataUpdate;
	byte			ID[8];
	unsigned int 	Address;
	bool 			confirmed;
	bool 			error;
	char			Name[20];
	unsigned int 	FWVersion;
	time_t 			lastreceivedtime;
	time_t 			lasterroroccurtime;
	time_t 			etotaltodaydate;
	double 			etotaltodaystartvalue;
	char			lasterrorcode[20];
	char			lasterrorstring[20];
	s_EnergyData 	EnergyData[InverterEnergylen];
	//For Check Communication Status
	uint 			tryreaddatacount;
    uint 			threetimefailcount ;
    uint 			offlinefailcount ;
    uint 			checkcount ;

}s_Inverter, *ps_Inverter;


typedef struct
{
    char    			 Name[50];
    char    			 Value[50];
} s_NameValue, *ps_NameValue;







typedef s_InverterTable{
	uint	inverter_count;
	uint 	RegisterResponseFailCount;
	uint 	RegisterResponseSuccessCount;
	uint 	RegisterNoResponseCount;
	uint	RegisterMODValue;
	bool	IfConflict;
	time_t 	lastRegisterTime;
	time_t 	lastUploadToCloudtime;
	time_t 	lastWriteToCSMANROM;
	time_t 	lastWriteToCSMANRAM;
	time_t 	lastPingSuccessTime;
	time_t 	lastPingTime;
    time_t 	lastCheckMem;
	time_t 	lastRestartHttpdTime;
    time_t 	lastUpdateTodayPacCsmanTime;
	MIRegisterMode RegisterMode;
    CommunicationType CommunicationBoxType;
    uint    MINumZigbee;
	s_Inverter inverter_list[0xff];


}s_InverterTable,*ps_InverterTable ;
*/

#define TYPE_U8 "u8"
#define TYPE_U16 "u16"
#define TYPE_U32 "u32"
#define TYPE_STR "str"
#define TYPE_IPV4 "ipv4"
#define TYPE_MAC "mac"


void Start();
void StartDemo();

void InitMIDATA() ;
void Logger();
void LoggerDemo();

void RemoveAllRegister( uint count);
void StartRegisterProcess(bool ckecktimeout);
unsigned int GetFWVersion(int SlaveID);
bool GetMIFWVersion(ps_Inverter inverter);





void StopAndShowMessage( char * pMessage,int showstop);
int BootloaderResetInverterModbus();
void Parameter( int argc, char *argv[]);
void FWUpgrade( int argc, char *argv[]);
int SendHexFile( FILE * pFile, eFamily Family);
eFamily ReadIDModbus(unsigned int *version);
//unsigned int GetFWVersion();
ModbusResponseStatus CheckResponse(float waittime_s);
void InitCurrentUsedglobesetting() ;
void PrintInverterlist(char* tag,bool showall) ;
//bool DirectlyAddMItoInveterlist(byte* ID,double Eac_Total,time_t ETotalTodayDate,double ETotalTodayStartValue);
//bool DirectlyAddMItoInveterlist(byte* ID,double Eac_Total,time_t ETotalTodayDate,double ETotalTodayStartValue,time_t LastGetDatatTime);
//bool DirectlyAddMItoInveterlistOnlyID(byte* ID);

bool SaveMIEnergyData(ps_Inverter inverter,char* name, double value, unsigned int startaddress);
double GetMIEnergyDataValue(ps_Inverter inverter,char* name, unsigned int startaddress);

bool HandleErrorMessage(ps_Inverter inverter, unsigned int errrodata, ErrorCodeType type, unsigned int lasterror);
void ClearROM_MITable();
void ReadMIDataProcess();
void GetInverteValueStrByName(ps_Inverter inverter, char* name, char* valuestr);
void CreateUploadFile();

void SendAddressToOneInverter(ps_Inverter inverter,int count);

void SendAddressToInverters(int count);

double GetEnergyDataValueByName(ps_Inverter inverter,char* name);
time_t GetCurrentDatetime() ;
time_t GetCurrentDatetimeSecAgo(int sec) ;

time_t Convert_Str_to_time_t(char* timestr) ;

void Convert_time_t_to_Str(time_t time, char* timestr) ;
void UpdateInverterEToday(ps_Inverter inverter,double lastETotal);
bool GetMIData(ps_Inverter inverter, int trycount, bool ifsetoffline);
bool CheckIfMITimeout(ps_Inverter inverter, double diff);
double GetAvgPac();
double GetTotalPac();
bool pingtest();
bool pingurl();
MIRegisterMode ReadRegisterMode();
void KeepAliveHttpd();
void KeepAliveHttpd2();

void ReadMItableAndWriteToInveterlist(bool force);
int  LanchCMD(char* cmd);
void *SaveHistoryData() ;
void UpdateTodayPacCsmanTime() ;
void split(char **arr, char *str, const char *del);
void FormatSNtoName(byte ID[8], char* Name);
void AddMItoInveterlistID(unsigned char* ID);

void GetMITableZigbee();

void GetMINumZigbee();


void GetAllMiSnTableZigbee();
bool CheckIfFWUpgradingNow();
bool WriteLineToCSVFile(char* FilePath, char* Content);
void SaveEnergyDataToCSV();
bool IfTheSameDate(time_t date1, time_t date2);

double ReadEtotalStartFromFile(ps_Inverter inverter);
void CheckUSBStorageAlive(void);
void GetInverterCSVDataFilePath(ps_Inverter inverter, char* FilePath, char* DirectoryPath );
void CheckInverterPac(ps_Inverter inverter);
































