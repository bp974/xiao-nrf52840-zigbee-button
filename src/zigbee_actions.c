#include <zephyr/logging/log.h>

#include "zigbee_actions.h"

LOG_MODULE_REGISTER(zigbee_actions, LOG_LEVEL_INF);

void zigbee_actions_handle(enum button_action action)
{
	switch (action) {
	case BUTTON_ACTION_SINGLE:
		LOG_INF("Button action: SINGLE");
		break;
	case BUTTON_ACTION_DOUBLE:
		LOG_INF("Button action: DOUBLE");
		break;
	default:
		LOG_WRN("Unknown button action: %d", action);
		break;
	}
}
