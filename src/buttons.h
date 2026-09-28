#ifndef XIAO_BUTTONS_H
#define XIAO_BUTTONS_H

enum button_action {
	BUTTON_ACTION_SINGLE,
	BUTTON_ACTION_DOUBLE,
	BUTTON_ACTION_HOLD,
	BUTTON_ACTION_RELEASE,
};

enum button_id {
	BUTTON_ID_1,
	BUTTON_ID_2,
	BUTTON_ID_3,
};

typedef void (*button_action_handler_t)(enum button_id button,
						 enum button_action action);

int buttons_init(button_action_handler_t handler);

#endif /* XIAO_BUTTONS_H */
