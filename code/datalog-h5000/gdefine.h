#ifndef GDEFINE_H_INCLUDE
#define GDEFINE_H_INCLUDE

typedef struct stSNOBJ {
    unsigned char    m_Addr;    // 1 ~ 253
    char    m_Sn[17];           // SN 16 bytes + end 0x00
    int     m_Device;           // 0x00 ~ 0x09 MI, 0x0A ~ 0xFF Hybrid
    int     m_Model;            // 1~10 table 1, 16~17 table 2
    int     m_Err;              // consecutive error times, >= 3 to run ReRegiser function
    int     m_state;            // 1 : online, 0 : offline
    int     m_FWver;            // xxxx
    time_t  m_ok_time;          // last getdata ok time
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
    int FWVER;
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

typedef struct stHybrid_ID_Flags1 {
    char B0B1_External_Sensor;
}HB_ID_FLAGS1;

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
    char B12_SunSpec;
    char B13_MA;
    char B14_ZeroExport;
}HB_ID_FLAGS2;

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
    int PeakShavingPower; // Volt_VAr
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
    int Volt_VAr; // PeakShavingPower
    int AC_Coupling_Power;
    int SunSpec_Write_All;
    int Remote_Control;
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
    short External_Power; // OnGrid_Mode //+-
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

typedef struct stHybrid_PV_Inv_Error_COD3 {
    char B0_External_PV_OPP;
    char B1_Model123_Reconnected_Delay;
    char B2_Peak_Shaving_Over_Power;
}HB_PVINV_ERR_COD3;

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

typedef struct stHybrid_DD_Error_COD2 {
    char B0_EEProm_Fault;
    char B1_Communi_Fault;
    char B2_OT_Fault;
    char B3_Fan_Fault;
    char B4_Low_Battery;
    char B5_Relay_state;
    char B6_Off_Grid_Operation;
    char B7_InvEnable_flag;
    char B8_Bypass_flag;
    char B9_DD_en;
    char B10_PVEnable_flag;
}HB_DD_ERR_COD2;

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

typedef struct stHybrid2_ID_Data {
// 0x0001
    int Grid_Voltage;
    int Model; // move High byte : hw ver to 0x000C
    int SN_Hi;
    int SN_Lo;
    int Year;
    int Month;
    int Date;
    int Inverter_Ver;
    int DD_Ver;
    int Parameter_Ver; // rename, ori name EEPROM_Ver
    int Display_Ver;
    int HW_Ver; // 0x0002 High byte to 0x000C
    int Safety_Control; // 0x000D, ori Flags1
    int DL_Ver; // add 0x000E
}HB2_ID_DATA;

typedef struct stHybrid2_ID_Flags {
    // char B0_Rule21; remove
    char B0_SunSpec;
    char B1_Heco1;
    char B2_Heco2;
    char B3_PREPA;
    char B4_Dominion;
    char B5_MA;
}HB2_ID_FLAGS;

