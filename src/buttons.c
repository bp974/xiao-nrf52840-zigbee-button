#include <stddef.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

#include <zigbee/zigbee_app_utils.h>

#include "buttons.h"
#include "app_logging.h"

/* Tune these values after testing the physical switches and enclosure. */
#if CONFIG_XIAO_ZIGBEE_THREE_BUTTON
#define BUTTON_COUNT                3U
#else
#define BUTTON_COUNT                1U
#endif
#define BUTTON_DEBOUNCE_MS          30
#define DOUBLE_PRESS_WINDOW_MS      350
#define HOLD_TIME_MS                600

LOG_MODULE_REGISTER(buttons, XIAO_ZIGBEE_LOG_LEVEL);

struct button_context {
	uint32_t mask;
	struct k_work_delayable single_press_work;
	struct k_work_delayable hold_work;
	bool pressed;
	bool hold_triggered;
	uint8_t click_count;
	int64_t last_transition_ms;
};

static button_action_handler_t action_handler;
static struct button_context button_contexts[BUTTON_COUNT] = {
	[BUTTON_ID_1] = {.mask = DK_BTN1_MSK,
		.last_transition_ms = -BUTTON_DEBOUNCE_MS},
	[BUTTON_ID_2] = {.mask = DK_BTN2_MSK,
		.last_transition_ms = -BUTTON_DEBOUNCE_MS},
	[BUTTON_ID_3] = {.mask = DK_BTN3_MSK,
		.last_transition_ms = -BUTTON_DEBOUNCE_MS},
};

static void emit_action(struct button_context *context,
				enum button_action action)
{
	if (action_handler != NULL) {
		action_handler((enum button_id)(context - button_contexts), action);
	}
}

static void single_press_work_fn(struct k_work *work)
{
	struct button_context *context = CONTAINER_OF(
		k_work_delayable_from_work(work), struct button_context,
		single_press_work);

	if (context->click_count == 1U) {
		context->click_count = 0U;
		emit_action(context, BUTTON_ACTION_SINGLE);
	}
}

static void hold_work_fn(struct k_work *work)
{
	struct button_context *context = CONTAINER_OF(
		k_work_delayable_from_work(work), struct button_context,
		hold_work);

	if (context->pressed && !context->hold_triggered) {
		context->hold_triggered = true;
		context->click_count = 0U;
		(void)k_work_cancel_delayable(&context->single_press_work);
		emit_action(context, BUTTON_ACTION_HOLD);
	}
}

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	int64_t now = k_uptime_get();

	if ((has_changed & DK_ALL_BTNS_MSK) == 0U) {
		return;
	}

	/* Wake/notify the Zigbee stack immediately on physical button activity. */
	user_input_indicate();

	for (size_t i = 0; i < BUTTON_COUNT; i++) {
		struct button_context *context = &button_contexts[i];
		bool pressed;

		if ((has_changed & context->mask) == 0U) {
			continue;
		}
		if ((now - context->last_transition_ms) < BUTTON_DEBOUNCE_MS) {
			continue;
		}
		context->last_transition_ms = now;

		pressed = (button_state & context->mask) != 0U;
		if (pressed == context->pressed) {
			continue;
		}
		context->pressed = pressed;

		if (context->pressed) {
			k_work_reschedule(&context->hold_work, K_MSEC(HOLD_TIME_MS));
			continue;
		}

		(void)k_work_cancel_delayable(&context->hold_work);
		if (context->hold_triggered) {
			context->hold_triggered = false;
			context->click_count = 0U;
			(void)k_work_cancel_delayable(&context->single_press_work);
			emit_action(context, BUTTON_ACTION_RELEASE);
			continue;
		}

		if (context->click_count == 0U) {
			context->click_count = 1U;
			k_work_reschedule(&context->single_press_work,
					  K_MSEC(DOUBLE_PRESS_WINDOW_MS));
		} else {
			context->click_count = 0U;
			(void)k_work_cancel_delayable(&context->single_press_work);
			emit_action(context, BUTTON_ACTION_DOUBLE);
		}
	}
}

int buttons_init(button_action_handler_t handler)
{
	action_handler = handler;
	for (size_t i = 0; i < BUTTON_COUNT; i++) {
		k_work_init_delayable(&button_contexts[i].single_press_work,
				      single_press_work_fn);
		k_work_init_delayable(&button_contexts[i].hold_work,
				      hold_work_fn);
	}

	return dk_buttons_init(button_changed);
}
