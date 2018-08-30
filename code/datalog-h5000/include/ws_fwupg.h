
/* Weblet Local Attributes: 0x00-- ~ 0xFF--*/

#define WSFL_FWUPG_RESCUE	0x8000

#if defined(GET_FW_BEFORE_UPG)
    /* Upload Firmware to Ram, and Calling Exec_UPGBU */
    #define OPEN_UPGBUF(flags)                      open_upgfile(flags)
    #define WRITE_UPGBUF(handle, buf, count)        write_upgfile(handle, buf, count)
    #define READ_UPGBUF(handle, buf, start, count)  read_upgfile(handle, buf, start, count)
    #define CLOSE_UPGBUF(handle)                    close_upgfile(handle)
    #define EXEC_UPGBUF(flags)                      exec_upgfile(flags)
#else
    #define OPEN_UPGBUF(flags)                      open_upgbuf(flags)        
    #define WRITE_UPGBUF(handle, buf, count)        write_upgbuf(handle, buf, count)
    #define READ_UPGBUF(handle, buf, start, count)  read_upgbuf(handle, buf, start, count)
    #define CLOSE_UPGBUF(handle)                    close_upgbuf(handle)
    #define EXEC_UPGBUF(flags)                      0
#endif
