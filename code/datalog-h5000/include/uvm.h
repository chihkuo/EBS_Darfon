/**
 * @file uvm.h
 *
 * Uyghur Virtual Machine Definition.
 * Copyright(c) Rupert Li @ AMIT, Inc.
 */

#include <stdint.h>

#define VER_MAJOR       1
#define VER_MINOR       3

#define UO_MAGIC        0x7955

typedef struct {
	/* part 1: uoimage attributes */
	uint16_t magic;
	uint8_t  ver_major;                 //major version of uvm
	uint8_t  ver_minor;                 //minor version of uvm
	uint16_t csid_rev;                  //revision of CSID repository
	uint16_t object_attr;               //object attribute
	uint32_t resv[2];
	/* part 2: object code information*/
	uint32_t text_offset;               //text offset to start of object
	uint32_t data_offset;               //data offset to start of object
	uint32_t entry_point;               //instruction pointer to execute
	uint32_t object_size;               //object size
} UO_HEADER;

int uvm_init(char *uo, int csman_fh, int flags);
int uvm_envar_num(void);
char *uvm_envar_name(void);
char *uvm_envar_next(char *ev);
int uvm_envar_set(int varno, int type, char *val);
int uvm_run(int (*print) (char *buf, int count));
int uvm_shut(void);
