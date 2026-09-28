#include <zephyr/logging/log.h>

#include <zboss_api.h>
#include <zboss_api_addons.h>

#include "zigbee_actions.h"
#include "zb_button_remote.h"
#include "app_logging.h"

LOG_MODULE_REGISTER(zigbee_actions, XIAO_ZIGBEE_LOG_LEVEL);

static zb_uint16_t coordinator_address = BUTTON_REMOTE_COORDINATOR;

static void send_toggle(zb_bufid_t bufid, zb_uint16_t source_endpoint)
{
	ZB_ZCL_ON_OFF_SEND_REQ(bufid,
		coordinator_address,
		ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
		BUTTON_REMOTE_COORDINATOR_ENDPOINT,
		source_endpoint,
		ZB_AF_HA_PROFILE_ID,
		ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
		ZB_ZCL_CMD_ON_OFF_TOGGLE_ID,
		NULL);
}

static void send_on(zb_bufid_t bufid, zb_uint16_t source_endpoint)
{
	ZB_ZCL_ON_OFF_SEND_REQ(bufid,
		coordinator_address,
		ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
		BUTTON_REMOTE_COORDINATOR_ENDPOINT,
		source_endpoint,
		ZB_AF_HA_PROFILE_ID,
		ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
		ZB_ZCL_CMD_ON_OFF_ON_ID,
		NULL);
}

static void send_hold(zb_bufid_t bufid, zb_uint16_t source_endpoint)
{
	ZB_ZCL_LEVEL_CONTROL_SEND_MOVE_REQ(bufid,
		coordinator_address,
		ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
		BUTTON_REMOTE_COORDINATOR_ENDPOINT,
		source_endpoint,
		ZB_AF_HA_PROFILE_ID,
		ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
		NULL,
		0,
		50);
}

static void send_release(zb_bufid_t bufid, zb_uint16_t source_endpoint)
{
	ZB_ZCL_LEVEL_CONTROL_SEND_STOP_REQ(bufid,
		coordinator_address,
		ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
		BUTTON_REMOTE_COORDINATOR_ENDPOINT,
		source_endpoint,
		ZB_AF_HA_PROFILE_ID,
		ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
		NULL);
}

static void schedule_command(zb_callback2_t callback, zb_uint16_t source_endpoint)
{
	zb_ret_t err = zb_buf_get_out_delayed_ext(callback, source_endpoint, 0);

	if (err != RET_OK) {
		LOG_ERR("Unable to schedule Zigbee action: %d", err);
	}
}

void zigbee_actions_handle(enum button_id button, enum button_action action)
{
	zb_uint16_t source_endpoint = BUTTON_REMOTE_ENDPOINT + button;
	const char *button_name = button == BUTTON_ID_1 ? "1" :
		button == BUTTON_ID_2 ? "2" : "3";

	if (!ZB_JOINED()) {
		LOG_WRN("Ignoring button action while Zigbee is not joined");
		return;
	}

	switch (action) {
	case BUTTON_ACTION_SINGLE:
		LOG_INF("Button %s action: SINGLE (Toggle)", button_name);
		schedule_command(send_toggle, source_endpoint);
		break;
	case BUTTON_ACTION_DOUBLE:
		LOG_INF("Button %s action: DOUBLE (On)", button_name);
		schedule_command(send_on, source_endpoint);
		break;
	case BUTTON_ACTION_HOLD:
		LOG_INF("Button %s action: HOLD (Move Up)", button_name);
		schedule_command(send_hold, source_endpoint);
		break;
	case BUTTON_ACTION_RELEASE:
		LOG_INF("Button %s action: RELEASE (Stop)", button_name);
		schedule_command(send_release, source_endpoint);
		break;
	default:
		LOG_WRN("Unknown button action: %d", action);
		break;
	}
}
