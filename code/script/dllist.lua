require("luci.sys")
require("luci.config")
require("nixio")

MODEL_LIST_PATH = "/usr/home/ModelList"

-- setting part --------------------------------------------------------------------------------------------
local map = Map("dllist", translate("Model Setting"), translate("Data Logger Model List Setting"))

local list = map:section(TypedSection, "list", "Model setting")
list.addremove = false
list.anonymous = true

local slave_addr = list:option(Value, "slave_addr", translate("Slave Address  (Please input DEC positive integer [1 ~ 255])"))

local dev_id = list:option(Value, "dev_id", translate("Device ID  (Please input DEC positive integer [1 ~ 255])"))

-- set all model in this table -----
local allmodel = {"Darfon", "CyberPower-1P", "CyberPower-3P", "ADtek-CS1-T", "Test"}
------------------------------------
local inverter_model = list:option(ListValue, "model", translate("Model"))
for i = 1,#allmodel do
	inverter_model:value(allmodel[i])
end
local allport = {"COM1", "COM2", "COM3", "COM4"}
local inverter_port = list:option(ListValue, "port", translate("Port"))
for i = 1,#allport do
	inverter_port:value(allport[i])
end

--[[local set = list:option(Button, "Set Data", translate("Set Data"))
set.inputtitle = translate("Set Execute")
set.inputstyle = "apply"
function set.write(self, section)
	-- apply ==> set parameter to uci
end]]

local add = list:option(Button, "Add To List", translate("Add To List"))
add.inputtitle = translate("Add Execute")
add.inputstyle = "save"
function add.write(self, section)
	local tmpaddr = map.uci:get("dllist", "LIST", "slave_addr")
	local tmpdevid = map.uci:get("dllist", "LIST", "dev_id")
	local addrtonum = tonumber(tmpaddr)
	local devidtonum = tonumber(tmpdevid)
	if ( (addrtonum ~= nil) and (devidtonum ~= nil) ) then
		if ( (addrtonum >= 1) and (addrtonum <= 255 ) and (devidtonum >= 1) ) then
			local intaddr = math.floor(addrtonum)
			local intdevid = math.floor(devidtonum)
			-- commit to uci & save data to model list
			map.uci:set("dllist", "LIST", "slave_addr", intaddr)
			map.uci:set("dllist", "LIST", "dev_id", intdevid)
			map.uci:commit("dllist")
			local file = io.open(MODEL_LIST_PATH,"a")
			local comtmp = string.sub(map.uci:get("dllist", "LIST", "port"), 4, 4)
			local idtmp = (comtmp-1)*255 + intaddr - 1
			local str = string.format("%04d", idtmp) ..
					" Addr:" .. string.format("%03d", intaddr) ..
					" DEVID:" .. string.format("%d", intdevid) ..
					" Port:" .. map.uci:get("dllist", "LIST", "port") ..
					" Model:" .. map.uci:get("dllist", "LIST", "model") .. "\n"
			file:write(str)
			file:close()
			luci.sys.call("sync")
		else
			--do nothing
		end
	end
end

local sort = list:option(Button, "Sort Model List", translate("Sort Model List"))
sort.inputtitle = translate("Sort Execute")
sort.inputstyle = "save"
function sort.write(self, section)
	if nixio.fs.access(MODEL_LIST_PATH) then
		local sortcmd = "sort " .. MODEL_LIST_PATH .. " > /tmp/sort"
		luci.sys.call(sortcmd)
		sortcmd = "cp /tmp/sort " .. MODEL_LIST_PATH
		luci.sys.call(sortcmd)
		luci.sys.call("sync")
	end
end

local reload = list:option(Button, "Reload Model List", translate("Reload Model List"))
reload.inputtitle = translate("Reload Execute")
reload.inputstyle = "apply"
function reload.write(self, section)
	--do nothing, auto reload by luci system
	--[[luci.sys.call("grep slave_addr /etc/config/dllist | awk '{print $3}' > /tmp/dllistaddr")
	if nixio.fs.access("/tmp/dllistaddr") then
		local file = io.open("/tmp/dllistaddr","r")
		local tmparg = file:read()
		file:close()
		local tmpsetarg = string.sub(tmparg, 2, string.len(tmparg)-1)
		map.uci:set("dllist", "LIST", "slave_addr", tmpsetarg)

		local tmpstr = "echo \"" .. tmpsetarg .. "\" > /tmp/myteststr"
		luci.sys.call(tmpstr)
	end
	--map.uci:commit("dllist")
	]]
end

local clean = list:option(Button, "Clean Model List", translate("Clean Model List"))
clean.inputtitle = translate("Clean Execute")
clean.inputstyle = "reset"
function clean.write(self, section)
	local cleancmd = "rm " .. MODEL_LIST_PATH
	luci.sys.call(cleancmd)
	luci.sys.call("sync")
end

--[[local download = list:option(Button, "Download Model List", translate("Download Model List"))
download.inputtitle = translate("Download Execute")
download.inputstyle = "apply"
function download.write(self, section)
	local dlcmd = "/usr/home/DataProgram.exe -d &"
	luci.sys.exec(dlcmd)
end]]
------------------------------------------------------------------------------------------------------------


-- show list part ------------------------------------------------------------------------------------------
local form = SimpleForm("dllist", translate("Model List"), translate("This list gives an overview over currently model list."))
form.reset = false
form.submit = false

local mytab = {}

local num = 0
if nixio.fs.access(MODEL_LIST_PATH) then
	for line in io.lines(MODEL_LIST_PATH) do
		local addr = string.sub(line, 11, 13)			-- 001 ~ 255
		local devid_start = string.find(line, "DEVID:")
		local devid_end = string.find(line, " Port:")
		local devid
		if ( devid_start ~= nil ) then
			devid = string.sub(line, devid_start+6, devid_end-1)
			if ( string.len(devid) == 0 ) then
				devid = 0
			end
		else
			devid = 0
		end
		local port_start = string.find(line, "Port:")
		local port = string.sub(line, port_start+5, port_start+8)	-- COM1 ~ COM4
		local model_start = string.find(line, "Model:")
		local model = string.sub(line, model_start+6, string.len(line))	-- model from list
		num = num + 1 -- line number in list, set it to remove line in file when push delete button
		
		-- set table value
		mytab[#mytab+1] = {Address = addr, DEVID = devid, Port = port, Model = model, Num = tostring(num)}
	end
else
		mytab = nil
end

local tab = form:section(Table, mytab)
tab:option(DummyValue, "Address", translate("Address"))
tab:option(DummyValue, "DEVID", translate("Device ID"))
tab:option(DummyValue, "Port", translate("Port"))
tab:option(DummyValue, "Model", translate("Model"))
--tab:option(DummyValue, "Num", translate("Num"))
local delete = tab:option(Button, "Delete", translate("Delete"))
function delete.render(self, section, scope)
	self.title = translate("Delete")
	self.inputstyle = "reset"
	Button.render(self, section, scope)
end

function delete.write(self, section)
	local deletecmd = "sed -i '" .. mytab[section].Num .. "d' " ..  MODEL_LIST_PATH
	luci.sys.call(deletecmd)
	luci.sys.call("sync")
end
------------------------------------------------------------------------------------------------------------

return map, form