typedef struct stHybrid2_Remote_Setting_Info {
// 0x00B0
    int Mode; // ori 0x090
    int BatteryPolic; // rename, ori 0x00A0 Charge/Discharge setting
    int MultiModuleSetting; // ori 0x095
    int BatteryType; // ori 0x096
    int ChargePower; // ori 0x00A1
    int DischargePower; //ori 0x00A2
    int FeedinPower; // ori 0x009E
    int BatteryPowerRating; // rename, ori 0x00A6 AC Coupling Power
    int MaxBatteryChargingCurrent; //rename, ori 0x0097 BatteryCurrent
    int BatteryShutdownVoltage; // ori 0x0098
    int BatteryAbsorptionChargingVoltage; // rename, ori 0x0099 BatteryFloatingVoltage
    int BatteryReservePercentage; // ori 0x009A
    int GridTiedMaxSOC; // new
    //0x00BD move to 0x00E6 //int PeakShavingPower; // Volt_VAr ori 0x009B
    int StartFrequency; // ori 0x009C
    int EndFrequency; // ori 0x009D
// 0x00C0
    int RampRatePercentage; // ori 0x00A3
    int DegreeLeadLag; // ori 0x00A4
    int PowerFactorControl; // rename, ori 0x00A5 Volt_VAr
    int TMP_C3;
    int SpecifiedModbusSlaveID; // new, Not post!
    int FunctionControl1; // 0x00C5, new
    int FunctionControl2; // new, Not post!
    int SunSpecWritable; // rename, ori 0x00A7 SunSpec_Write_All
    int SoftwareControl; // 0x00C8, rename, ori 0x00A8 Remote_Control
    // ... empty to 0x00CF
// 0x00D0 to 0x00E5, new, Not post
    int TOU_Season1_Operation_Mode;
    int TOU_OtherSeason_Operation_Mode;
    int TOU_Season1_Start_Month;
    int TOU_Season1_Start_Day;
    int TOU_Season1_End_Month;
    int TOU_Season1_End_Day;
    int Peak_Period_Start_Hour1_of_Season1;
    int Peak_Period_Start_Minute1_of_Season1;
    int Peak_Period_End_Hour1_of_Season1;
    int Peak_Period_End_Minute1_of_Season1;
    int Peak_Period_Start_Hour2_of_Season1;
    int Peak_Period_Start_Minute2_of_Season1;
    int Peak_Period_End_Hour2_of_Season1;
    int Peak_Period_End_Minute2_of_Season1;
    int Peak_Period_Start_Hour1_of_OtherSeason;
    int Peak_Period_Start_Minute1_of_OtherSeason;
// 0x00E0
    int Peak_Period_End_Hour1_of_OtherSeason;
    int Peak_Period_End_Minute1_of_OtherSeason;
    int Peak_Period_Start_Hour2_of_OtherSeason;
    int Peak_Period_Start_Minute2_of_OtherSeason;
    int Peak_Period_End_Hour2_of_OtherSeason;
    int Peak_Period_End_Minute2_of_OtherSeason;
    int PeakShavingPower; // ori 0x00BD // Volt_VAr ori 0x009B
}HB2_RS_INFO;

typedef struct stHybrid2_RS_Function_Flags {
    char B0_PV_12Parallel;
    char B1_PV_Off_Grid;
    char B2_AC_Coupling;
    char B3_Frequency_Control;
    char B4_Arc_detection;
    char B8_9_External_Sensor;
    char B10_Generator_charge_battery;
    char B11_Battery_Calibration_Enable;
    char B12_PV_34Parallel;
}HB2_RS_F_FLAGS;

typedef struct stHybrid2_RS_Software_Flags {
    char B0_System_Reset;
    char B1_Off_Grid_Enable;
    char B2_Enable_Disable;
}HB2_RS_S_FLAGS;

typedef struct stHybrid2_Realtime_Info {
// 0x0100
    int Inv_Temp;
    int Charger_Temp; // rename, ori 0x00B1 PV1_Temp;
    int Environment_Temp; // rename, ori 0x00B2 PV2_Temp;
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
// 0x0110
    int Battery_Voltage; //VBattery;
    int Battery_Current; //IBattery;
    short Battery_Power; // rename, ori 0x00DF PBat
    int Bus_Voltage; //Vbus;
    int Bus_Current; //Ibus;
    int PV3_Voltage; // new, Vpv_C?;
    int PV3_Current; // new, Ipv_C?;
    int PV3_Power; // new, Ppv_C?;
    int PV4_Voltage; // new, Vpv_D?;
    int PV4_Current; // new, Ipv_D?;
    int PV4_Power; // new, Ppv_D?;
    int PV_Total_Power; //Ppv_Total;
    int Battery_SOC; // ori 0x00DC
    // 0x011D empty
    int Sys_State; // ori 0x00D2
    int Module_Status; // 0x011F, new, Not post!
// 0x0120
    int Function_Status; // 0x0120, new, Not post!
    int PV_Inv_Error_COD1_Record; // 0x0121, ori 0x00D3
    int PV_Inv_Error_COD2_Record; // 0x0122, ori 0x00D4
    int PV_Inv_Error_COD3_Record; // 0x0123, ori 0x00E0
    int DD_Error_COD1_Record; // 0x0124, ori 0x00D5
    int DD_Error_COD2_Record; // 0x0125, ori 0x00E1
    //int PV_Inv_Error_COD1; // remove
    int PV_Inv_Error_COD2; // ori 0x00D7, Not post!
    int DD_Error_COD; // ori 0x00D8, Not post!
    int Error_Code; // ori 0x00DB
    int Invert_Frequency; // ori 0x00DD
    int Grid_Frequency; // ori 0x00DE
    int Total_Load_Power; // new, Not post!
    int Total_Load_Current; // new, Not post!
    short CT_Power; // rename, 0ri 0x00D1 External_Power; // OnGrid_Mode //+-
    int CT_Current; // new, Not post!
}HB2_RT_INFO;

