#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *settings;
    lv_obj_t *stats;
    lv_obj_t *lbl_alarm_state;
    lv_obj_t *led_state;
    lv_obj_t *lbl_relay1;
    lv_obj_t *lbl_relay2;
    lv_obj_t *lbl_node;
    lv_obj_t *lbl_node_id;
    lv_obj_t *relay1_state;
    lv_obj_t *relay2_state;
    lv_obj_t *relay1_state_led;
    lv_obj_t *relay2_state_led;
    lv_obj_t *led_watchdog;
    lv_obj_t *sw_relay1;
    lv_obj_t *sw_relay2;
    lv_obj_t *lbl_relay1_1;
    lv_obj_t *lbl_relay1_2;
    lv_obj_t *lbl_node_1;
    lv_obj_t *lbl_node_id_1;
    lv_obj_t *lbl_retries;
    lv_obj_t *lbl_retry_count;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SETTINGS = 2,
    SCREEN_ID_STATS = 3,
};

void create_screen_main();
void tick_screen_main();

void create_screen_settings();
void tick_screen_settings();

void create_screen_stats();
void tick_screen_stats();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/