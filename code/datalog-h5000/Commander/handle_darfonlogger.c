//This is for RELEASE IN SDK COPY
/**********************************************************************************************/
// This file is for DEMO use.
// To let it work, You need remove 3 comments from commander.h, commander_sdk.c and Makefile.
// Searching "helloworld" as a keyword is helpful.
// Executing "rrlog" command will show system log, let you check the result when you are in console mode.
//
/**********************************************************************************************/
#include "commander.h"
#include "csid/csid_darfonlogger.h"



#define DeviceMode 1 //0:MI,1:Hybrid




static int nf_handle_darfonlogger(void *arg, int fd);

int failcount=0;
int waitcount=0;


char DarfonloggerMemSizepath[50]="/tmp/MemSize.txt";

#if DeviceMode==0
char ComponentName[40]="Darfonlogger";
char ComponentLaunchCmd[40]="Darfonlogger&";
#else if DeviceMode==1
char ComponentName[40]="DarfonloggerH";
char ComponentLaunchCmd[40]="DarfonloggerH&";

#endif

int csmanfd;
int CheckMem()
{
    char line[100];
    FILE *fp;
    int count;	
	char cmd[100];
        
	memset(cmd,0,sizeof(cmd));	
	sprintf(cmd,"free | grep 'Mem' | awk '{print $4}'>%s", DarfonloggerMemSizepath);
	system(cmd);
    
    count=0;
    if((fp=fopen(DarfonloggerMemSizepath,"r"))==NULL)
    {
        printf("CheckMem: file not exits\n");        
		return;
    }
    
    while(!feof(fp))
    {
        memset(line,0,sizeof(line));
        fgets(line,100,fp);
        if(strlen(line)>0)
        {            
            int memsize = atoi ( line );         
            if(memsize<20000)
            {                
                unsigned int reboot=1;
    			write_csman(csmanfd, CSID_S_LOCAL_REBOOT_ALTERED, &reboot, sizeof(unsigned int), CSM_W_ZERO);//reboot    			
            }
        }
    }
    fclose(fp);

}


pid_t proc_find(const char* name) 
{
    DIR* dir;
    struct dirent* ent;
    char buf[512];

    long  pid;
    char pname[100] = {0,};
    char state;
    FILE *fp=NULL; 

    if (!(dir = opendir("/proc"))) {
        perror("can't open /proc");
        return -1;
    }

    while((ent = readdir(dir)) != NULL) {
        long lpid = atol(ent->d_name);
        if(lpid < 0)
            continue;
        snprintf(buf, sizeof(buf), "/proc/%ld/stat", lpid);
        fp = fopen(buf, "r");

        if (fp) {
            if ( (fscanf(fp, "%ld (%[^)]) %c", &pid, pname, &state)) != 3 ){
                printf("fscanf failed \n");
                fclose(fp);
                closedir(dir);
                return -1; 
            }
            if (!strcmp(pname, name)) {
                fclose(fp);
                closedir(dir);
                return (pid_t)lpid;
            }
            fclose(fp);
        }
    }


closedir(dir);
return -1;
}




/**********************************************************************************************/
// init_helloworld() is a initial function, will initialize and register nf_handle_helloworldd()
// as a callback function into Commander monitor routine.
//
/**********************************************************************************************/
int init_darfonlogger(int fd, CMD_JOBS_T *job)
{

/*	
	unsigned int MODE = 0;
	read_csman(csmanfd, CSID_C_DARFONLOGGER_ROM_RULE_MODE, &MODE, sizeof(unsigned int), CSM_R_ZERO);
	if(MODE==0)
	{		
		sprintf(ComponentName,"Darfonlogger");
		sprintf(ComponentLaunchCmd,"Darfonlogger&");
	}
	else if(MODE==1)
	{
		sprintf(ComponentName,"DarfonloggerH");
		sprintf(ComponentLaunchCmd,"DarfonloggerH&");
	}
	else
	{
		MODE=0;
		write_csman(csmanfd, CSID_C_DARFONLOGGER_ROM_RULE_MODE, &MODE, sizeof(unsigned int), 0);
		sprintf(ComponentName,"Darfonlogger");
		sprintf(ComponentLaunchCmd,"Darfonlogger&");
	}
*/		
	
	
	system(ComponentLaunchCmd);
	return add_notifier(JOB_REGULAR_ROUTINE, nf_handle_darfonlogger, job, job);
}

/**********************************************************************************************/
// nf_handle_helloworldd() is a callback function.
// It will be invoked by Commander every 1 second.
// Here you can do what you want to do.
//
/**********************************************************************************************/
static int nf_handle_darfonlogger(void *arg, int fd)
{	
	//mike20150408disable
	//keel Darfonlogger Alive
	if(waitcount>20)
	{
		waitcount=0;
		pid_t pid = proc_find(ComponentName);
	    if (pid == -1) {
	        printf("commander: %s not found\n", ComponentName);
			failcount++;		
	    }
		else
		{
			printf("commander: %s: alive\n", ComponentName);
			failcount=0;		
		}
		
		
		
		if(failcount>5)
		{
			system(ComponentLaunchCmd);
			failcount=0;	
            CheckMem();
		}
		/*

		unsigned int reboot = 0;
		int csmanfd;

		if((csmanfd = open_csman(NULL,0)) < 0)
	    {
	        //LPRINTF(PRI_INFO, "Can't Open CSMAN\n");
	        //return -1;
	    }
		else
		{		    
		    read_csman(csmanfd, CSID_S_HELLOWORLD_RAM_ALTERED, &reboot, sizeof(unsigned int), CSM_R_ZERO);
		    if(reboot)
			{
		        reboot = 0;
		        write_csman(csmanfd, CSID_S_HELLOWORLD_RAM_ALTERED, &reboot, sizeof(unsigned int), 0);		    			
				system("reboot");		        
			}
			close_csman(csmanfd);
		}
		*/
	}

	waitcount++;
	//
	
	
    return 0;
}

