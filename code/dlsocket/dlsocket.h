// define
#define VERSION     "1.0.0"
#define LOG_PATH    "/mnt/XML/LOG"
#define ERRLOG_PATH "/mnt/XML/ERRLOG"
#define ENV_PATH    "/mnt/XML/ENV"
#define SYSLOG_PATH "/mnt/SYSLOG"
#define MODEL_PATH  "/usr/home/ModelList"

#define OFFLINE_SECOND_HB 180
#define QUERY_SIZE 4096

#define HYBRID_LIST_PATH        "/tmp/Hybrid_List"
#define HYBRID_TMP_DATA_PATH    "/tmp/Hybrid_Tmp_Data"

// for extern
#define bool int
#define true 1
#define false 0
#define byte unsigned char




// extern part
extern int  MyModbusDrvInit(char *port, int baud, int data_bits, char parity, int stop_bits);
extern int  ModbusDrvDeinit(int fd);
extern void MakeReadDataCRC(unsigned char *,int );
extern void MClearRX();
extern void MStartTX(int fd);
extern unsigned char *GetRespond(int fd, int iSize, int delay);
//extern void RemoveRegisterQuery(int fd, unsigned int byAddr);
extern void CleanRespond();
extern void initenv(char *init_name);

extern unsigned int     txsize;
extern unsigned char    waitAddr, waitFCode;

extern bool have_respond;
extern unsigned char txbuffer[1544];//MODBUS_TX_BUFFER_SIZE
extern unsigned char respond_buff[4096];
extern int MySyncOffLineQuery(int fd, byte addr, byte MOD, byte buf[], int buf_size);
extern int MyOffLineQuery(int fd, byte addr, byte buf[], int buf_size);
extern int MyAssignAddress(int fd, unsigned char *ID, unsigned char Addr);
extern void RemoveRegisterQuery(int fd, byte byAddr);
extern bool CheckCRC(unsigned char *,int );
// extern end




// struct
typedef struct device_list {
    unsigned char   m_Addr;         // 1 ~ 253
    char            m_Sn[17];       // SN 16 bytes + end 0x00
    int             m_Device;       // 0x00 ~ 0x09 MI, 0x0A ~ 0xFF Hybrid
    int             m_Err;          // consecutive error times, >= 3 to run ReRegiser function
    int             m_state;        // 1 : online, 0 : offline
    //int       m_FWver;            // mi fw ver
    time_t          m_ok_time;      // last get data ok time
} DEV_LIST;

// ID data : 0x01 ~ 0x0F
typedef struct stHybrid_ID_Data {
    int Grid_Voltage;
    int Model;
    int HW_Ver;
    int SN_Hi;
    int SN_Lo;
    int Year;
    int Month;
    int Date;
    int Inverter_Ver;
    int DD_Ver;
    int EEPROM_Ver;
    int Display_Ver;
    int Flags1;
    int Flags2;
}HB_ID_DATA;

// system flags1 : 0x0D
typedef struct stHybrid_ID_Flags1 {
    char B0B1_External_Sensor;
}HB_ID_FLAGS1;

// system flags2 : 0x0E
typedef struct stHybrid_ID_Flags2 {
    char B0_Rule21;
    char B1_PVParallel;
    char B2_PVOffGrid;
    char B3_Heco1;
    char B4_Heco2;
    char B5_ACCoupling;
    char B6_FreControl;
    char B7_ArcDetection;
    char B8_PREPA;
    char B9_Self_Supply;
    char B10_Charge_only_from_PV;
    char B11_Dominion;
}HB_ID_FLAGS2;

// RTC data : 0x40 ~ 0x4F
typedef struct stHybrid_RTC_Data {
    int Second;
    int Minute;
    int Hour;
    int Date;
    int Month;
    int Year;
}HB_RTC_DATA;

