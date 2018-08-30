#
#This file is for RELEASE IN SDK COPY
#
#
ifdef TARGET
	TARGET_PREFIX = $(TARGET)-
endif

ifndef CROSS_COMPILE
	CROSS_COMPILE = $(TARGET_PREFIX)
	CC = $(CROSS_COMPILE)gcc
	STRIP = $(CROSS_COMPILE)strip
endif

ifdef LOG_COLLECTOR
	CFLAGS += -DLOG_COLLECTOR=$(LOG_COLLECTOR)
endif

ifdef DEBUG_LEVEL
	CFLAGS += -DDEBUG_LEVEL=$(DEBUG_LEVEL)
endif

ifndef KERNEL_DIR
	KERNEL_DIR=/usr/src/linux
endif

ifdef ENDIAN
	CFLAGS += -DHANDLE_BIG_ENDIAN=1
endif

# this must be added
CFLAGS += -DSDK_RELEASE

#
# user can recompile .o files like handle_helloworld.o into commander
#
OBJS = commander_sdk.o \
	handle_helloworld.o\
	handle_darfonlogger.o


EXEC = commander

INCPATH= -I./ -I../include -I../../include -I$(KERNEL_DIR)/include 

LIBPATH= -L../$(TARGET_PREFIX)lib -L../../$(TARGET_PREFIX)lib

CFLAGS += $(INCPATH)

BINARIES = $(EXEC)

LFLAGS += $(LIBPATH) -lcommander -lcsman -lupgbuf -lcrypt -lpthread

ifeq ($(HANDLE_RADIUS_USER),1)
INCPATH += -I$(TOPDIR)/tmp/mysql_mips/include/mysql
LIBPATH += -rdynamic -L$(TOPDIR)/tmp/mysql_mips/lib/mysql -lmysqlclient -lz -lcrypt -lnsl -lm
endif

all: precheck $(BINARIES)
	$(foreach var,$(BINARIES),$(STRIP) --remove-section=.note --remove-section=.comment $(var);)

$(EXEC): $(OBJS)
	@$(CC) $(LFLAGS) -o ${EXEC} $(OBJS)


%.o: %.c
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

clean:
	-rm -f $(BINARIES) *.o

install: postcheck
	@cp -a commander					$(INSTALL_ROOT)/usr/sbin
	
precheck:
	@echo ""
	@echo "Build $(shell basename `pwd`) for $(PROJECT_NAME) ($(PROJECT_CUSTOMER))"
ifeq ($(PROJECT_CUSTOMER), )
	@echo ""
	@echo "No specify PROJECT_CUSTOMER in Makefile. "
	@echo "Maybe you should update the components Makefile of the Project"
	@echo ""
	@exit 1
endif

postcheck:
	@echo ""
	@echo "Install $(shell basename `pwd`) to $(INSTALL_ROOT)"
ifeq ($(INSTALL_ROOT), )
	@echo ""
	@echo "No specify INSTALL_ROOT in Makefile."
	@echo "Maybe you should update the components Makefile of the Project"
	@echo ""
	@exit 1
endif

