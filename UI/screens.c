#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            lv_obj_set_pos(obj, 10, 180);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, action_load_settings, LV_EVENT_RELEASED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Settings");
                }
            }
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            lv_obj_set_pos(obj, 212, 180);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, action_load_stats, LV_EVENT_RELEASED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Stats");
                }
            }
        }
        {
            // lblAlarmState
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_alarm_state = obj;
            lv_obj_set_pos(obj, 10, 62);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Alarm State");
        }
        {
            // ledState
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.led_state = obj;
            lv_obj_set_pos(obj, 169, 54);
            lv_obj_set_size(obj, 32, 32);
            lv_led_set_color(obj, lv_color_hex(0xffff0000));
            lv_led_set_brightness(obj, 255);
        }
        {
            // lblRelay1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_relay1 = obj;
            lv_obj_set_pos(obj, 10, 101);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Relay 1 State");
        }
        {
            // lblRelay2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_relay2 = obj;
            lv_obj_set_pos(obj, 10, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Relay 2 State");
        }
        {
            // lblNode
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_node = obj;
            lv_obj_set_pos(obj, 10, 20);
            lv_obj_set_size(obj, 39, 16);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Node");
        }
        {
            // lblNodeID
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_node_id = obj;
            lv_obj_set_pos(obj, 160, 20);
            lv_obj_set_size(obj, 41, 16);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Node");
        }
        {
            // relay1State
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.relay1_state = obj;
            lv_obj_set_pos(obj, 213, 104);
            lv_obj_set_size(obj, 87, 16);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Enabled");
        }
        {
            // relay2State
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.relay2_state = obj;
            lv_obj_set_pos(obj, 213, 146);
            lv_obj_set_size(obj, 87, 16);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Enabled");
        }
        {
            // relay1StateLed
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.relay1_state_led = obj;
            lv_obj_set_pos(obj, 169, 96);
            lv_obj_set_size(obj, 32, 32);
            lv_led_set_color(obj, lv_color_hex(0xffff0000));
            lv_led_set_brightness(obj, 255);
        }
        {
            // relay2StateLed
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.relay2_state_led = obj;
            lv_obj_set_pos(obj, 169, 138);
            lv_obj_set_size(obj, 32, 32);
            lv_led_set_color(obj, lv_color_hex(0xffff0000));
            lv_led_set_brightness(obj, 255);
        }
        {
            // ledWatchdog
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.led_watchdog = obj;
            lv_obj_set_pos(obj, 282, 20);
            lv_obj_set_size(obj, 18, 16);
            lv_led_set_color(obj, lv_color_hex(0xfffffc00));
            lv_led_set_brightness(obj, 255);
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
}

void create_screen_settings() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.settings = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            lv_obj_set_pos(obj, 10, 180);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, action_load_main, LV_EVENT_RELEASED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Main");
                }
            }
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            lv_obj_set_pos(obj, 212, 180);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, action_send_states, LV_EVENT_RELEASED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Send");
                }
            }
        }
        {
            // sw_relay1
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.sw_relay1 = obj;
            lv_obj_set_pos(obj, 212, 60);
            lv_obj_set_size(obj, 50, 25);
            lv_obj_add_event_cb(obj, action_sw_state_changed, LV_EVENT_VALUE_CHANGED, (void *)0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff02effa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // sw_relay2
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.sw_relay2 = obj;
            lv_obj_set_pos(obj, 212, 100);
            lv_obj_set_size(obj, 50, 25);
            lv_obj_add_event_cb(obj, action_sw_state_changed, LV_EVENT_VALUE_CHANGED, (void *)1);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff02effa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // lblNodeID_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_node_id_1 = obj;
            lv_obj_set_pos(obj, 212, 20);
            lv_obj_set_size(obj, 50, 16);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Node");
        }
        {
            // lblRelay1_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_relay1_1 = obj;
            lv_obj_set_pos(obj, 10, 60);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Relay 1 State");
        }
        {
            // lblRelay1_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_relay1_2 = obj;
            lv_obj_set_pos(obj, 10, 100);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Relay 2 State");
        }
        {
            // lblNode_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_node_1 = obj;
            lv_obj_set_pos(obj, 10, 20);
            lv_obj_set_size(obj, 39, 16);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Node");
        }
        {
            // sw_test
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.sw_test = obj;
            lv_obj_set_pos(obj, 213, 140);
            lv_obj_set_size(obj, 50, 25);
            lv_obj_add_event_cb(obj, action_sw_state_changed, LV_EVENT_VALUE_CHANGED, (void *)2);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff02effa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // lblRelay1_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_relay1_3 = obj;
            lv_obj_set_pos(obj, 11, 140);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Test");
        }
    }
    
    tick_screen_settings();
}

void tick_screen_settings() {
}

void create_screen_stats() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.stats = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            lv_obj_set_pos(obj, 212, 180);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, action_load_main, LV_EVENT_RELEASED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Main");
                }
            }
        }
        {
            // lblRetries
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_retries = obj;
            lv_obj_set_pos(obj, 10, 15);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Handshakes lost");
        }
        {
            // lblRetryCount
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_retry_count = obj;
            lv_obj_set_pos(obj, 196, 15);
            lv_obj_set_size(obj, 116, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-10");
        }
        {
            // lblRetries_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_retries_1 = obj;
            lv_obj_set_pos(obj, 10, 35);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "rssi");
        }
        {
            // lbl_rssi
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_rssi = obj;
            lv_obj_set_pos(obj, 196, 35);
            lv_obj_set_size(obj, 116, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "rssi");
        }
        {
            // lblRetries_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_retries_2 = obj;
            lv_obj_set_pos(obj, 10, 55);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Alarm activations");
        }
        {
            // lbl_activations
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_activations = obj;
            lv_obj_set_pos(obj, 196, 55);
            lv_obj_set_size(obj, 116, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "activations");
        }
    }
    
    tick_screen_stats();
}

void tick_screen_stats() {
}

void create_screen_saver() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.saver = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    lv_obj_add_event_cb(obj, action_load_last, LV_EVENT_RELEASED, (void *)0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    tick_screen_saver();
}

void tick_screen_saver() {
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_settings,
    tick_screen_stats,
    tick_screen_saver,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
    create_screen_settings();
    create_screen_stats();
    create_screen_saver();
}