typedef struct stHybrid2_RT_Module_Flags {
    char B0_Relay_status;
    char B1_Software_offgrid_flag;
    char B2_InvEnable_flag;
    char B3_Bypass_flag;
    char B4_DD_en;
    char B5_PVEnable_flag;
    char B6_Generator_state;
}HB2_RT_M_FLAGS;

typedef struct stHybrid2_RT_Function_Flags {
    char B0_ADC;
    char B1_Buzz_flag;
    char B2_Bat_Protect;
    char B3_Bat_reserve_flag;
    char B4_Froce_Charge_flag;
    char B5_Grid_Char_En;
    char B6_Grid_flag;
    char B7_DD_en_Protect;
    char B8_buck_Dis;
    char B9_boots_dis;
    char B10_Info_ready;
    char B11_Inverter_Delay;
}HB2_RT_F_FLAGS;

typedef struct stHybrid2_PV_Inv_Error_COD1 {
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
}HB2_PVINV_ERR_COD1;

typedef struct stHybrid2_PV_Inv_Error_COD2 {
    char B0_Arc;
    char B1_Para_Check;
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
}HB2_PVINV_ERR_COD2;

typedef struct stHybrid2_PV_Inv_Error_COD3 {
    char B0_External_PV_OPP;
    char B1_Model123_Reconnected_Delay;
    char B2_Peak_Shaving_Over_Power;
    char B3_CLA_Execute_Time_Over;
    char B4_Fac_Sudden;               // 1.42 add
    char B5_Inverter_Idc_Fault;       // 1.42 add
    char B6_Zero_Export_Fault;        // 1.42 add
    char B7_Grid_Charge_To_Bat_Fault; // 1.42 add
    char B8_Grid_Ok_For_DD;           // 1.42 add
}HB2_PVINV_ERR_COD3;

typedef struct stHybrid2_DD_Error_COD1 {
    char B0_Vbat_H;
    char B1_Vbat_L;
    char B2_Vbus_H;
    char B3_Vbus_L;
    char B4_Ibus_H;
    char B5_Ibat_H;
    // b6 remove
    char B7_Code;
    char B8_Vbat_Drop;
    char B9_Model_FW_Define_Err;
    // b10 remove
    char B11_No_bat;
    char B12_BMS_Comute_Fault;
    // b13 remove
    char B14_Vbus_High_VBat; // 0ri B14_Restart;
    // b15 remove
}HB2_DD_ERR_COD1;

typedef struct stHybrid2_DD_Error_COD2 {
    char B0_EEProm_Fault;
    char B1_Communi_Fault;
    char B2_OT_Fault;
    char B3_Fan_Fault;
    char B4_Low_Battery;
    char B5_PV3_S; // ori B5_Relay_state;
    char B6_PV4_S; // ori B6_Off_Grid_Operation;
    char B7_PV_Over_Power; // ori B7_InvEnable_flag;
    char B8_PV_Input_High; // ori B8_Bypass_flag;
    char B9_Restart; // ori B9_DD_en;
    char B10_GND_Fault; // ori B10_PVEnable_flag;
    char B11_OT_Alarm; // new
    char B12_Bat_Wake_Up_Fault; // new
    char B13_Vbat_Inconsistent; // new
    char B14_Bat_FW_Update;
}HB2_DD_ERR_COD2;

