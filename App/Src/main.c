#include "app_state.h"
#include "bsp.h"
#include "display.h"

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

const char* menu_entries[NUM_ENTRIES] = { "Curva 1", "Curva 2", "Curva 3" };

static inline void write_menu_entry(uint8_t cursor_pos, uint16_t bg_color) {
    uint16_t y = FIRST_ENTRY + BOX_HEIGHT * (cursor_pos - 1) + MARGIN;
    BSP_Display_write_text(menu_entries[cursor_pos-1], 10, y, MENU_FONT, fg_color, bg_color);
}

static inline void fill_menu_entry(uint8_t at, uint16_t color) {
    uint16_t y = FIRST_ENTRY + BOX_HEIGHT * (at - 1);
    BSP_Display_fill_rect(0, y, SCREEN_WIDTH, BOX_HEIGHT, color);
}

int main(void) {
    BSP_Init();

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

        BSP_Delay(10);
    }
}

static AppState main_menu(uint8_t first_entry) {
    static uint8_t cursor_pos = 0;

    if (first_entry) {
        BSP_Display_clear(bg_color);
        BSP_Display_write_text("TEST", 10, 10, FONT3, RED, WHITE);

        fill_menu_entry(1, highlight_color);
        write_menu_entry(1, highlight_color);
        for (uint8_t i = 1; i < NUM_ENTRIES; ++i) {
            write_menu_entry(i + 1, bg_color);
        }
    }

    uint8_t next_cursor_pos = BSP_get_cursor(cursor_pos);
    if (next_cursor_pos != cursor_pos) {
        if (cursor_pos > 0 && cursor_pos < NUM_ENTRIES) {
            fill_menu_entry(cursor_pos, bg_color);
            write_menu_entry(cursor_pos, bg_color);
        }

        if (next_cursor_pos > 0 && next_cursor_pos < NUM_ENTRIES) {
            fill_menu_entry(cursor_pos, highlight_color);
            write_menu_entry(cursor_pos, highlight_color);
        }

        cursor_pos = next_cursor_pos;
    }

    if (BSP_ok_clicked()) {
        if (cursor_pos > 0 && cursor_pos < NUM_ENTRIES) {
            fill_menu_entry(cursor_pos, clicked_color);
            write_menu_entry(cursor_pos, clicked_color);
        }
        /* return (AppState) cursor_pos; */
    }

    return MAIN_MENU;
}

static AppState curve1(uint8_t first_entry) {
    return MAIN_MENU;
}

static AppState curve2(uint8_t first_entry) {
    return MAIN_MENU;
}

static AppState curve3(uint8_t first_entry) {
    return MAIN_MENU;
}
