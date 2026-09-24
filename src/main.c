#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <ram_pwrdn.h>

#include <zboss_api.h>
#include <zboss_api_addons.h>
#include <zigbee/zigbee_app_utils.h>
#include <zigbee/zigbee_error_handler.h>
#include <zb_nrf_platform.h>

#include "buttons.h"
#include "battery.h"
#include "zb_button_remote.h"
#include "zb_mem_config_custom.h"
#include "zigbee_actions.h"

#define ERASE_PERSISTENT_CONFIG ZB_FALSE

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

struct zb_device_ctx {
	zb_zcl_basic_attrs_ext_t basic_attr;
	zb_zcl_identify_attrs_t identify_attr;
};

static struct zb_device_ctx dev_ctx;
static zb_bool_t device_enable_basic_server_attr_list = ZB_TRUE;

ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(
	basic_server_attr_list, ZB_ZCL_BASIC)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID,
	&dev_ctx.basic_attr.zcl_version)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID,
	&dev_ctx.basic_attr.app_version)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_STACK_VERSION_ID,
	&dev_ctx.basic_attr.stack_version)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_HW_VERSION_ID,
	&dev_ctx.basic_attr.hw_version)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID,
	dev_ctx.basic_attr.mf_name)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID,
	dev_ctx.basic_attr.model_id)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_DATE_CODE_ID,
	dev_ctx.basic_attr.date_code)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID,
	&dev_ctx.basic_attr.power_source)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_SW_BUILD_ID,
	dev_ctx.basic_attr.sw_ver)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_DEVICE_ENABLED_ID,
	&device_enable_basic_server_attr_list)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_LOCATION_DESCRIPTION_ID,
	dev_ctx.basic_attr.location_id)
ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_PHYSICAL_ENVIRONMENT_ID,
	&dev_ctx.basic_attr.ph_env)
{
	BUTTON_REMOTE_EXACT_VOLTAGE_ATTR_ID,
	ZB_ZCL_ATTR_TYPE_U16,
	ZB_ZCL_ATTR_ACCESS_READ_ONLY | ZB_ZCL_ATTR_ACCESS_REPORTING,
	ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
	(void *)&battery_voltage_exact_attr
},
ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST;

ZB_ZCL_DECLARE_IDENTIFY_SERVER_ATTRIB_LIST(
	identify_server_attr_list,
	&dev_ctx.identify_attr.identify_time);

ZB_DECLARE_BUTTON_REMOTE_CLUSTER_LIST(
	button_remote_clusters,
	basic_server_attr_list,
	identify_server_attr_list,
	battery_power_config_attr_list);

static zb_zcl_reporting_info_t button_remote_reporting_info[
	BUTTON_REMOTE_REPORTING_SLOT_COUNT];

ZB_DECLARE_BUTTON_REMOTE_EP(button_remote_ep, button_remote_clusters);
ZBOSS_DECLARE_DEVICE_CTX_1_EP(button_remote_ctx, button_remote_ep);

static void app_clusters_attr_init(void)
{
	dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
	dev_ctx.basic_attr.app_version = 1;
	dev_ctx.basic_attr.stack_version = 1;
	dev_ctx.basic_attr.hw_version = 1;
	dev_ctx.basic_attr.date_code[0] = 0;
	dev_ctx.basic_attr.location_id[0] = 0;
	ZB_ZCL_SET_STRING_VAL(dev_ctx.basic_attr.mf_name,
		"Broskie Applications", ZB_ZCL_STRING_CONST_SIZE("Broskie Applications"));
	ZB_ZCL_SET_STRING_VAL(dev_ctx.basic_attr.model_id,
		"XIAO-Zigbee-Button", ZB_ZCL_STRING_CONST_SIZE("XIAO-Zigbee-Button"));
	ZB_ZCL_SET_STRING_VAL(dev_ctx.basic_attr.sw_ver,
		"1", ZB_ZCL_STRING_CONST_SIZE("1"));
	dev_ctx.basic_attr.power_source = ZB_ZCL_BASIC_POWER_SOURCE_BATTERY;
	dev_ctx.identify_attr.identify_time =
		ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;
}

void zboss_signal_handler(zb_bufid_t bufid)
{
	zb_zdo_app_signal_hdr_t *sig_hndler = NULL;
	zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &sig_hndler);
	zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);

	if (sig == ZB_COMMON_SIGNAL_CAN_SLEEP && status == RET_OK) {
		LOG_INF("Zigbee stack idle; entering sleep");
	}

	if ((sig == ZB_BDB_SIGNAL_DEVICE_REBOOT || sig == ZB_BDB_SIGNAL_STEERING) &&
	    status == RET_OK) {
		LOG_INF("Zigbee joined; starting battery measurement");
		battery_start();
	}

	ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));

	if (bufid) {
		zb_buf_free(bufid);
	}
}

int main(void)
{
	int err;

	LOG_INF("Starting XIAO Zigbee button");

	err = buttons_init(zigbee_actions_handle);
	if (err) {
		LOG_ERR("Button initialization failed: %d", err);
		return err;
	}

	err = battery_init();
	if (err) {
		LOG_ERR("Battery initialization failed: %d", err);
		return err;
	}

	zigbee_erase_persistent_storage(ERASE_PERSISTENT_CONFIG);
	zb_set_ed_timeout(ED_AGING_TIMEOUT_64MIN);
	zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(3000));

	ZB_AF_REGISTER_DEVICE_CTX(&button_remote_ctx);
	app_clusters_attr_init();

	/* Operate as a sleepy end device; ZBOSS will sleep when the stack is idle. */
	zigbee_configure_sleepy_behavior(true);

	/* Start the Zigbee thread; commissioning and rejoin are handled by ZBOSS. */
	zigbee_enable();

	if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
		power_down_unused_ram();
	}

	LOG_INF("XIAO Zigbee button started");
	return 0;
}
