#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

#include <zigbee/zigbee_app_utils.h>

#include "buttons.h"

/* Tune these values after testing the physical switch and enclosure. */
#define BUTTON_MASK                 DK_BTN1_MSK
#define BUTTON_DEBOUNCE_MS          30
#define DOUBLE_PRESS_WINDOW_MS      350
#define HOLD_TIME_MS                600

LOG_MODULE_REGISTER(buttons, LOG_LEVEL_INF);

static button_action_handler_t action_handler;
static struct k_work_delayable single_press_work;
static struct k_work_delayable hold_work;
static bool button_pressed;
static bool hold_triggered;
static uint8_t click_count;
static int64_t last_transition_ms = -BUTTON_DEBOUNCE_MS;

static void emit_action(enum button_action action)
{
	if (action_handler != NULL) {
		action_handler(action);
	}
}

static void single_press_work_fn(struct k_work *work)
{
	ARG_UNUSED(work);

	if (click_count == 1U) {
		click_count = 0U;
		emit_action(BUTTON_ACTION_SINGLE);
	}
}

static void hold_work_fn(struct k_work *work)
{
	ARG_UNUSED(work);

	if (button_pressed && !hold_triggered) {
		hold_triggered = true;
		click_count = 0U;
		(void)k_work_cancel_delayable(&single_press_work);
		emit_action(BUTTON_ACTION_HOLD);
	}
}

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	int64_t now;
	bool pressed;

	if ((has_changed & BUTTON_MASK) == 0U) {
		return;
	}

	/* Wake/notify the Zigbee stack immediately on physical button activity. */
	user_input_indicate();

	now = k_uptime_get();
	if ((now - last_transition_ms) < BUTTON_DEBOUNCE_MS) {
		return;
	}
	last_transition_ms = now;

	pressed = (button_state & BUTTON_MASK) != 0U;
	if (pressed == button_pressed) {
		return;
	}
	button_pressed = pressed;

	if (button_pressed) {
		k_work_reschedule(&hold_work, K_MSEC(HOLD_TIME_MS));
		return;
	}

	(void)k_work_cancel_delayable(&hold_work);
	if (hold_triggered) {
		hold_triggered = false;
		click_count = 0U;
		(void)k_work_cancel_delayable(&single_press_work);
		emit_action(BUTTON_ACTION_RELEASE);
		return;
	}

	if (click_count == 0U) {
		click_count = 1U;
		k_work_reschedule(&single_press_work,
				  K_MSEC(DOUBLE_PRESS_WINDOW_MS));
	} else {
		click_count = 0U;
		(void)k_work_cancel_delayable(&single_press_work);
		emit_action(BUTTON_ACTION_DOUBLE);
	}
}

int buttons_init(button_action_handler_t handler)
{
	action_handler = handler;
	k_work_init_delayable(&single_press_work, single_press_work_fn);
	k_work_init_delayable(&hold_work, hold_work_fn);

	return dk_buttons_init(button_changed);
}
