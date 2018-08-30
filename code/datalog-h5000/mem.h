#include "16-Bit Flash Programmer.h"

#define PM_SIZE 1536 /* Max: 144KB/3/32=1536 PM rows for 30F. */
#define EE_SIZE 128 /* 4KB/2/16=128 EE rows */
#define CM_SIZE 8//mike0116*#define CM_SIZE 8



typedef enum 
{
	Program,
	EEProm,
	Configuration
}eType;

typedef struct mem_cMemRow
{
    char           * m_pBuffer;
	int				 m_RowSize;
	unsigned short   m_Data[PM33F_ROW_SIZE*2];
    unsigned int     m_Address;
	int             m_bEmpty;	
    eType            m_eType;	
	int              m_RowNumber;	
    eFamily          m_eFamily;
	int             IfCheckAck;
} mem_cMemRow;



