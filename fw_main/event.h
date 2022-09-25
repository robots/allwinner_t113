#ifndef EVENT_h_
#define EVENT_h_

typedef struct {
	uint8_t is_modifier;
	uint8_t down;
	uint8_t code;
	uint8_t ch;
} kbd_event_t;

extern QueueHandle_t kbd_queue;


#endif
