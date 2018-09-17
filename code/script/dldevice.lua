require("nixio")

DEVICE_LIST_PATH = "/tmp/DeviceList"

local form = SimpleForm("dldevice", translate("DL Device"), translate("This list gives an overview over currently device status."))
form.reset = false
form.submit = false

local mytab = {}

if nixio.fs.access(DEVICE_LIST_PATH) then
	for line in io.lines(DEVICE_LIST_PATH) do
		local addr = string.sub(line, 1, 3)		-- 1 ~ 253
		local sn = string.sub(line, 5, 20)		-- 16 bits
		local state = string.sub(line, 22, 22)	-- 0 : off line , 1 : on line
		local devtype							-- 0 : Unknown , 1 : MI , 2 : Hybrid
		local device = string.sub(line, 24, 24)
		if ( device == "0" ) then
			devtype = "Unknown"
		else if ( device == "1" ) then
				devtype = "MI"
			else
				devtype = "Hybrid"
			end
		end
		local energy_data_start = string.find(line, "ENERGY_START")
		local energy_data_end = string.find(line, "ENERGY_END")
		local energy_data
		if ( energy_data_start ~= nil ) then
			energy_data = string.sub(line, energy_data_start+12, energy_data_end-1)
		else
			energy_data = "Empty"
		end
		local error_data_start = string.find(line, "ERROR_START")
		local error_data_end = string.find(line, "ERROR_END")
		local error_data
		if ( error_data_start ~= nil ) then
			error_data = string.sub(line, error_data_start+11, error_data_end-1)
		else
			error_data = "Empty"
		end
		-- set table value
		mytab[#mytab+1] = {Address = addr, SN = sn, State = state, Device = devtype, ENERGY_DATA = energy_data, ERROR_DATA = error_data}
	end
else
		mytab = nil
end

local tab = form:section(Table, mytab)
tab:option(DummyValue, "Address", translate("Address"))
tab:option(DummyValue, "SN", translate("SN"))

--tab:option(DummyValue, "State", translate("State"))
local button = tab:option(Button, "State", translate("State"))
button.render = function(self, section, scope)
	if ( mytab[section].State == "1" ) then
		self.title = translate("On Line")
		self.inputstyle = "save"
	else
		self.title = translate("Off Line")
		self.inputstyle = "reset"
	end
	Button.render(self, section, scope)
end

button.write = function(self, section)
	-- do nothing
end

tab:option(DummyValue, "Device", translate("Device"))

tab:option(DummyValue, "ENERGY_DATA", translate("ENERGY DATA"))

tab:option(DummyValue, "ERROR_DATA", translate("ERROR DATA"))

return form

