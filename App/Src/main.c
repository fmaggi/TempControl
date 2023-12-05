#include "bsp.h"
#include "control.h"
#include "display.h"
#include "io.h"
#include "power.h"
#include "temperature.h"

#include <stdint.h>
#include <stdio.h>

typedef enum {
    MAIN_MENU = 0,
    CURVE1,
    CURVE2,
    CURVE3,
    LAST_STATE = CURVE3
} AppState;

static AppState main_menu(uint8_t first_entry);
static AppState curve1(uint8_t first_entry);
static AppState curve2(uint8_t first_entry);
static AppState curve3(uint8_t first_entry);

static uint16_t fg_color = BLACK;
static uint16_t bg_color = WHITE;
static uint16_t highlight_color = ORANGE;
static uint16_t clicked_color = BLUE;

#define MENU_FONT   FONT3
#define FIRST_ENTRY 40
#define BOX_HEIGHT  36
#define MARGIN      12

static inline void write_menu_entry(const char** menu, uint8_t at, uint16_t bg_color) {
    uint16_t y = FIRST_ENTRY + BOX_HEIGHT * at + MARGIN;
    BSP_Display_write_text(menu[at], 10, y, MENU_FONT, fg_color, bg_color);
}

static inline void fill_menu_entry(uint8_t at, uint16_t color) {
    uint16_t y = FIRST_ENTRY + BOX_HEIGHT * at;
    BSP_Display_fill_rect(0, y, SCREEN_WIDTH, BOX_HEIGHT, color);
}

int main(void) {
    BSP_init();

    AppState current_state = MAIN_MENU;
    AppState next_state = MAIN_MENU;

    // Make sure we run main_menu_setup on first run
    uint8_t changed_state = 1;

    while (1) {
        switch (current_state) {
            case MAIN_MENU: next_state = main_menu(changed_state); break;
            case CURVE1: next_state = curve1(changed_state); break;
            case CURVE2: next_state = curve2(changed_state); break;
            case CURVE3: next_state = curve3(changed_state); break;
        }

        changed_state = next_state != current_state;
        current_state = next_state;

        /* BSP_delay(10); */
    }
}

static AppState main_menu(uint8_t first_entry) {
    static uint8_t cursor_pos = 0;
    static const char* menu_entries[] = { "Test T", "Test Triac", "Test Potencia" };

    if (first_entry) {
        cursor_pos = BSP_IO_get_cursor(cursor_pos, LAST_STATE);
        BSP_Display_clear(bg_color);
        BSP_Display_write_text("MENU PRINCIPAL", 10, 10, FONT3, RED, WHITE);

        for (uint8_t i = 0; i < 3; ++i) {
            if (i == cursor_pos) {
                fill_menu_entry(i, highlight_color);
                write_menu_entry(menu_entries, i, highlight_color);
            } else {
                write_menu_entry(menu_entries, i, bg_color);
            }
        }
    }

    uint8_t next_cursor_pos = BSP_IO_get_cursor(cursor_pos, LAST_STATE);
    if (next_cursor_pos != cursor_pos) {
        fill_menu_entry(cursor_pos, bg_color);
        write_menu_entry(menu_entries, cursor_pos, bg_color);

        fill_menu_entry(next_cursor_pos, highlight_color);
        write_menu_entry(menu_entries, next_cursor_pos, highlight_color);

        cursor_pos = next_cursor_pos;
    }

    if (BSP_IO_ok_clicked()) {
        fill_menu_entry(cursor_pos, clicked_color);
        write_menu_entry(menu_entries, cursor_pos, clicked_color);
        return (AppState) (cursor_pos + 1);
    }

    return MAIN_MENU;
}

static AppState curve1(uint8_t first_entry) {
    static uint32_t last_t = 0;
    if (first_entry) {
        BSP_Display_clear(bg_color);
        BSP_Display_write_text("TEST1 - Meidicion de T", 10, 10, FONT3, RED, WHITE);
        BSP_Display_write_text("T(C)=", 95, 150, FONT3, fg_color, bg_color);
        BSP_T_start();
    }

    uint32_t current_t = Oven_temperature();
    if (current_t != last_t) {
        last_t = current_t;
        char tbuf[3] = { 0 };
        sprintf(tbuf, "%d", current_t);
        BSP_Display_write_text("999", 136, 150, FONT3, bg_color, bg_color);
        BSP_Display_write_text(tbuf, 136, 150, FONT3, fg_color, bg_color);
    }

    if (BSP_IO_ok_clicked()) {
        BSP_T_stop();
        return MAIN_MENU;
    }

    return CURVE1;
}

