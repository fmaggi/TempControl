#include "ui.h"

#include "bsp.h"
#include "display.h"
#include "io.h"
#include "string.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>

#define STARTX 10
#define STARTY

static inline void write_menu_entry(const char** ui, uint8_t at, uint16_t bg) {
    uint16_t y = FIRST_ENTRY + BOX_HEIGHT * at + MARGIN;
    BSP_Display_write_text(ui[at], STARTX, y, MENU_FONT, FG_COLOR, bg);
}

static inline void fill_menu_entry(uint8_t at, uint16_t color) {
    uint16_t y = FIRST_ENTRY + BOX_HEIGHT * at;
    BSP_Display_fill_rect(0, y, SCREEN_WIDTH, BOX_HEIGHT, color);
}

void UI_Enter(struct UI* ui, const char* title) {
    UI_Reset(ui);
    UI_Clear();
    UI_Write_Title(title);
    UI_Write(ui);
}

void UI_Clear(void) {
    BSP_Display_clear(BG_COLOR);
}

void UI_Write_Title(const char* title) {
    BSP_Display_write_text(title, STARTX, 10, FONT3, RED, WHITE);
}

void UI_Reset(struct UI* ui) {
    BSP_IO_set_rotary(0);
    ui->pos = 0;
    ui->selected = UI_UNSELECTED;
}

void UI_Write(const struct UI* ui) {
    for (uint8_t i = 0; i < ui->num; ++i) {
        if (i == ui->pos) {
            fill_menu_entry(i, HL_COLOR);
            write_menu_entry(ui->entries, i, HL_COLOR);
        } else {
            write_menu_entry(ui->entries, i, BG_COLOR);
        }
    }
}

void UI_Update_entry(const struct UI* ui, uint8_t entry, uint8_t from) {
    uint16_t color;
    if (entry == ui->selected) {
        color = OK_COLOR;
    } else if (entry == ui->pos) {
        color = HL_COLOR;
    } else {
        color = BG_COLOR;
    }

    uint8_t xOffset = 0;
    const char* str = ui->entries[entry];
    uint8_t fOffset = MENU_FONT[0]; /* Offset of character */
    uint8_t fWidth = MENU_FONT[1];
    for (int i = 0; i < from; ++i) {
        uint8_t* tempChar = (uint8_t*) &MENU_FONT[((*str - 0x20) * fOffset) + 4];
        uint8_t cWidth = tempChar[0] + 2;
        xOffset += cWidth < fWidth ? cWidth : fWidth;
        str++;
    }

    uint16_t y = FIRST_ENTRY + BOX_HEIGHT * entry + MARGIN;

    BSP_Display_fill_rect(xOffset + 10, y, 80, MENU_FONT[2], color);
    BSP_Display_write_text(ui->entries[entry] + from, STARTX + xOffset, y, MENU_FONT, FG_COLOR, color);
}

void UI_Move_highlight(struct UI* ui, uint8_t to) {
    fill_menu_entry(ui->pos, BG_COLOR);
    write_menu_entry(ui->entries, ui->pos, BG_COLOR);

    fill_menu_entry(to, HL_COLOR);
    write_menu_entry(ui->entries, to, HL_COLOR);
}

uint8_t UI_Select(struct UI* ui) {
    if (ui->selected != UI_UNSELECTED) {
        return 0;
    }

    uint8_t ok = BSP_IO_ok_clicked();
    if (ok) {
        fill_menu_entry(ui->pos, OK_COLOR);
        write_menu_entry(ui->entries, ui->pos, OK_COLOR);
        ui->selected = ui->pos;
    }
    return ok;
}

uint8_t UI_Unselect(struct UI* ui) {
    if (ui->selected == UI_UNSELECTED) {
        return 0;
    }

    uint8_t ok = BSP_IO_ok_clicked();
    if (ok) {
        fill_menu_entry(ui->pos, HL_COLOR);
        write_menu_entry(ui->entries, ui->pos, HL_COLOR);
        ui->selected = UI_UNSELECTED;

        BSP_IO_set_rotary(ui->pos);
    }
    return ok;
}

void UI_Move_cursor(struct UI* ui) {
    uint8_t next_cursor_pos = BSP_IO_get_cursor(ui->pos, ui->num);
    if (next_cursor_pos != ui->pos) {
        UI_Move_highlight(ui, next_cursor_pos);
        ui->pos = next_cursor_pos;
    }
}
