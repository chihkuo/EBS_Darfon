#export STAGING_DIR = /home/chih/toolchain/lede-sdk/staging_dir
#CC = /home/chih/toolchain/lede-sdk/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-musl-gcc
#CPP = /home/chih/toolchain/lede-sdk/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-musl-g++
#AR = /home/chih/toolchain/lede-sdk/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-musl-ar

export STAGING_DIR = /home/chih/toolchain/openwrt-sdk
CC = /home/chih/toolchain/openwrt-sdk/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-musl-gcc
CPP = /home/chih/toolchain/openwrt-sdk/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-musl-g++
AR = /home/chih/toolchain/openwrt-sdk/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-musl-ar

.PHONY: all clean

all:
	cd iniparser-2.17; make
	cd ../common; make
	$(CC) -Wall -c modbusdrv.c
	$(CC) -Wall -c Darfonlogger.c
	$(CPP) -Wall -c G320.cpp
	$(CPP) -Wall main.cpp modbusdrv.o Darfonlogger.o G320.o iniparser-2.17/src/dictionary.o iniparser-2.17/src/strlib.o iniparser-2.17/src/iniparser.o ../common/SaveLog.o -o dlg320.exe

clean:
	cd iniparser-2.17; make clean
	cd ../common; make clean
	rm *.o
	rm dlg320.exe
	
