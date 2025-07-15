include $(TOPDIR)/rules.mk

PKG_NAME:=tuyareporter
PKG_RELEASE:=1
PKG_VERSION:=1.0.2

include $(INCLUDE_DIR)/package.mk

define Package/tuyareporter
	CATEGORY:=Utilities
	TITLE:=A simple program that reports memory usage to Tuya IoT Cloud
	DEPENDS:=+libtuya +libubus +libubox +libblobmsg-json
endef

define Package/tuyareporter/description
	This is a library needed to communicate with the Tuya IoT cloud network
endef

define Package/tuyareporter/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tuyareporter $(1)/usr/bin
	$(INSTALL_BIN) ./files/tuyareporter.init $(1)/etc/init.d/tuyareporter
	$(INSTALL_BIN) ./files/tuyareporter.config $(1)/etc/config/tuyareporter
endef

$(eval $(call BuildPackage,tuyareporter))