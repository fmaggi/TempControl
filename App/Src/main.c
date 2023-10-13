#include "app_state.h"
#include "bsp.h"

#include <stdint.h>
#include <stdio.h>

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

#define NUM_ENTRIES 3

const char* menu_entries[NUM_ENTRIES] = { "Test T", "Test Triac", "?????" };

static inline void write_menu_entry(uint8_t cursor_pos, uint16_t bg_color) {
    uint16_t y = FIRST_ENTRY + BOX_HEIGHT * cursor_pos + MARGIN;
    BSP_Display_write_text(menu_entries[cursor_pos], 10, y, MENU_FONT, fg_color, bg_color);
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

    if (first_entry) {
        BSP_Display_clear(bg_color);
        BSP_Display_write_text("MENU PRINCIPAL", 10, 10, FONT3, RED, WHITE);

        for (uint8_t i = 0; i < NUM_ENTRIES; ++i) {
            if (i == cursor_pos) {
                fill_menu_entry(0, highlight_color);
                write_menu_entry(0, highlight_color);
            } else {
                write_menu_entry(i, bg_color);
            }
        }
    }

    uint8_t next_cursor_pos = BSP_IO_get_cursor(cursor_pos);
    if (next_cursor_pos != cursor_pos) {
        fill_menu_entry(cursor_pos, bg_color);
        write_menu_entry(cursor_pos, bg_color);

        fill_menu_entry(next_cursor_pos, highlight_color);
        write_menu_entry(next_cursor_pos, highlight_color);

        cursor_pos = next_cursor_pos;
    }

    if (BSP_IO_ok_clicked()) {
        fill_menu_entry(cursor_pos, clicked_color);
        write_menu_entry(cursor_pos, clicked_color);
        return (AppState) (cursor_pos + 1);
    }

    return MAIN_MENU;
}

static AppState curve1(uint8_t first_entry) {
    if (first_entry) {
        BSP_Display_clear(bg_color);
        BSP_Display_write_text("TEST1 - Meidicion de T", 10, 10, FONT3, RED, WHITE);
        BSP_T_start();
    }

    static uint32_t last_t = 0;
    uint32_t t = BSP_T_get()[0];
    char buf[10] = { 0 };
    if (t != last_t) {
        static uint32_t last_time = 0;
        uint32_t time = BSP_millis();
        sprintf(buf, "dt=%d", time - last_time);
        last_time = time;
        BSP_Display_write_text("AAAAAAAAAA", 100, 200, FONT3, bg_color, bg_color);
        BSP_Display_write_text(buf, 100, 200, FONT3, fg_color, bg_color);

        sprintf(buf, "T=%d", t);
        BSP_Display_write_text("AAAAAAAAAA", 100, 150, FONT3, bg_color, bg_color);
        BSP_Display_write_text(buf, 100, 150, FONT3, fg_color, bg_color);
    }

    if (BSP_IO_ok_clicked()) {
        BSP_T_stop();
        return MAIN_MENU;
    }

    return CURVE1;
}

static AppState curve2(uint8_t first_entry) {
    if (first_entry) {
        BSP_Display_clear(bg_color);
        BSP_Display_write_text("TEST2 - Disparo de Triac", 10, 10, FONT3, RED, WHITE);
        BSP_T_start();
        BSP_Power_start();
    }

    uint32_t t = BSP_T_get()[0];
    BSP_Power_set(t);

    if (BSP_IO_ok_clicked()) {
        BSP_Power_stop();
        BSP_T_stop();
        return MAIN_MENU;
    }

    return CURVE2;
}

static AppState curve3(uint8_t first_entry) {
    (void) first_entry;
    Error_Handler("curve3 not implemented");
    return CURVE3;
}
