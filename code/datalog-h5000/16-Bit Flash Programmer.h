#define BUFFER_SIZE         4096

#define PM30F_ROW_SIZE 32
#define PM33F_ROW_SIZE 64*8
#define EE30F_ROW_SIZE 16

#define COMMAND_NACK     0x00
#define COMMAND_ACK      0x01
#define COMMAND_READ_PM  0x02
#define COMMAND_WRITE_PM 0x03
#define COMMAND_READ_EE  0x04
#define COMMAND_WRITE_EE 0x05
#define COMMAND_READ_CM  0x06
#define COMMAND_WRITE_CM 0x07
#define COMMAND_RESET    0x08
#define COMMAND_READ_ID  0x09
#define COMMAND_READ_PM_PAGE  	0x0A
#define COMMAND_WRITE_PM_PAGE 	0x0B
#define COMMAND_WRITE_PM_FLUSH 	0x0C
#define COMMAND_READ_BOOTLOADER_VERSION 	0x0D
#define COMMAND_READ_PM_BLOCK_CRC  	0x0E


typedef enum 
{
    dsPIC30F,
    dsPIC33F,
    PIC24H,
    PIC24F,
    Unknow//mike+
}eFamily;

typedef struct
{
    char               * pName;
    unsigned short int   Id;
    unsigned short int   ProcessId;
    eFamily              Family;
} sDevice;










