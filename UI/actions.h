#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_load_settings(lv_event_t * e);
extern void action_load_main(lv_event_t * e);
extern void action_send_states(lv_event_t * e);
extern void action_sw_state_changed(lv_event_t * e);
extern void action_load_stats(lv_event_t * e);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/