typedef struct stHybrid2_Cumulative_Energy_Value {
// 0x0160
    int Total_Life_TimeH; // new
    int Total_Life_TimeL; // new
    int PV_Total_EnergyH; // ori 0x00C7 //Ppv_TotalH;
    int PV_Total_EnergyL; // ori 0x00C8 //Ppv_TotalL;
    int Bat_Charge_Total_EnergyH; // ori 0x00C9 Bat_Total_EnergyH //Pbat_TotalH;
    int Bat_Charge_Total_EnergyL; // ori 0x00CA Bat_Total_EnergyL //Pbat_TotalL;
    int Bat_Discharge_Total_EnergyH; // new
    int Bat_Discharge_Total_EnergyL; // new
    int Load_Total_EnergyH; // ori 0x00CB //Pload_TotalH;
    int Load_Total_EnergyL; // ori 0x00CC //Pload_TotalL;
    int Negative_Load_Total_EnergyH; // new
    int Negative_Load_Total_EnergyL; // new
    int GridFeed_TotalH; // ori 0x00CD
    int GridFeed_TotalL; // ori 0x00CE
    int GridCharge_TotalH; // ori 0x00CF
    int GridCharge_TotalL; // ori 0x00D0
// 0x0170
    int PV_Today_EnergyH; // ori 0x00C5 //Ppv_TodayH;
    int PV_Today_EnergyL; // ori 0x00C6 //Ppv_TodayL;
    int Bat_Charge_Today_EnergyH;
    int Bat_Charge_Today_EnergyL;
    int Bat_Discharge_Today_EnergyH;
    int Bat_Discharge_Today_EnergyL;
    int Load_Today_EnergyH;
    int Load_Today_EnergyL;
    int Negative_Load_Today_EnergyH;
    int Negative_Load_Today_EnergyL;
    int GridFeed_TodayH;
    int GridFeed_TodayL;
    int GridCharge_TodayH;
    int GridCharge_TodayL;
    int PV_Month_EnergyH;
    int PV_Month_EnergyL;
// 0x0180
    int Bat_Charge_Month_EnergyH;
    int Bat_Charge_Month_EnergyL;
    int Bat_Discharge_Month_EnergyH;
    int Bat_Discharge_Month_EnergyL;
    int Load_Month_EnergyH;
    int Load_Month_EnergyL;
    int Negative_Load_Month_EnergyH;
    int Negative_Load_Month_EnergyL;
    int GridFeed_MonthH;
    int GridFeed_MonthL;
    int GridCharge_MonthH;
    int GridCharge_MonthL;
    int CT_Total_Feedin_EnergyH;
    int CT_Total_Feedin_EnergyL;
    int CT_Today_Feedin_EnergyH;
    int CT_Today_Feedin_EnergyL;
// 0x0190
    int CT_Total_Charge_EnergyH;
    int CT_Total_Charge_EnergyL;
    int CT_Today_Charge_EnergyH;
    int CT_Today_Charge_EnergyL;
// 1.42 add
// 0x01A0
    int PV_Year_EnergyH;
    int PV_Year_EnergyL;
    int Bat_Charge_Year_EnergyH;
    int Bat_Charge_Year_EnergyL;
    int Bat_Discharge_Year_EnergyH;
    int Bat_Discharge_Year_EnergyL;
    int Load_Year_EnergyH;
    int Load_Year_EnergyL;
    int Negative_Load_Year_EnergyH;
    int Negative_Load_Year_EnergyL;
    int GridFeed_Year_EnergyH;
    int GridFeed_Year_EnergyL;
    int GridCharge_Year_EnergyH;
    int GridCharge_Year_EnergyL;
    int PV1_Todat_Energy;
    int PV2_Todat_Energy;
// 0x01B0
    int PV3_Todat_Energy;
    int PV4_Todat_Energy;
    int PV1_Todat_Energy_Peak_Power;
    int PV1_Todat_Energy_Peak_Current;
    int PV1_Todat_Energy_Peak_Voltage;
    int PV2_Todat_Energy_Peak_Power;
    int PV2_Todat_Energy_Peak_Current;
    int PV2_Todat_Energy_Peak_Voltage;
    int PV3_Todat_Energy_Peak_Power;
    int PV3_Todat_Energy_Peak_Current;
    int PV3_Todat_Energy_Peak_Voltage;
    int PV4_Todat_Energy_Peak_Power;
    int PV4_Todat_Energy_Peak_Current;
    int PV4_Todat_Energy_Peak_Voltage;
    int GridCharge_Todat_Energy_Peak_Power;
    int GridCharge_Todat_Energy_Peak_Current;
// 0x01C0
    int GridFeed_Today_Energy_Peak_Power;
    int GridFeed_Today_Energy_Peak_Current;
    int Grid_Today_Energy_Peak_Voltage_Min;
    int Grid_Today_Energy_Peak_Voltage_Max;
    int Grid_Today_Energy_Peak_Frequency_Min;
    int Grid_Today_Energy_Peak_Frequency_Max;
    int Bat_Charge_Today_Energy_Peak_Current;
    int Bat_Discharge_Today_Energy_Peak_Current;
    int Bat_Today_Energy_Peak_Voltage_Min;
    int Bat_Today_Energy_Peak_Voltage_Max;
    int Load_Today_Energy_Peak_Power;
    int Load_Today_Energy_Peak_Current;
    int Negative_Load_Today_Energy_Peak_Power;
    int Negative_Load_Today_Energy_Peak_Current;
    int Load_Today_Energy_Peak_Voltage_Min;
    int Load_Today_Energy_Peak_Voltage_Max;
// 0x01D0
    int Load_Today_Energy_Peak_Frequency_Min;
    int Load_Today_Energy_Peak_Frequency_Max;
}HB2_CE_VALUE;

