require("luci.sys")
require("luci.config")

local map = Map("dlsetting", translate("DL Setting"), translate("Data Logger Parameter Setting"))

-- SMS setting
local sms = map:section(TypedSection, "sms", "SMS setting")
sms.addremove = false
sms.anonymous = true

local sms_server_ip = sms:option(Value, "sms_server", translate("SMS Server"))
local sms_server_port = sms:option(Value, "sms_port", translate("SMS Port"))
local update_server_ip = sms:option(Value, "update_server", translate("Update Server"))
local update_server_port = sms:option(Value, "update_port", translate("Update Port"))

local sample_value = {1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60, 120, 180, 300, 600, 1200, 1800, 3600}
local sample_time = sms:option(ListValue, "sample_time", translate("Sample time (Sec.)"))
for i = 1,#sample_value do
	sample_time:value(sample_value[i])
end

local upload_value = {1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60, 120, 180, 300, 600, 1200, 1800, 3600}
local upload_time = sms:option(ListValue, "upload_time", translate("Upload time (Sec.)"))
for i = 1,#upload_value do
	upload_time:value(upload_value[i])
end

local update_SW_value = {5, 10, 15, 20, 30, 60}
local update_SW_time = sms:option(ListValue, "update_SW_time", translate("Update SW time (Min.)"))
for i = 1,#update_SW_value do
	update_SW_time:value(update_SW_value[i])
end

local delay_time_1 = sms:option(Value, "delay_time_1", translate("Delay time 1 (us. 1000000us = 1s)"))
local delay_time_2 = sms:option(Value, "delay_time_2", translate("Delay time 2 (us. 1000000us = 1s)"))
local cleartx_delay = sms:option(Value, "cleartx_delay", translate("Clear TX delay time (us. 1000000us = 1s)"))
local shelf_life = sms:option(Value, "shelf_life", translate("Data shelf life (Day.)"))
local reboot_time = sms:option(Value, "reboot_time", translate("Reboot time (Day.)"))

local update_FW_value = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23}
local update_FW_start = sms:option(ListValue, "update_FW_start", translate("Update FW start time (clock)"))
for i = 1,#update_FW_value do
	update_FW_start:value(update_FW_value[i])
end
local update_FW_stop = sms:option(ListValue, "update_FW_stop", translate("Update FW stop time (clock)"))
for i = 1,#update_FW_value do
	update_FW_stop:value(update_FW_value[i])
end

-- COM port setting
local comport = map:section(TypedSection, "comport", "COM Port setting")
comport.addremove = false
comport.anonymous = true

--[[
comport:tab("port_select",  translate("COM Port select"))
local allport = {"COM1", "COM2", "COM3", "COM4"}
--local allmodel = {"Darfon", "Test"}
local inverter_port = comport:taboption("port_select", ListValue, "inverter_port", translate("Inverter Port"))
for i = 1,#allport do
	inverter_port:value(allport[i])
end
--local inverter_model = comport:taboption("port_select", ListValue, "inverter_model", translate("Inverter Model"))
--for i = 1,#allmodel do
--	inverter_model:value(allmodel[i])
--end
local other1_port = comport:taboption("port_select", ListValue, "other1_port", translate("Other1 Port"))
for i = 1,#allport do
	other1_port:value(allport[i])
end
local other2_port = comport:taboption("port_select", ListValue, "other2_port", translate("Other2 Port"))
for i = 1,#allport do
	other2_port:value(allport[i])
end
local other3_port = comport:taboption("port_select", ListValue, "other3_port", translate("Other3 Port"))
for i = 1,#allport do
	other3_port:value(allport[i])
end
]]

comport:tab("com1_setting",  translate("COM1"))
local baudrate = {4800, 9600, 19200, 28800, 38400, 57600, 115200}
local alldatabits = {5, 6, 7, 8}
local allparity = {"None", "Odd", "Even"}
local allstopbits = {1, 2}
local com1_baud = comport:taboption("com1_setting", ListValue, "com1_baud", translate("Baud rate"))
for i = 1,#baudrate do
	com1_baud:value(baudrate[i])
end
local com1_data_bits = comport:taboption("com1_setting", ListValue, "com1_data_bits", translate("Data bits"))
for i = 1,#alldatabits do
	com1_data_bits:value(alldatabits[i])
end
local com1_parity = comport:taboption("com1_setting", ListValue, "com1_parity", translate("Parity"))
for i = 1,#allparity do
	com1_parity:value(allparity[i])
end
local com1_stop_bits = comport:taboption("com1_setting", ListValue, "com1_stop_bits", translate("Stop bits"))
for i = 1,#allstopbits do
	com1_stop_bits:value(allstopbits[i])
end

comport:tab("com2_setting",  translate("COM2"))
local com2_baud = comport:taboption("com2_setting", ListValue, "com2_baud", translate("Baud rate"))
for i = 1,#baudrate do
	com2_baud:value(baudrate[i])
