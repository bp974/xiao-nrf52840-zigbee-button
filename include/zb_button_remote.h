#ifndef ZB_BUTTON_REMOTE_H
#define ZB_BUTTON_REMOTE_H

/* Endpoint and command destination match the ESP/Zigbee2MQTT implementation. */
#define BUTTON_REMOTE_ENDPOINT 10
#define BUTTON_REMOTE_COORDINATOR 0x0000
#define BUTTON_REMOTE_COORDINATOR_ENDPOINT 1

#define BUTTON_REMOTE_IN_CLUSTER_NUM 3
#define BUTTON_REMOTE_OUT_CLUSTER_NUM 2
#define BUTTON_REMOTE_POWER_CONFIG_ATTR_COUNT 7

#define ZB_DECLARE_BUTTON_REMOTE_CLUSTER_LIST( \
		cluster_list_name, basic_attr_list, identify_attr_list, power_config_attr_list) \
zb_zcl_cluster_desc_t cluster_list_name[] = { \
	ZB_ZCL_CLUSTER_DESC( \
		ZB_ZCL_CLUSTER_ID_BASIC, \
		ZB_ZCL_ARRAY_SIZE(basic_attr_list, zb_zcl_attr_t), \
		(basic_attr_list), \
		ZB_ZCL_CLUSTER_SERVER_ROLE, \
		ZB_ZCL_MANUF_CODE_INVALID), \
	ZB_ZCL_CLUSTER_DESC( \
		ZB_ZCL_CLUSTER_ID_IDENTIFY, \
		ZB_ZCL_ARRAY_SIZE(identify_attr_list, zb_zcl_attr_t), \
		(identify_attr_list), \
		ZB_ZCL_CLUSTER_SERVER_ROLE, \
		ZB_ZCL_MANUF_CODE_INVALID), \
	ZB_ZCL_CLUSTER_DESC( \
		ZB_ZCL_CLUSTER_ID_POWER_CONFIG, \
		BUTTON_REMOTE_POWER_CONFIG_ATTR_COUNT, \
		(power_config_attr_list), \
		ZB_ZCL_CLUSTER_SERVER_ROLE, \
		ZB_ZCL_MANUF_CODE_INVALID), \
	ZB_ZCL_CLUSTER_DESC( \
		ZB_ZCL_CLUSTER_ID_ON_OFF, \
		0, NULL, \
		ZB_ZCL_CLUSTER_CLIENT_ROLE, \
		ZB_ZCL_MANUF_CODE_INVALID), \
	ZB_ZCL_CLUSTER_DESC( \
		ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL, \
		0, NULL, \
		ZB_ZCL_CLUSTER_CLIENT_ROLE, \
		ZB_ZCL_MANUF_CODE_INVALID) \
}

#define ZB_DECLARE_BUTTON_REMOTE_EP(ep_name, cluster_list) \
	ZB_DECLARE_SIMPLE_DESC(3, 2); \
	ZB_AF_SIMPLE_DESC_TYPE(3, 2) simple_desc_##ep_name = { \
		BUTTON_REMOTE_ENDPOINT, \
		ZB_AF_HA_PROFILE_ID, \
		0x0009, \
		0, \
		0, \
		BUTTON_REMOTE_IN_CLUSTER_NUM, \
		BUTTON_REMOTE_OUT_CLUSTER_NUM, \
		{ ZB_ZCL_CLUSTER_ID_BASIC, ZB_ZCL_CLUSTER_ID_IDENTIFY, \
		  ZB_ZCL_CLUSTER_ID_POWER_CONFIG, \
		  ZB_ZCL_CLUSTER_ID_ON_OFF, ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL } \
	}; \
	ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, BUTTON_REMOTE_ENDPOINT, \
		ZB_AF_HA_PROFILE_ID, 0, NULL, \
		ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t), \
		cluster_list, (zb_af_simple_desc_1_1_t *)&simple_desc_##ep_name, \
		0, NULL, 0, NULL)

#endif /* ZB_BUTTON_REMOTE_H */
