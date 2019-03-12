#export STAGING_DIR = /home/chih/toolchain/lede-sdk/staging_dir
#CC = /home/chih/toolchain/lede-sdk/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-musl-gcc

export STAGING_DIR = /home/chih/toolchain/openwrt-sdk
CC = /home/chih/toolchain/openwrt-sdk/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-musl-gcc

.PHONY: all clean

all:
	$(CC) -Wall main.c ../common/libcommon.a -o DataProgram.exe

clean:
	rm DataProgram.exe
	
