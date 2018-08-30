#ifndef GDEFINE_H_INCLUDE
#define GDEFINE_H_INCLUDE

typedef struct stSNOBJ {
    unsigned char    m_Addr;    // 1 ~ 253
    char    m_Sn[17];           // SN 16 bytes + end 0x00
    int     m_Device;           // 0x00 ~ 0x09, 0x0A ~ 0xFF
    int     m_Err;              // consecutive error times, >= 3 to run ReRegiser function
    int     m_state;            // 1 : online, 0 : offline
} SNOBJ;

typedef struct stGlobal {
    int     g_delay1;
    int     g_delay2;
    int     g_delay3;
    int     g_fetchtime;
} GLOBAL_DATA;


typedef struct stDL_DATA {
    char   g_macaddr[18];
    unsigned char  g_plcid[32];
    char   g_internalIp[16];
    char   g_externalIp[16];
    char   g_zonename[64];
    char   g_timezone[64];
} DL_DATA;

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

// Global Data
extern char  *szPort[];
extern GLOBAL_DATA  g_global;
extern DL_DATA      g_dlData;
extern MI_INFO      g_miInfo;
extern MI_DATA      g_miData;

// MI ID Information
typedef struct stMi_ID_Info {
    int Customer;
    int Model;
    int SN_Hi;
    int SN_Lo;
    int Year;
    int Month;
    int Date;
    int Device;
} MI_ID_INFO;

// MI Power Information
typedef struct stMi_Power_Info {
    int Temperature;
    int Date;
    int Hour;
    int Minute;
    int Ch1_EacH;
    int Ch1_EacL;
    int Ch2_EacH;
    int Ch2_EacL;
    int Total_EacH;
    int Total_EacL;
    int Ch1_Pac;
    int Ch2_Pac;
    int Ch1_Vpv;
    int Ch1_Ipv;
    int Ch1_Ppv;
    int Vac;
    int Total_Iac;
    int Total_Pac;
    int Fac;
    int Error_Code1;
    int Error_Code2;
    int Pre1_Code1;
    int Pre1_Code2;
    int Pre2_Code1;
    int Pre2_Code2;
    int Ch2_Vpv;
    int Ch2_Ipv;
    int Ch2_Ppv;
}MI_POWER_INFO;

typedef struct stHybrid_ID_Data {
    int Grid_Voltage;
    int Model;
    int SN_Hi;
    int SN_Lo;
    int Year;
    int Month;
    int Date;
    int Inverter_Ver;
    int DD_Ver;
    int EEPROM_Ver;
    int Flags;
}HB_ID_DATA;

typedef struct stHybrid_ID_Flags {
    char B0_Rule21;
    char B1_PVParallel;
    char B2_PVOffGrid;
    char B3_Heco1;
    char B4_Heco2;
    char B5_ACCoupling;
    char B6_FreControl;
}HB_ID_FLAGS;

typedef struct stHybrid_RTC_Data {
    int Second;
    int Minute;
    int Hour;
    int Date;
    int Month;
    int Year;
}HB_RTC_DATA;

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
    int Volt_VAr;
    int StartFrequency;
    int EndFrequency;
    int FeedinPower;
}HB_RS_INFO;

typedef struct stHybrid_Remote_Realtime_Setting_Info {
    int ChargeSetting;
    int ChargePower;
    int DischargePower;
    int RampRatePercentage;
    int DegreeLeadLag;
}HB_RRS_INFO;

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
    int Load_Power; //Pac_A;
    int Grid_Voltage; //VGrid_A;
    int Grid_Current; //IGrid_A;
    int Grid_Power; //PGrid_A;
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
    int OnGrid_Mode;
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
}HB_RT_INFO;

typedef struct stHybrid_PV_Inv_Error_COD1 {
    char B0_Fac_HL;
    char B1_PV_Low;
    char B2_Islanding;
    char B3_Vac_H;
    char B4_Vac_L;
    char B5_Fac_H;
    char B6_Fac_L;
    char B7_Fac_LL;
    char B8_Vac_OCP;
    char B9_Vac_HL;
    char B10_Vac_LL;
    char B11_GFDI;
    char B12_Iac_H;
    char B13_Ipv_H;
    char B14_ADCINT_OVF;
    char B15_Vbus_H;
}HB_PVINV_ERR_COD1;

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

typedef struct stHybrid_DD_Error_COD {
    char B0_Vbat_H;
    char B1_Vbat_L;
    char B2_Vbus_H;
    char B3_Vbus_L;
    char B4_Ibus_H;
    char B5_Ibat_H;
    char B6_Charger_T;
    char B7_Code;
    char B8_VBL;
    char B9_INV_Fault;
    char B10_GND_Fault;
    char B11_No_bat;
    char B12_BMS_Comute_Fault;
    char B13_BMS_Over_Current;
    char B14_Restart;
    char B15_Bat_Setting_Fault;
}HB_DD_ERR_COD;

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
    char B16_CommStation;
    char B17_Residential;
    char B18_CommStationNoGrid;
    char B19_ResidentialNoGrid;
    char B20_PowerShiftApplication;
    char B21_SettingOK;
    char B22_24_BatType;
    char B25_26_MultiINV;
    char B27_LoadCharge;
    char B28_LoadDischarge;
    char B29_30_LeadLag;
}HB_ICON_INFO;

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
}HB_BMS_INFO;

typedef struct stDL_CMD {
    char    m_Sn[17];
    int     m_addr;  // start address
    int     m_count; // number of data
    int     m_data[16]; // for now set size 16, because data range is 0xX0 ~ 0xXF
}DL_CMD;

typedef struct stDL_Config {
    char    m_sms_server[128];
    int     m_sms_port;
    char    m_update_server[128];
    int     m_update_port;
    int     m_sample_time;
    int     m_delay_time; // us, 1000000 us = 1 s
    int     m_port;
    int     m_baud;
    int     m_data_bits;
    char    m_parity[8];
    int     m_stop_bits;
}DL_CONFIG;

typedef struct stDL_Path {
    char    m_root_path[128];
    char    m_xml_path[128];
    char    m_log_path[128];
    char    m_errlog_path[128];
    char    m_bms_path[128];
    char    m_syslog_path[128];
}DL_PATH;
#endif // GDEFINE_H_INCLUDE
