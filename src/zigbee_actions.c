#include <zephyr/logging/log.h>

#include <zboss_api.h>
#include <zboss_api_addons.h>

#include "zigbee_actions.h"
#include "zb_button_remote.h"

LOG_MODULE_REGISTER(zigbee_actions, LOG_LEVEL_INF);

static zb_uint16_t coordinator_address = BUTTON_REMOTE_COORDINATOR;

static void send_toggle(zb_bufid_t bufid)
{
	ZB_ZCL_ON_OFF_SEND_REQ(bufid,
		coordinator_address,
		ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
		BUTTON_REMOTE_COORDINATOR_ENDPOINT,
		BUTTON_REMOTE_ENDPOINT,
		ZB_AF_HA_PROFILE_ID,
		ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
		ZB_ZCL_CMD_ON_OFF_TOGGLE_ID,
		NULL);
}

static void send_on(zb_bufid_t bufid)
{
	ZB_ZCL_ON_OFF_SEND_REQ(bufid,
		coordinator_address,
		ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
		BUTTON_REMOTE_COORDINATOR_ENDPOINT,
		BUTTON_REMOTE_ENDPOINT,
		ZB_AF_HA_PROFILE_ID,
		ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
		ZB_ZCL_CMD_ON_OFF_ON_ID,
		NULL);
}

static void send_hold(zb_bufid_t bufid)
{
	ZB_ZCL_LEVEL_CONTROL_SEND_MOVE_REQ(bufid,
		coordinator_address,
		ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
		BUTTON_REMOTE_COORDINATOR_ENDPOINT,
		BUTTON_REMOTE_ENDPOINT,
		ZB_AF_HA_PROFILE_ID,
		ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
		NULL,
		0,
		50);
}

static void send_release(zb_bufid_t bufid)
{
	ZB_ZCL_LEVEL_CONTROL_SEND_STOP_REQ(bufid,
		coordinator_address,
		ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
		BUTTON_REMOTE_COORDINATOR_ENDPOINT,
		BUTTON_REMOTE_ENDPOINT,
		ZB_AF_HA_PROFILE_ID,
		ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
		NULL);
}

static void schedule_command(zb_callback_t callback)
{
	zb_ret_t err = zb_buf_get_out_delayed(callback);

	if (err != RET_OK) {
		LOG_ERR("Unable to schedule Zigbee action: %d", err);
	}
}

void zigbee_actions_handle(enum button_action action)
{
	if (!ZB_JOINED()) {
		LOG_WRN("Ignoring button action while Zigbee is not joined");
		return;
	}

	switch (action) {
	case BUTTON_ACTION_SINGLE:
		LOG_INF("Button action: SINGLE (Toggle)");
		schedule_command(send_toggle);
		break;
	case BUTTON_ACTION_DOUBLE:
		LOG_INF("Button action: DOUBLE (On)");
		schedule_command(send_on);
		break;
	case BUTTON_ACTION_HOLD:
		LOG_INF("Button action: HOLD (Move Up)");
		schedule_command(send_hold);
		break;
	case BUTTON_ACTION_RELEASE:
		LOG_INF("Button action: RELEASE (Stop)");
		schedule_command(send_release);
		break;
	default:
		LOG_WRN("Unknown button action: %d", action);
		break;
	}
}
