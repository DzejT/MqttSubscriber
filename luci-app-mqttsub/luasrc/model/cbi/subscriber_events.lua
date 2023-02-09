map = Map("mqtt_sub", "Events")
local x

local uci = require "uci"
local cursor = luci.model.uci.cursor()
local options = cursor:get_all("mqtt_sub", "interface", "topic")

-- local options = uci:get("mqtt_sub", "event", "recipient")

s4 = map:section(TypedSection, "event", translate("Events"), translate(
    " Thissection is used to specify which events. To add more events to the list, click the 'Add' button"))
s4.anonymous = true
s4.addremove = true
s4.template = "cbi/tblsection"
s4.template_addremove = "cbi/add_rule"
s4.delete_alert = true
s4.alert_message = translate("Are you sure you want to delete this event?")

-- options = {">", "<", ">=", "<=", "==", "!="}

o = s4:option(ListValue, "topic", translate("Topic"), translate("Topic"))
map.uci:foreach("mqtt_sub", "interface", function(i)
    o:value(i.topic, i.topic)
end)
o.datatype = "string"
function o.validate(self, value, section)
    if value == nil or value == "" then
        return nil, "Error: Value can not be empty"
    end
    return value
end

o = s4:option(ListValue, "comparator", translate("Comparison sign"), translate("Comparison sign"))
o:value(">", ">")
o:value("<", "<")
o:value(">=", ">=")
o:value("<=", "<=")
o:value("==", "==")
o:value("!=", "!=")
o.datatype = "string"

o = s4:option(Value, "parameter", translate("parameter's name"), translate("The name of the parameter whose value will be checked"))
o.datatype = "string"

type = s4:option(ListValue, "type", translate("Variable type"), translate("Numerical / Alphabetical"))
type:value("num", "Numeric")
type:value("alph", "Alphanumeric")
type.datatype = "string"

o = s4:option(Value, "value", translate("Value to compare"), translate("Value to compare"))
o.datatype = "string"
if type == "num" then
    o.datatype = "integer"
end
o.maxlength = 49
function o.validate(self, value, section)
    if value == nil or value == "" then
        return nil, "Error: Value can not be empty"
    end
    return value
end

o = s4:option(Value, "sender", translate("Sender's email address"), translate("Sender's email address"))
o.datatype = "string"
o.maxlength = 49
function o.validate(self, value, section)
    if value == nil or value == "" then
        return nil, "Error: Value can not be empty"
    end
    return value
end


o = s4:option(Value, "password", translate("Sender's email password"), translate("Sender's email password"))
o.datatype = "string"
o.maxlength = 49

o = s4:option(Value, "reciever", translate("Recipient"), translate("Recipient"))
-- o.anonymous = true
-- o.addremove = true
-- o.template = "cbi/tblsection"
-- o.template_addremove = "cbi/add_rule"
-- o.delete_alert = true
-- o.alert_message = translate("Are you sure you want to delete this event?")
o.datatype = "string"
o.maxlength = 49

function o.validate(self, value, section)
    if value == nil or value == "" then
        return nil, "Error: Value can not be empty"
    end
    return value
end

return map
