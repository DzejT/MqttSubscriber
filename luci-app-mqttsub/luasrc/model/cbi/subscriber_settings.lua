map = Map("mqtt_sub", "Subsriber settings")

local certs = require "luci.model.certificate"
local certificates = certs.get_certificates()
local keys = certs.get_keys()
local cas = certs.get_ca_files().certs


s1 = map:section(NamedSection, "general", "general", "General")
o1_1 = s1:option(Flag, "enable", "Enable", "Enable subscriber")
o1_2 = s1:option(Value, "host", "Host", "Host ip")
o1_2.datatype = "ip4addr"
o1_2.placeholder = "192.168.1.1"
o1_3 = s1:option(Value, "port", "Port", "Port")
o1_3.placeholder = 1883

s2 = map:section(NamedSection, "security", "security", "TLS settings")

use_tls = s2:option(Flag, "use_tls", "Enable TLS", "Enable TLS")
use_tls.rmempty = false

tls_type = s2:option(ListValue, "tls_type", translate("TLS Type"), translate("Select the type of TLS encryption"))
tls_type:depends("use_tls", 1)
tls_type:value("cert", translate("Certificate based"))
tls_type:value("psk", translate("Pre-Shared-Key based"))


local certificates_link = luci.dispatcher.build_url("admin", "system", "admin", "certificates")
o = s2:option(Flag, "_device_sec_files", translate("Certificate files from device"),
	translatef("Choose this option if you want to select certificate files from device.\
	Certificate files can be generated <a class=link href=%s>%s</a>", certificates_link, translate("here")))
o:depends({use_tls='1', tls_type = "cert"})

ca_file = s2:option(FileUpload, "ca_file", translate("CA File"), translate("Upload CA file"));
ca_file:depends({use_tls='1',_device_sec_files="", tls_type = 'cert'})

cert_file = s2:option(FileUpload, "cert_file", translate("CERT File"), translate("Upload CERT file"));
cert_file:depends({use_tls='1',_device_sec_files="", tls_type = 'cert'})

key_file = s2:option(FileUpload, "key_file", translate("Key File"), translate("Upload Key file"));
key_file:depends({use_tls='1',_device_sec_files="", tls_type = 'cert'})

ca_file = s2:option(ListValue, "_device_ca_file", translate("CA File"), translate("Upload CA file"));
ca_file:depends({_device_sec_files = "1", tls_type = "cert"})

if #cas > 0 then
	for _,ca in pairs(cas) do
		ca_file:value("/etc/certificates/" .. ca.name, ca.name)
	end
else 
	ca_file:value("", translate("-- No file available --"))
end

function ca_file.write(self, section, value)
	map.uci:set(self.config, section, "ca_file", value)
end

ca_file.cfgvalue = function(self, section)
	return map.uci:get(map.config, section, "ca_file") or ""
end

cert_file = s2:option(ListValue, "_device_cert_file", translate("CERT File"), translate("Upload CERT file"));
cert_file:depends({_device_sec_files = "1", tls_type = "cert"})

if #certificates > 0 then
	for _,cert in pairs(certificates) do
		cert_file:value("/etc/certificates/" .. cert.name, cert.name)
	end
else 
	cert_file:value("", translate("-- No file available --"))
end

function cert_file.write(self, section, value)
	map.uci:set(self.config, section, "cert_file", value)
end

cert_file.cfgvalue = function(self, section)
	return map.uci:get(map.config, section, "cert_file") or ""
end

key_file = s2:option(ListValue, "_device_key_file", translate("Key File"), translate("Upload Key file"));
key_file:depends({_device_sec_files = "1", tls_type = "cert"})

if #keys > 0 then
	for _,key in pairs(keys) do
		key_file:value("/etc/certificates/" .. key.name, key.name)
	end
else 
	key_file:value("", translate("-- No file available --"))
end

function key_file.write(self, section, value)
	map.uci:set(self.config, section, "key_file", value)
end

key_file.cfgvalue = function(self, section)
	return map.uci:get(map.config, section, "key_file") or ""
end

o = s2:option(Value, "psk", "PSK", "Pre-Shared-Key")
o:depends("tls_type", "psk")
o = s2:option(Value, "identity", "Identity", "Identity")
o:depends("tls_type", "psk")



s3 = map:section(NamedSection, "user_info", "User information", "User information")
use_auth = s3:option(Flag, "use_auth", "Login using credentials", "Not neccesary if broker doesn't require username and password")
use_auth.rmempty = false
username = s3:option(Value, "username", "Username", "Enter your username, that will be used to login into the broker")
username:depends('use_auth', '1')
password = s3:option(Value, "password", "Password", "Enter your password, that will be used to login into the broker")
password:depends('use_auth', '1')


s4 = map:section(TypedSection, "interface", translate("Topics"), translate("This section is used to specify which topic to subscribe to. To add more topics to the list, click the 'Add' button"))
s4.anonymous = true
s4.addremove = true
s4.template = "cbi/tblsection"
s4.template_addremove = "cbi/add_rule"
s4.delete_alert = true
s4.alert_message = translate("Are you sure you want to delete this topic?")

o = s4:option(Value, "topic", translate("Topic"), translate("Topic"))
o.placeholder = "Example: router/id"
o.datatype = "string"
o.maxlength = 49


return map