// Remote setting info : 0x90 ~ 0x9F
typedef struct stHybrid_Remote_Setting_Info {
    int Mode;
    int StarHour;
    int StarMin;
    int EndHour;
    int EndMin;
    int MultiModuleSetting;
    int BatteryType;
    int BatteryCurrent;
    int BatteryShutdownVoltage;
    int BatteryFloatingVoltage;
    int BatteryReservePercentage;
    int PeakShavingPower; // Volt_VAr
    int StartFrequency;
    int EndFrequency;
    int FeedinPower;
}HB_RS_INFO;

// Remote real-time setting Info : 0xA0 ~ 0xAF
typedef struct stHybrid_Remote_Realtime_Setting_Info {
    int ChargeSetting;
    int ChargePower;
    int DischargePower;
    int RampRatePercentage;
    int DegreeLeadLag;
    int Volt_VAr; // PeakShavingPower
    int AC_Coupling_Power;
}HB_RRS_INFO;

// Real time info : 0xB0 ~ 0xFF
typedef struct stHybrid_Realtime_Info {
    int Inv_Temp;
    int PV1_Temp;
    int PV2_Temp;
    int DD_Temp;
    int PV1_Voltage; //Vpv_A;
    int PV1_Current; //Ipv_A;
    int PV1_Power; //Ppv_A;
    int PV2_Voltage; //Vpv_B;
    int PV2_Current; //Ipv_B;
    int PV2_Power; //Ppv_B;
    int Load_Voltage; //Vac_A;
    int Load_Current; //Iac_A;
    short Load_Power; //Pac_A; //+-
    int Grid_Voltage; //VGrid_A;
    int Grid_Current; //IGrid_A;
    short Grid_Power; //PGrid_A; //+-
    int Battery_Voltage; //VBattery;
    int Battery_Current; //IBattery;
    int Bus_Voltage; //Vbus;
    int Bus_Current; //Ibus;
    int PV_Total_Power; //Ppv_Total;
    int PV_Today_EnergyH; //Ppv_TodayH;
    int PV_Today_EnergyL; //Ppv_TodayL;
    int PV_Total_EnergyH; //Ppv_TotalH;
    int PV_Total_EnergyL; //Ppv_TotalL;
    int Bat_Total_EnergyH; //Pbat_TotalH;
    int Bat_Total_EnergyL; //Pbat_TotalL;
    int Load_Total_EnergyH; //Pload_TotalH;
    int Load_Total_EnergyL; //Pload_TotalL;
    int GridFeed_TotalH;
    int GridFeed_TotalL;
    int GridCharge_TotalH;
    int GridCharge_TotalL;
    int External_Power; // OnGrid_Mode
    int Sys_State;
    int PV_Inv_Error_COD1_Record;
    int PV_Inv_Error_COD2_Record;
    int DD_Error_COD_Record;
    int PV_Inv_Error_COD1;
    int PV_Inv_Error_COD2;
    int DD_Error_COD;
    int Hybrid_IconL;
    int Hybrid_IconH;
    int Error_Code;
    int Battery_SOC;
    int Invert_Frequency;
    int Grid_Frequency;
    short PBat; //+-
    int PV_Inv_Error_COD3_Record;
    int DD_Error_COD2_Record;
}HB_RT_INFO;

// PV-Inverter Error Code1 : 0xD3
typedef struct stHybrid_PV_Inv_Error_COD1 {
    char B0_Fac_HL;
    char B1_CanBus_Fault;
    char B2_Islanding;
    char B3_Vac_H;
    char B4_Vac_L;
    char B5_Fac_H;
    char B6_Fac_L;
    char B7_Fac_LL;
    char B8_Vac_OCP;
    char B9_Vac_HL;
    char B10_Vac_LL;
    char B11_OPP;
    char B12_Iac_H;
    char B13_Ipv_H;
    char B14_ADCINT_OVF;
    char B15_Vbus_H;
}HB_PVINV_ERR_COD1;