typedef struct stHybrid2_Display_Info {
// 0x01A0 => 0x01E0
    int Display_Working_State; // new
    int Output_Power_Restraint_Reeson; // new
    int Battery_To_Load_Consumption_Time; // new
    int OnGrid_CountDown; // new
    int Hybrid_IconL; // ori 0x00D9
    int Hybrid_IconH; // ori 0x00DA
}HB2_DP_INFO;

typedef struct stHybrid2_Icon_Info {
    char B0_1_PV;
    char B2_3_Battery;
    char B4_5_Grid;
    char B6_7_Load;
    char B8_1O_Inverter_System;
    char B11_Generator;
    char B12_DL_Comm;
    char B13_Cloud_Comm;
    char B14_Bat_Calibration;
    char B15_Derate;
    char B16_PV_To_Battery;
    char B17_PV_To_Grid;
    char B18_PV_To_Load;
    char B19_Battery_To_Grid;
    char B20_Battery_To_Load;
    char B21_Grid_To_Battery;
    char B22_Grid_To_Load;
    char B23_Load_To_Battery;
    char B24_Load_To_Grid;
    char B25_26_Mode;
    char B27_29_System_Status;
}HB2_ICON_INFO;

typedef struct stHybrid2_RTC_Data {
// 0x01D0 => 0x0150
    int Second;
    int Minute;
    int Hour;
    int Date;
    int Month;
    int Year;
}HB2_RTC_DATA;

typedef struct stHybrid2_ARC_Info {
// 0x01F0
    int Self_Testing;
    int Warning_Threshold;
    int Warning_Clear;
    int Reset_EEPROM;
}HB2_ARC_INFO;

typedef struct stHybrid2_BMS_Info {
// 0x0200
    int Charge_Voltage_Requirement; // rename, ori Voltage;
    int Charge_Current_Requirement; // rename, ori Current;
    int Number_Of_Module_Warning; // new
    int Master_Total_Voltage; // new
    int Discharging_Charging_Current; // new
    int SOC; // ori 0x0202
    int FCC; // new
    int RC; // new
    int Status_Flag_Register; // rename, ori 0x0205 Status;
    int IO_Flag_Register; // new
    int Warning_Flag_Register; // new
    int Alarm_Flag_Register; // new
    int Max_Cell_Voltage; // rename, ori 0x0209 BMS_Max_Cell;
    int Min_Cell_Voltage; // rename, ori 0x020A BMS_Min_Cell;
    int Max_Cell_Temperature; // rename, ori 0x0203 MaxTemperature;
    int Min_Cell_Temperature; // new
// 0x0210
    int Master_Average_Cycle_Count; // rename, ori 0x0204 CycleCount;
    int Master_Average_SOH; // new
    int Battery_Cumulative_Input_Capacity; // new
    int Battery_Cumulative_Output_Capacity; // new
    int Master_FW_Version; // new
    int Master_Manufacture_Data; // new
    int Master_Serial_Number; // new
    int Number_Of_Whole_Cells; // new
    int Number_Of_Module; // rename, ori 0x0207 Module No.
}HB2_BMS_INFO;

typedef struct stDL_CMD {
    char    m_Sn[17];
    int     m_addr;  // start address
    int     m_count; // number of data
    int     m_data[32]; // for now set size 32, because data range max 0xB0 ~ 0xC8 for H5500/9600
}DL_CMD;

typedef struct stDL_Config {
    char    m_sms_server[128];
    int     m_sms_port;
    int     m_sample_time;
    int     m_delay_time_1; // us, 1000000 us = 1 s
    int     m_delay_time_2;
    int     m_inverter_port;
    int     m_inverter_baud;
    int     m_inverter_data_bits;
    char    m_inverter_parity[8];
    int     m_inverter_stop_bits;
}DL_CONFIG;

typedef struct stDL_Path {
    char    m_root_path[128];
    char    m_xml_path[128];
    char    m_log_path[128];
    char    m_errlog_path[128];
    char    m_bms_path[128];
    char    m_syslog_path[128];
    char    m_env_path[128];
}DL_PATH;
#endif // GDEFINE_H_INCLUDE
