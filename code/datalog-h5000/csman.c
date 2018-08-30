#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "csman.h"

#define SNMPRESULT "/tmp/snmp_return"
#define TMPSIZE 8192

#define TYPE_U8 "u8"
#define TYPE_U16 "u16"
#define TYPE_U32 "u32"
#define TYPE_STR "str"
#define TYPE_IPV4 "ipv4"
#define TYPE_MAC "mac"

#define LOCAL_WANTYPE_DHCP              0x00
#define LOCAL_WANTYPE_NULL              0x01
#define LOCAL_WANTYPE_3G                0x10
#define LOCAL_WANTYPE_IBURST            0x11
#define LOCAL_WANTYPE_WIBRO             0x12
#define LOCAL_WANTYPE_WISP              0x13
#define LOCAL_WANTYPE_FIXED             0x20
#define LOCAL_WANTYPE_PPPOE             0x40
#define LOCAL_WANTYPE_PPPOE_MULTI       0x41
#define LOCAL_WANTYPE_PPTP              0x60
#define LOCAL_WANTYPE_L2TP              0x80
#define LOCAL_WANTYPE_DIALUP            0x90

char switch_type(char *type);

int open_csman(const char *config, int flags)
{
#ifdef  _TEST
    return 1;
#endif
    mode_t mode=S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
    int fd = open(SNMPRESULT, O_CREAT|O_RDWR, mode);
	return fd;
}
ssize_t read_csman(int fd, unsigned int csid, void *buf, size_t count, char *type)
{
	int ret=0;
	char cmd[8192];
	char result;
	//char result;
#ifdef  _TEST
    return 1;
#endif

    if (csid == 0)
		return -61;

	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"rdcsman %d %s > %s",csid,type,SNMPRESULT);
	ret = system(cmd);
	if (ret!=0) return -1;

	result = switch_type(type);

	int statfd;

	char tmp[TMPSIZE];
	memset(tmp,0,TMPSIZE);

	if((statfd=open(SNMPRESULT, O_RDONLY, 0)) != -1)
	{
		read(statfd,tmp,TMPSIZE-1);
		close(statfd);
		switch(result)
		{
			case '0':
	//			printf("u8 u16 u32\n");
				sscanf(tmp,"%u",(int *)buf);
				break;
			case '1':
	//			printf("str\n");
				sprintf((char *)buf,"%s",tmp);
				break;
			case '2':
				{
				printf("ipv4\n");
				char *result = NULL;
				char delims[] = ".";
				unsigned char ip[4];
				int i=0;

				result = strtok( tmp, delims );

				ip[i] = atoi(result);
				*((unsigned char *)buf+i)=ip[i];

				i++;
				for(i;i<4;i++)
				{
					result = strtok( NULL, delims );
					ip[i] = atoi(result);
					*((unsigned char *)buf+i)=ip[i];
				}
				break;
				}
			case '3':
				{
					//			printf("mac\n");
					char *result = NULL;
					char delims[] = "-";
					unsigned char mac[6];
					int i=0;
					result = strtok( tmp, delims );
					//mac[i] = atoi(result);
					mac[i] = strtol(result,NULL,16); //frank: fix read mac bug!
					*((unsigned char *)buf+i)=mac[i];
					i++;
					for(i;i<6;i++)
					{
						result = strtok( NULL, delims );
						//mac[i] = atoi(result);
						mac[i] = strtol(result,NULL,16); //frank: fix read mac bug!
						*((unsigned char *)buf+i)=mac[i];
					}
//				sscanf(tmp,"%s",(char *)buf);
					break;
				}
		}
	}
	return 0;

}
ssize_t write_csman(int fd, unsigned int csid, const void *buf, size_t count, char *type)
{
	int ret=0;
	char cmd[8192];
	char result;
	//char result;
#ifdef  _TEST
    return 1;
#endif
    if (csid == 0)
		return -61;
printf("write_csman: fd=%d, csid=%d, buf=%s, type=%s",fd,csid,(char *)buf,type);

	memset(cmd,0,sizeof(cmd));
	//sprintf(cmd,"rdcsman %d %s > %s",csid,type,SNMPRESULT);
//	ret = system(cmd);
//	if (ret!=0) return -1;

	result = switch_type(type);

//	int statfd;

//	char tmp[TMPSIZE];
//	memset(tmp,0,TMPSIZE);

//	if((statfd=open(SNMPRESULT, O_RDONLY, 0)) != -1)
//	{
//		read(statfd,tmp,TMPSIZE-1);
//		close(statfd);
	switch(result)
	{
		case '0':
			//			printf("u8 u16 u32\n");
			sprintf(cmd,"wrcsman \"%d %u\"",csid,*(int *)buf);
			break;
		case '1':
			//			printf("str\n");
			sprintf(cmd,"wrcsman \"%d \\\"%s\"",csid,(char*)buf);
			break;
		case '2':
			//			printf("ipv4\n");
//			printf("(char *) = %s\n",(char*)buf);
			sprintf(cmd,"wrcsman \"%d {%u.%u.%u.%u}\"",csid,*(char*)buf,*((char*)buf+1),*((char*)buf+2),*((char*)buf+3));
			//sscanf(tmp,"%s",buf);
			break;
		case '3':
			//			printf("mac\n");
			//sprintf(cmd,"wrcsman \"%d {%u-%u-%u-%u-%u-%u}\"",csid,*(char*)buf,*((char*)buf+1),*((char*)buf+2),*((char*)buf+3),*((char*)buf+4),*((char*)buf+5));

		  {
					char *result1 = NULL,*get_check;
					char delims[] = "-",delims2[]=":";
					unsigned char tmp1[6];
					unsigned char mac1[6];
					int i=0;
					/*2014.03.13, aaron_wu: add support using :*/
					get_check=strstr(buf,delims);
					if(get_check)	result1 = strtok( buf, delims );	//cut -
					else		result1 = strtok( buf, delims2);	//cut :

					tmp1[i] = strtol(result1,NULL,16); //frank: fix read mac bug!
					*((unsigned char *)mac1+i)=tmp1[i];
					i++;
					for(i;i<6;i++)
					{
						if(get_check)	result1 = strtok( NULL, delims );
						else		result1 = strtok( NULL, delims2 );
						tmp1[i] = strtol(result1,NULL,16); //frank: fix read mac bug!
						*((unsigned char *)mac1+i)=tmp1[i];
					}

			sprintf(cmd,"wrcsman \"%d @6 %u %u %u %u %u %u\"",csid,*(char*)mac1,*((char*)mac1+1),*((char*)mac1+2),*((char*)mac1+3),*((char*)mac1+4),*((char*)mac1+5));
			//printf("aaaaaaaaaaaaaaaaa_mac_cmd:%s\n",cmd);
			break;
	}
	}
	//	}
	//printf("cmd = %s\n",cmd);
	ret = system(cmd);
	if (ret!=0) return -1;
	return 0;

}
int close_csman(int fd)
{
	return 0;
}

char switch_type(char *type)
{
	char result;

	if(strcmp(TYPE_U32,type) == 0)
	{
//		printf("result = 0,u32\n");
		result = '0';
	}
	else if(strcmp(TYPE_U16,type) == 0)
	{
//		printf("result = 0,u16\n");
		result = '0';
	}
	else if(strcmp(TYPE_U8,type) == 0)
	{
//		printf("result = 0,u8\n");
		result = '0';
	}
	else if(strcmp(TYPE_STR,type) == 0)
	{
//		printf("result = 1,str\n");
		result = '1';
	}
	else if(strcmp(TYPE_IPV4,type) == 0)
	{
//		printf("result = 2,ipv4\n");
		result = '2';
	}
	else if(strcmp(TYPE_MAC,type) == 0)
	{
//		printf("result = 3,mac\n");
		result = '3';
	}
	return result;

}