// PV-Inverter Error Code2 : 0xD4
typedef struct stHybrid_PV_Inv_Error_COD2 {
    char B0_Arc;
    char B1_Vac_Relay_Fault;
    char B2_Ipv1_Short;
    char B3_Ipv2_Short;
    char B4_Vac_Short;
    char B5_CT_Fault;
    char B6_PV_Over_Power;
    char B7_NO_GRID;
    char B8_PV_Input_High;
    char B9_INV_Overload;
    char B10_RCMU_30;
    char B11_RCMU_60;
    char B12_RCMU_150;
    char B13_RCMU_300;
    char B14_RCMU_Test_Fault;
    char B15_Vac_LM;
}HB_PVINV_ERR_COD2;

// PV-Inverter Error Code3 : 0xF0
typedef struct stHybrid_PV_Inv_Error_COD3 {
    char B0_External_PV_OPP;
}HB_PVINV_ERR_COD3;

// DD Error Code1 : 0xD5
typedef struct stHybrid_DD_Error_COD {
    char B0_Vbat_H;
    char B1_Vbat_L;
    char B2_Vbus_H;
    char B3_Vbus_L;
    char B4_Ibus_H;
    char B5_Ibat_H;
    char B6_Charger_T;
    char B7_Code;
    char B8_Vbat_Drop;
    char B9_INV_Fault;
    char B10_GND_Fault;
    char B11_No_bat;
    char B12_BMS_Comute_Fault;
    char B13_BMS_Over_Current;
    char B14_Restart;
    char B15_Bat_Setting_Fault;
}HB_DD_ERR_COD;

// DD Error Code2 : 0xF1
typedef struct stHybrid_DD_Error_COD2 {
    char B0_EEProm_Fault;
    char B1_Communi_Fault;
    char B2_OT_Fault;
    char B3_Fan_Fault;
    char B4_Low_Battery;
}HB_DD_ERR_COD2;

// Hybrid_Icon Info : 0xD9
typedef struct stHybrid_Icon_Info {
    char B0_PV;
    char B1_MPPT;
    char B2_Battery;
    char B3_Inverter;
    char B4_Grid;
    char B5_Load;
    char B6_OverLoad;
    char B7_Error;
    char B8_Warning;
    char B9_PC;
    char B10_BatCharge;
    char B11_BatDischarge;
    char B12_FeedingGrid;
    char B13_PFCMode;
    char B14_GridCharge;
    char B15_GridDischarge;
    char B16_18_INVFlag;
    char B19_GeneratorMode;
    char B20_Master_Slave;
    char B21_SettingOK;
    char B22_24_BatType;
    char B25_26_MultiINV;
    char B27_LoadCharge;
    char B28_LoadDischarge;
    char B29_30_LeadLag;
}HB_ICON_INFO;

// BMS info
typedef struct stHybrid_BMS_Info {
    int Voltage;
    int Current;
    int SOC;
    int MaxTemperature;
    int CycleCount;
    int Status;
    int Error;
    int Number;
    int BMS_Info;
    int BMS_Max_Cell;
    int BMS_Min_Cell;
    int BMS_BaudRate;
}HB_BMS_INFO;




// function
// system parameter
void get_config();
int open_com_port();
void stop_process();
void start_process();
// register
int run_register();
int get_device(int);
int AllocateProcess(unsigned char *, int);
int write_Hybrid_list();
// get data
int get_current_data();
int re_register(int);
// ID data 0x00
int get_id(int);
void dump_id(unsigned char *);
//int set_id(int);
void parser_id_flags1(int);
void parser_id_flags2(int);
// RTC data 0x40
int get_RTC(int);
void dump_RTC(unsigned char *);
//int set_RTC(int);
// Remote Setting 0x90
int get_RS(int);
void dump_RS(unsigned char *);
//int set_RS(int);
// Remote Real-time Setting 0xA0
int get_RRS(int);
void dump_RRS(unsigned char *);
//int set_RRS(int);
// Real Time Info 0xB0
int get_RT(int);
void dump_RT(unsigned char *);
void dump_RT2(unsigned char *);
void parser_PVInvErrCOD1(int);
void parser_PVInvErrCOD2(int);
void parser_PVInvErrCOD3(int);
void parser_DDErrCOD(int);
void parser_DDErrCOD2(int);
void parser_IconInfo(int, int);
// BMS info 0x200
int get_BMS(int);
void dump_BMS(unsigned char *);