static AppState curve2(uint8_t first_entry) {
    return MAIN_MENU;
}

static AppState curve3(uint8_t first_entry) {
    static uint8_t cursor_pos = 0;
    static uint32_t last_t = 0;
    static uint32_t last_power = 0;
    static uint32_t current_target = 50;
    static char buf[20] = "Set point(C)=50";
    static const char* menu_entries[] = { buf, "< Volver" };

    static enum {
        NORMAL,
        CHANGE_TARGET
    } state = NORMAL;

    if (first_entry) {
        state = NORMAL;
        current_target = 50;
        BSP_IO_set_rotary(50);
        sprintf(buf, "Set point(C)=50");
        BSP_Display_clear(bg_color);
        BSP_Display_write_text("TEST3 - Potencia", 10, 10, FONT3, RED, WHITE);
        BSP_Display_write_text("T(C)=", 95, 150, FONT3, fg_color, bg_color);

        for (uint8_t i = 0; i < 2; ++i) {
            if (i == cursor_pos) {
                fill_menu_entry(i, highlight_color);
                write_menu_entry(menu_entries, i, highlight_color);
            } else {
                write_menu_entry(menu_entries, i, bg_color);
            }
        }

        Oven_set_target(50);
        Oven_start();
    }
    
    uint32_t current_t = Oven_temperature();
    if (current_t != last_t) {
        last_t = current_t;
        char tbuf[3] = { 0 };
        sprintf(tbuf, "%d", current_t);
        BSP_Display_write_text("999", 136, 150, FONT3, bg_color, bg_color);
        BSP_Display_write_text(tbuf, 136, 150, FONT3, fg_color, bg_color);
    }

    struct PowerState power_state = BSP_Power_get();
    char pbuf[20] = { 0 };
    sprintf(pbuf, "%d %d %d", power_state.power, power_state.period1, power_state.period2);
    BSP_Display_write_text("AAAAAAAAAAAAAAAAAAAAAAAAaAAAx", 36, 190, FONT3, bg_color, bg_color);
    BSP_Display_write_text(pbuf, 36, 190, FONT3, fg_color, bg_color);

    switch (state) {
        case NORMAL: {
            uint8_t next_cursor_pos = BSP_IO_get_cursor(cursor_pos, 2);
            if (next_cursor_pos != cursor_pos) {
                fill_menu_entry(cursor_pos, bg_color);
                write_menu_entry(menu_entries, cursor_pos, bg_color);

                fill_menu_entry(next_cursor_pos, highlight_color);
                write_menu_entry(menu_entries, next_cursor_pos, highlight_color);

                cursor_pos = next_cursor_pos;
            }

            if (BSP_IO_ok_clicked()) {
                fill_menu_entry(cursor_pos, clicked_color);
                write_menu_entry(menu_entries, cursor_pos, clicked_color);
                switch (cursor_pos) {
                    case 0: {
                        state = CHANGE_TARGET;
                        break;
                    }
                    case 1: {
                        Oven_stop();
                        return MAIN_MENU;
                    }
                }
            }
            break;
        }
        case CHANGE_TARGET: {
            uint32_t target = BSP_IO_get_rotary(0, 100);
            if (target != current_target) {
                current_target = target;
                sprintf(buf, "Set point(C)=%d", target);
                BSP_Display_write_text("999", 80, FIRST_ENTRY + MARGIN, MENU_FONT, clicked_color, clicked_color);
                write_menu_entry(menu_entries, 0, clicked_color);
            }

            if (BSP_IO_ok_clicked()) {
                Oven_set_target(target);
                fill_menu_entry(0, highlight_color);
                write_menu_entry(menu_entries, 0, highlight_color);
                state = NORMAL;
            }
            break;
        };
    };

    return CURVE3;
}
