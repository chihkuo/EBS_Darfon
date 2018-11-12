require("nixio")

DEVICE_LIST_PATH = "/tmp/DeviceList"

local form = SimpleForm("dldevice", translate("DL Device"), translate("This list gives an overview over currently device status."))
form.reset = false
form.submit = false

local mytab = {}

if nixio.fs.access(DEVICE_LIST_PATH) then
	for line in io.lines(DEVICE_LIST_PATH) do
		-- get SN
		local addr = string.sub(line, 1, 3)		-- 1 ~ 253
		local sn_start = string.find(line, "<SN>")
		local sn_end = string.find(line, "</SN>")
		local sn
		if ( sn_start ~= nil ) then
			sn = string.sub(line, sn_start+4, sn_end-1)
			if ( string.len(sn) == 0 ) then
				sn = "Empty"
			end
		else
			sn = "Empty"
		end
		-- get STATE
		local state_start = string.find(line, "<STATE>")
		local state_end = string.find(line, "</STATE>")
		local state
		if ( state_start ~= nil ) then
			state = string.sub(line, state_start+7, state_end-1)
		else
			state = "Empty"
		end
		-- get DEVICE
		local device_start = string.find(line, "<DEVICE>")
		local device_end = string.find(line, "</DEVICE>")
		local device
		if ( device_start ~= nil ) then
			device = string.sub(line, device_start+8, device_end-1)
		else
			device = "Empty"
		end
		-- get ENERGY
		local energy_data_start = string.find(line, "<ENERGY>")
		local energy_data_end = string.find(line, "</ENERGY>")
		local energy_data
		if ( energy_data_start ~= nil ) then
			energy_data = string.sub(line, energy_data_start+8, energy_data_end-1)
		else
			energy_data = "Empty"
		end
		-- get ERROR
		local error_data_start = string.find(line, "<ERROR>")
		local error_data_end = string.find(line, "</ERROR>")
		local error_data
		if ( error_data_start ~= nil ) then
			error_data = string.sub(line, error_data_start+7, error_data_end-1)
		else
			error_data = "Empty"
		end
		-- set table value
		mytab[#mytab+1] = {Address = addr, SN = sn, State = state, Device = device, ENERGY_DATA = energy_data, ERROR_DATA = error_data}
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

