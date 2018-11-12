#define INV_COUNT   5
#define INV_SIZE    64

enum INVERTER_ID{
    ID_Unknown,
    ID_Darfon,
    ID_CyberPower1P,
    ID_CyberPower3P,
    ID_Test
};

char INVERTER[INV_COUNT][INV_SIZE] = {
"Unknown",
"Darfon",
"CyberPower-1P",
"CyberPower-3P",
"Test"
};