//int find_list(char *);








int check_cmd(unsigned char *);
int send_file(int , char *, int);




#if 0
// base64 start
static const char encoding_table[] = {
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
'w', 'x', 'y', 'z', '0', '1', '2', '3',
'4', '5', '6', '7', '8', '9', '+', '/'
};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

// input XML file which want to transform, write base64 code string to file which file descriptor is fd, return encode data length, if ERROR return -1
int base64_encode(char* filename, FILE* fd);

// input data string, input data length, output data length, return decode data string
unsigned char* base64_decode(const char* data, int inlen, int *outlen);

void build_decoding_table();
void base64_cleanup();

void build_decoding_table() {

    int i = 0;

    decoding_table = (char*) malloc(256);
    memset(decoding_table, 0x00, 256);

    for (i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
    //for (i = 0; i < 256; i++)
    //    printf("decoding_table[%d] = %d\n", i, decoding_table[i]);
}

void base64_cleanup() {
    if ( decoding_table )
        free(decoding_table);
}

int base64_encode(char *filename, FILE *fd)
{
    FILE *pfd = NULL;
    char inbuf[3072] = {0};
    char outbuf[4096] = {0};
    int len = 0, i = 0, j = 0, outlen = 0;

    printf("======== base64_encode start ========\n");
    pfd = fopen(filename, "rb");
    if ( pfd == NULL ) {
        printf("#### base64_encode Open %s Fail ####\n", filename);
        return -1;
    }

    while (1) {
        len = fread (inbuf, 1, 3072, pfd);
        if ( len > 0 ) {
            outlen = 4 * ((len + 2) / 3);
            printf("len = %d, outlen = %d\n", len, outlen);
            //printf("inbuf = \n%s\n", inbuf);

            for (i = 0, j = 0; i < len; ) {
                unsigned int octet_a = i < len ? (unsigned char)inbuf[i++] : 0;
                unsigned int octet_b = i < len ? (unsigned char)inbuf[i++] : 0;
                unsigned int octet_c = i < len ? (unsigned char)inbuf[i++] : 0;

                unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

                outbuf[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
                outbuf[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
                outbuf[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
                outbuf[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
            }

            for (i = 0; i < mod_table[len % 3]; i++)
                outbuf[outlen - 1 - i] = '=';

            fwrite(outbuf , sizeof(char), outlen, fd);
            //printf("base64_encode : \n%s\n", outbuf);

        } else {
            break;
        }

        if (feof(pfd))
            break;
    }
    fclose(pfd);

    printf("========= base64_encode end =========\n");
    return outlen;
}

unsigned char* base64_decode(const char* data, int inlen, int *outlen)
{
    unsigned char *decoded_data = NULL;
    int i = 0, j = 0;

    printf("======== base64_decode start ========\n");
    if ( decoding_table == NULL )
        build_decoding_table();

    if ( inlen%4 != 0 ){
        *outlen = 0;
        printf("Input data length error!\n");
        return NULL;
    }

    *outlen = inlen/4 * 3;
    if ( data[inlen - 1] == '=' )
        (*outlen)--;
    if ( data[inlen - 2] == '=' )
        (*outlen)--;

    decoded_data = (unsigned char*)malloc(*outlen + 1);
    memset(decoded_data, 0x00, *outlen);
    if ( decoded_data == NULL )
        return NULL;

    for ( i = 0, j = 0; i < inlen; ) {
        unsigned int sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
        unsigned int sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
        unsigned int sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
        unsigned int sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];

        unsigned int triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if (j < *outlen)
            decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *outlen)
            decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *outlen)
            decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }
    decoded_data[*outlen] = 0;
    //printf("base64_decode : \n%s\n", decoded_data);

    printf("========= base64_decode end =========\n");
    return decoded_data;
}
// base64 end
#endif
