#ifndef _UI_H
#define _UI_H

#include <stdint.h>

#define FG_COLOR BLACK
#define BG_COLOR WHITE
#define HL_COLOR ORANGE
#define OK_COLOR BLUE

#define MENU_FONT   FONT3
#define FIRST_ENTRY 40
#define BOX_HEIGHT  36
#define MARGIN      12

void UI_Clear(void);
void UI_Write_Title(const char* title);

struct UI {
    const char** entries;
    uint8_t num;
    uint8_t pos;
    uint8_t selected;
};

#define UI_UNSELECTED 0xff
#define UI_INIT(entries) { entries, sizeof(entries) / sizeof(entries[0]), 0, 0 }

void UI_Enter(struct UI* ui, const char* title);
void UI_Reset(struct UI* ui);
void UI_Write(const struct UI* ui);
void UI_Update_entry(const struct UI* ui, uint8_t entry, uint8_t from);
void UI_Move_highlight(struct UI* ui, uint8_t to);
uint8_t UI_Selected(struct UI* ui);
uint8_t UI_Unselected(struct UI* ui);
void UI_Move_cursor(struct UI* ui);

#endif // !_UI_H
