#ifndef XIAO_BUTTONS_H
#define XIAO_BUTTONS_H

enum button_action {
	BUTTON_ACTION_SINGLE,
	BUTTON_ACTION_DOUBLE,
};

typedef void (*button_action_handler_t)(enum button_action action);

int buttons_init(button_action_handler_t handler);

#endif /* XIAO_BUTTONS_H */