end
local com2_data_bits = comport:taboption("com2_setting", ListValue, "com2_data_bits", translate("Data bits"))
for i = 1,#alldatabits do
	com2_data_bits:value(alldatabits[i])
end
local com2_parity = comport:taboption("com2_setting", ListValue, "com2_parity", translate("Parity"))
for i = 1,#allparity do
	com2_parity:value(allparity[i])
end
local com2_stop_bits = comport:taboption("com2_setting", ListValue, "com2_stop_bits", translate("Stop bits"))
for i = 1,#allstopbits do
	com2_stop_bits:value(allstopbits[i])
end

comport:tab("com3_setting",  translate("COM3"))
local com3_baud = comport:taboption("com3_setting", ListValue, "com3_baud", translate("Baud rate"))
for i = 1,#baudrate do
	com3_baud:value(baudrate[i])
end
local com3_data_bits = comport:taboption("com3_setting", ListValue, "com3_data_bits", translate("Data bits"))
for i = 1,#alldatabits do
	com3_data_bits:value(alldatabits[i])
end
local com3_parity = comport:taboption("com3_setting", ListValue, "com3_parity", translate("Parity"))
for i = 1,#allparity do
	com3_parity:value(allparity[i])
end
local com3_stop_bits = comport:taboption("com3_setting", ListValue, "com3_stop_bits", translate("Stop bits"))
for i = 1,#allstopbits do
	com3_stop_bits:value(allstopbits[i])
end

comport:tab("com4_setting",  translate("COM4"))
local com4_baud = comport:taboption("com4_setting", ListValue, "com4_baud", translate("Baud rate"))
for i = 1,#baudrate do
	com4_baud:value(baudrate[i])
end
local com4_data_bits = comport:taboption("com4_setting", ListValue, "com4_data_bits", translate("Data bits"))
for i = 1,#alldatabits do
	com4_data_bits:value(alldatabits[i])
end
local com4_parity = comport:taboption("com4_setting", ListValue, "com4_parity", translate("Parity"))
for i = 1,#allparity do
	com4_parity:value(allparity[i])
end
local com4_stop_bits = comport:taboption("com4_setting", ListValue, "com4_stop_bits", translate("Stop bits"))
for i = 1,#allstopbits do
	com4_stop_bits:value(allstopbits[i])
end

-- show Software Version
local value = luci.sys.exec("/usr/home/dlg320.exe -v")
local str = string.format("uci set dlsetting.@swver[0].dlver=%s", value)
luci.sys.call(str)
value = luci.sys.exec("/usr/home/DataProgram.exe -v")
str = string.format("uci set dlsetting.@swver[0].dpver=%s", value)
luci.sys.call(str)
value = luci.sys.exec("/usr/home/FWupdate.exe -v")
str = string.format("uci set dlsetting.@swver[0].fuver=%s", value)
luci.sys.call(str)
value = luci.sys.exec("/usr/home/DLsocket.exe -v")
str = string.format("uci set dlsetting.@swver[0].dsver=%s", value)
luci.sys.call(str)
luci.sys.call("uci commit")

local swver = map:section(TypedSection, "swver", "SW VER")
swver.addremove = false
swver.anonymous = true
local dlver = swver:option(DummyValue, "dlver", translate("Data Logger VER"))
local dpver = swver:option(DummyValue, "dpver", translate("Data Program VER"))
local fuver = swver:option(DummyValue, "fuver", translate("FW Update VER"))
local fuver = swver:option(DummyValue, "dsver", translate("DL Socket VER"))

-- Add button part
local cleanreset = map:section(TypedSection, "button", "Clean and Reset")
cleanreset.addremove = false
cleanreset.anonymous = true
local clean = cleanreset:option(Button, "Clean White List", translate("Clean White List"))
clean.inputtitle = translate("Clean Execute")
clean.inputstyle = "apply"
function clean.write(self, section)
        --luci.sys.call("echo \"clean button push\" > /usr/home/clean_button") -- for test
		luci.sys.call("rm /usr/home/White-List.txt; sync;")
		luci.sys.call("rm /usr/home/White-List_V3.txt; sync;")
end
local reset = cleanreset:option(Button, "Reset DL setting", translate("Reset DL setting"))
reset.inputtitle = translate("Reset Execute")
reset.inputstyle = "apply"
function reset.write(self, section)
        --luci.sys.call("echo \"reset button push\" > /usr/home/reset_button") -- for test
		--luci.sys.call("cp /usr/home/G320_default.ini /usr/home/G320.ini; sync;")
		--luci.sys.call("cp /usr/home/dlsetting /etc/config/dlsetting; sync;")
		luci.sys.call("cp /usr/home/config/* /etc/config/; sync;")
		luci.sys.call("uci commit")
		luci.sys.call("cp /usr/home/ModelList_ini /usr/home/ModelList; sync;")
end

return map

