module("luci.controller.mqtt_sub_controller", package.seeall)

local db = luci.model.uci.cursor()

function index()
    entry({"admin", "services", "mqtt", "subscriber"}, firstchild(), _("Subscriber"), 3)
    entry({"admin", "services", "mqtt", "subscriber", "settings"}, cbi("subscriber_settings"), _("Subscriber settings"), 1)
    entry({"admin", "services", "mqtt", "subscriber", "events"}, cbi("subscriber_events"), _("Event settings"), 2)
    entry({"admin", "services", "mqtt", ""}, "", " ", 2)
end