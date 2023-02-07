include $(TOPDIR)/rules.mk

PKG_NAME:=mqtt_subscriber
PKG_RELEASE:=1
PKG_VERSION:=1.1.59
include $(INCLUDE_DIR)/package.mk

define Package/mqtt_subscriber
	CATEGORY:=Base system
	TITLE:=mqtt_subscriber
	DEPENDS:=+libmosquitto +libuci +libcurl +libjson-c
endef


# define Build/InstallDev
# 	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/wiotp/sdk/iotp_device.h $(STAGING_DIR)/usr/include
# endef

define Package/mqtt_subscriber/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mqtt_sub $(1)/usr/bin
	$(INSTALL_BIN) ./files/mqttsub.init $(1)/etc/init.d/mqtt_sub
	$(INSTALL_CONF) ./files/mqtt_sub.config $(1)/etc/config/mqtt_sub
endef

$(eval $(call BuildPackage,mqtt_subscriber))
