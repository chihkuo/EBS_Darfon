//#define SUPPORT_ZIGBEE

//#define MI_ReadDebugData

#ifndef MODBUS_DEBUG
#define MODBUS_DEBUG				3
#endif




typedef enum { false, true } bool;
typedef enum { rError, rSuccess, rNoresponse } ModbusResponseStatus;
typedef enum { Auto, OnlyMonitor, OnlyRegister } MIRegisterMode;
typedef enum { offline, online, error } MIstatus;
typedef unsigned char byte;
typedef unsigned int uint;

typedef struct
{
    char   	name[20];
	double 	value;
	char   	UnitName[20];
	char 	csman_value[20];

	//unsigned int csidindex;

} s_EnergyData, *ps_EnergyData;




#ifdef MI_ReadDebugData

#define InverterEnergylen 50



//#define TitleCSVDataNum 45
//#define TitleCSVErrorNum 3

//extern char CsvDataTitle[TitleCSVDataNum][20];
//extern char CsvErrorTitle[TitleCSVErrorNum][20];
#else

#define InverterEnergylen 16



//#define TitleCSVDataNum 11
//#define TitleCSVErrorNum 3

//extern char CsvDataTitle[TitleCSVDataNum][20];// = {"Time","Temperature","Eac_Today","Vpv","Ipv","Ppv","Vac","Iac","Pac","Fac","Eac_Total"};
//extern char CsvErrorTitle[TitleCSVErrorNum][20];// = {"Time","Error_Code","Error_Message"};
#endif








typedef struct{
	bool 			registered;
    bool 			CsmanDataUpdate;
	byte			ID[8];
	unsigned int 	Address;
	bool 			confirmed;
	bool 			error;
	char			Name[20];
	unsigned int 	FWVersion;
	time_t 			lastPacChecktime;
	time_t 			lastReadEtotalFromFiletime;
	time_t 			lastreceivedtime;
    time_t 			lastreceivedEnergytime;
	time_t 			lasterroroccurtime;
	time_t 			etotaltodaydate;
	double 			etotaltodaystartvalue;
	char			lasterrorcode[20];
	char			lasterrorstring[100];
	s_EnergyData 	EnergyData[InverterEnergylen];
	//For Check Communication Status
	uint 			tryreaddatacount;
    uint 			threetimefailcount ;
    uint 			offlinefailcount ;
    uint 			CommunicationCheckcount ;
	uint 			PacCheckFailcount ;

}s_Inverter, *ps_Inverter;


typedef enum
{
    PLC,
    Zigbee
}CommunicationType;


typedef struct  s_InverterTable{
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
    time_t 	lastCheckAutoRebootTime;
    time_t 	lastCheckMem;
	time_t 	lastRestartHttpdTime;
    time_t 	lastRestartHttpdTime2;
    time_t 	lastUpdateTodayPacCsmanTime;
    time_t 	lastSaveCSVTime;
	MIRegisterMode RegisterMode;
    CommunicationType CommunicationBoxType;
    uint    MINumZigbee;
    double  TotalETotal;
    double  TotalEToday;
    double  TotalPac;
	s_Inverter inverter_list[0xff];


}s_InverterTable,*ps_InverterTable ;

enum
{
	DFAPP_SYSTEM_FN_CLEAR_MI_SN_TABLE = 0,
    DFAPP_SYSTEM_FN_INSERT_ONE_MI_SN,
    DFAPP_SYSTEM_FN_INSERT_MULTI_MI_SN,
    DFAPP_SYSTEM_FN_GET_MI_NUM,
    DFAPP_SYSTEM_FN_READ_ALL_MI_SN_TABLE,
    DFAPP_SYSTEM_FN_READ_ONE_MI_SN_TABLE,
    DFAPP_SYSTEM_FN_SEND_TO_UNIQ_MI
};
void AddMItoInveterlistID(unsigned char* pID);
void WaitZigbeeCMD(float waittime_s);


