//This is for RELEASE IN SDK COPY
/**********************************************************************************************/
// This file is for DEMO use.
// To let it work, You need remove 3 comments from commander.h, commander_sdk.c and Makefile.
// Searching "helloworld" as a keyword is helpful.
// Executing "rrlog" command will show system log, let you check the result when you are in console mode.
//
/**********************************************************************************************/
#include "commander.h"
#include "csid/csid_helloworld.h"



static int nf_handle_helloworldd(void *arg, int fd);

/**********************************************************************************************/
// init_helloworld() is a initial function, will initialize and register nf_handle_helloworldd()
// as a callback function into Commander monitor routine.
//
/**********************************************************************************************/
int init_helloworld(int fd, CMD_JOBS_T *job)
{
	unsigned int helloworld_enable=0;

    read_csman(fd, CSID_C_HELLOWORLD_ROM_ENABLE, &helloworld_enable, sizeof(unsigned int), CSM_R_ZERO);

	LPRINTF(PRI_INFO, "[%s][%d]\n", __func__, __LINE__);
    if(helloworld_enable == 1) 
	{
		system("helloworld-action start");
		LPRINTF(PRI_INFO, "[%s][%d] ------- hello world start -------\n", __func__, __LINE__);
    }

	return add_notifier(JOB_REGULAR_ROUTINE, nf_handle_helloworldd, job, job);
}

/**********************************************************************************************/
// nf_handle_helloworldd() is a callback function.
// It will be invoked by Commander every 1 second.
// Here you can do what you want to do.
//
/**********************************************************************************************/
static int nf_handle_helloworldd(void *arg, int fd)
{
	unsigned int helloworld_enable = 0, ui_alter = 0;
	int csmanfd;

	if((csmanfd = open_csman(NULL,0)) < 0)
    {
        LPRINTF(PRI_INFO, "Can't Open CSMAN\n");
        return -1;
    }

    read_csman(csmanfd, CSID_C_HELLOWORLD_ROM_ENABLE, &helloworld_enable, sizeof(unsigned int), CSM_R_ZERO);
    read_csman(csmanfd, CSID_S_HELLOWORLD_RAM_ALTERED, &ui_alter, sizeof(unsigned int), CSM_R_ZERO);
    if(ui_alter)
	{
        ui_alter = 0;
        write_csman(csmanfd, CSID_S_HELLOWORLD_RAM_ALTERED, &ui_alter, sizeof(unsigned int), 0);
    	if (helloworld_enable == 1) 
		{			
			system("helloworld-action restart");
			LPRINTF(PRI_INFO, "[%s][%d] ------- hello world restart -------\n", __func__, __LINE__);
        } 
		else
		{
			system("helloworld-action stop");
			LPRINTF(PRI_INFO, "[%s][%d] ------- hello world stop -------\n", __func__, __LINE__);
        }
	}

	close_csman(csmanfd);
    return 0;
}

