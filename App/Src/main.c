#include "bsp.h"
#include "control.h"
#include "display.h"
#include "fp16.h"
#include "io.h"
#include "power.h"
#include "storage.h"
#include "temperature.h"
#include "ui.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// NOTE: I think this is a bit cursed
#define ON_CHANGE(var, action)                                                                                                 \
    do {                                                                                                                       \
        static uint32_t _last_##var = 0;                                                                                       \
        if (_last_##var != (uint32_t) var) {                                                                                   \
            _last_##var = (uint32_t) var;                                                                                      \
            action;                                                                                                            \
        }                                                                                                                      \
    } while (0)

enum Command {
    ZERO = 0x0,
    Talk = 0xAA,
    CurveSet = 0xCC,
    CurveSend = 0xCE,
    PIDSend = 0xDD,
    PIDSet = 0xDE,
    Start = 0xFF,
    Stop = 0xFE,
};

typedef enum {
    MAIN_MENU = 0,
    MEASURE_T,
    CURVE,
    PID_EDIT,
    EXT_CONTROL,
    LAST_STATE = EXT_CONTROL
} AppState;

static AppState main_menu(uint8_t first_entry, uint8_t* curve_index) {
    static const char* menu_entries[] = { "Medir T", "Curva 1", "Curva 2", "Curva 3", "Editar PID" };
    static struct UI menu = UI_INIT(menu_entries);

    static uint8_t header = 0;

    if (first_entry) {
        header = 0;
        BSP_Comms_receive_expect(&header, 1);
        UI_Enter(&menu, "Menu Principal");
    }

    UI_Move_cursor(&menu);

    if (UI_Select(&menu)) {
        BSP_Comms_abort();
        switch (menu.pos) {
            case 0: return MEASURE_T;
            case 1:
            case 2:
            case 3: *curve_index = menu.pos - 1; return CURVE;
            case 4: return PID_EDIT;
        }
    }

    if (BSP_Comms_received()) {
        if (header == Talk) {
            return EXT_CONTROL;
        }

        BSP_Comms_receive_expect(&header, 1);
    }

    return MAIN_MENU;
}

static AppState external_control(uint8_t first_entry, uint8_t* curve_index) {
    static enum Command command = 0;

    if (first_entry) {
        command = Talk;
        BSP_Comms_receive_expect((uint8_t*) &command, sizeof(uint8_t));
        BSP_Display_clear(GREEN);
    }

    if (BSP_Comms_received()) {
        switch (command) {
            case ZERO: return MAIN_MENU;
            case Talk: break;
            case CurveSet: {
                uint8_t index = 0;
                BSP_Comms_receive_block(&index, 1);
                struct Curve curve = { 0 };
                BSP_Comms_receive_block(&curve.length, 1);
                BSP_Comms_receive_block((uint8_t*) curve.points, sizeof(struct CurvePoint) * curve.length);
                Storage_set_curve(index, &curve);
                BSP_Display_write_text("Curve setted", 10, 10, MENU_FONT, BLACK, GREEN);
                break;
            };
            case CurveSend: {
                uint8_t index = 0;
                BSP_Comms_receive_block(&index, 1);
                struct Curve curve = { 0 };
                Storage_get_curve(index, &curve);
                BSP_Comms_transmit_block((uint8_t*) &curve.length, 1);
                BSP_Comms_transmit_block((uint8_t*) curve.points, sizeof(struct CurvePoint) * curve.length);
                const uint16_t ending = 0xABAB;
                BSP_Comms_transmit_block((uint8_t*) &ending, sizeof(uint16_t));
                BSP_Display_write_text("Curve sent", 10, 10, MENU_FONT, BLACK, GREEN);
                break;
            }
            case PIDSend: {
                PID pid = Oven_get_PID();
                BSP_Comms_transmit_block((uint8_t*) pid.coeffs, sizeof(uint32_t) * 3);
                BSP_Display_write_text("PID sent", 10, 10, MENU_FONT, BLACK, GREEN);
                break;
            };
            case PIDSet: {
                PID pid = { 0 };
                BSP_Comms_receive_block((uint8_t*) &pid.p, sizeof(uint32_t));
                BSP_Comms_receive_block((uint8_t*) &pid.i, sizeof(uint32_t));
                BSP_Comms_receive_block((uint8_t*) &pid.d, sizeof(uint32_t));
                Oven_set_PID(pid);
                Storage_set_PID(pid);
                BSP_Display_write_text("PID setted", 10, 10, MENU_FONT, BLACK, GREEN);
                break;
            };
            case Start: {
                BSP_Comms_receive_block(curve_index, 1);
                return CURVE;
            };
            case Stop: return MAIN_MENU;
            default: {
                char err[50] = "Unknown message";
                nformat_u32s(err + 15, 50 - 15 - 1, "%", command);
                Error_Handler(err);
            }
        }
        BSP_Comms_receive_expect((uint8_t*) &command, sizeof(uint8_t));
    }

    return EXT_CONTROL;
}

static AppState edit_pid(uint8_t first_entry) {
    static char p_buf[30] = "P=";
    static char i_buf[30] = "I=";
    static char d_buf[30] = "D=";
    static const char* menu_entries[] = { p_buf, i_buf, d_buf, "< Volver" };
    static struct UI menu = UI_INIT(menu_entries);

    static PID pid = { 0 };

    if (first_entry) {
        pid = Oven_get_PID();
        FP_format(p_buf + 2, 28, pid.p, 1000);
        FP_format(i_buf + 2, 28, pid.i, 1000);
        FP_format(d_buf + 2, 28, pid.d, 1000);

        UI_Enter(&menu, "Editar PID");
    }

    // P and D will have a step of 1 per rotary click. That's why we convert it back to ints.
    // I in the PID has to be < 1. So we have to handle it different.
    if (UI_Select(&menu)) {
        uint32_t value = 0;
        switch (menu.selected) {
            case 0: value = FP_toInt(pid.p); break;
            case 1: value = (uint32_t) (pid.i >> 8); break;
            case 2: value = FP_toInt(pid.d); break;
            case 3: {
                Oven_set_PID(pid);
                Storage_set_PID(pid);
                return MAIN_MENU;
            }
            default: break;
        }
        BSP_IO_set_rotary(value);
    }

    switch (menu.selected) {
        case 0: {
            uint16_t v = (uint16_t) BSP_IO_get_rotary(0, UINT16_MAX);
            FP16 p = FP_fromInt(v);
            ON_CHANGE(p, {
                pid.p = p;
                FP_format((char*) p_buf + 2, 28, p, 1000);
                UI_Update_entry(&menu, 0, 2);
            });
            UI_Unselect(&menu);
            break;
        }
        case 1: {
            uint32_t v = BSP_IO_get_rotary(0, UINT32_MAX);
            FP16 i = (FP16) (v << 8);
            ON_CHANGE(i, {
                pid.i = i;
                FP_format((char*) i_buf + 2, 28, i, 1000);
                UI_Update_entry(&menu, 1, 2);
            });
            UI_Unselect(&menu);
            break;
        }
        case 2: {
            uint16_t v = (uint16_t) BSP_IO_get_rotary(0, UINT16_MAX);
            FP16 d = FP_fromInt(v);
            ON_CHANGE(d, {
                pid.d = d;
                FP_format((char*) d_buf + 2, 28, d, 1000);
                UI_Update_entry(&menu, 2, 2);
            });
            UI_Unselect(&menu);
            break;
        }
        case UI_UNSELECTED:
        default: UI_Move_cursor(&menu); break;
    }

    return PID_EDIT;
}

static AppState measure_temp(uint8_t first_entry) {
    static char measurement[50] = "T(C)=";
    static const char* entries[] = { measurement };
    static struct UI ui = UI_INIT(entries);

    if (first_entry) {
        UI_Enter(&ui, "Medicion de T");
        BSP_T_start();
    }

    uint16_t t = Oven_get_temperature();
    ON_CHANGE(t, {
        nformat_u32(measurement + 5, 50 - 5 - 1, t);
        UI_Update_entry(&ui, 0, 5);
    });

    if (BSP_IO_ok_clicked()) {
        BSP_T_stop();
        return MAIN_MENU;
    }

    return MEASURE_T;
}

static AppState curve(uint8_t first_entry, uint8_t curve_index) {
#define STOP()                                                                                                                 \
    do {                                                                                                                       \
        Curve_stop(&r);                                                                                                        \
        const uint16_t ending = 0xABAB;                                                                                        \
        BSP_Comms_transmit_block((uint8_t*) &ending, sizeof(uint16_t));                                                        \
        BSP_Comms_abort();                                                                                                     \
        return MAIN_MENU;                                                                                                      \
    } while (0)

    static char set_point_buf[50] = "Set point(C)=";
    static char temp_buf[50] = "T(C)=";
    static const char* menu_entries[] = { set_point_buf, temp_buf, "< Volver" };
    static struct UI ui = UI_INIT(menu_entries);

    static uint32_t start_time = 0;

    static struct CurveRunner r = { 0 };

    if (first_entry) {
        char title[] = "Curva 0";
        title[6] = '0' + curve_index + 1;
        UI_Enter(&ui, title);

        Curve_start(&r, curve_index);

        start_time = BSP_millis();
    }

    uint32_t current_time = BSP_millis();
    uint16_t elapsed_seconds = (uint16_t) ((current_time - start_time) / 1000);

    if (Curve_step(&r, elapsed_seconds)) {
        STOP();
    }

    uint16_t target = Oven_get_target();
    ON_CHANGE(target, {
        nformat_u32(set_point_buf + 13, 50 - 13 - 1, target);
        UI_Update_entry(&ui, 0, 13);
    });

    uint16_t temp = Oven_get_temperature();
    ON_CHANGE(temp, {
        nformat_u32(temp_buf + 5, 50 - 5 - 1, temp);
        UI_Update_entry(&ui, 1, 5);
    });

    ON_CHANGE(elapsed_seconds, {
        uint16_t data[4];
        data[0] = 0xAAAA;
        data[1] = elapsed_seconds;
        data[2] = temp;
        data[3] = target;
        BSP_Comms_transmit_block((uint8_t*) data, sizeof(uint16_t) * 4);
    });

    UI_Select(&ui);
    switch (ui.selected) {
        case 2: STOP();
        case UI_UNSELECTED: UI_Move_cursor(&ui); break;
        default: UI_Unselect(&ui); break;
    }

#ifdef DEBUG
    char _debugbuf[20] = { 0 };

    #if 1
    struct Error e = Oven_error();
    nformat_i32s(_debugbuf, 20 - 1, "% % % %", e.p, e.i, e.d, FP_toInt(r.gradient));
    BSP_Display_write_text("AAAAAAAAAAAAAAAAAAA", 36, 190, FONT3, BG_COLOR, BG_COLOR);
    BSP_Display_write_text(_debugbuf, 36, 190, FONT3, FG_COLOR, BG_COLOR);
    #endif

    #if 0
    nformat_u32s(_debugbuf, 20-1, "% %", r.index, r.curve.length);
    BSP_Display_write_text("AAAAAAAAAAAAAAAAAAA", 36, 190, FONT3, BG_COLOR, BG_COLOR);
    BSP_Display_write_text(_debugbuf, 36, 190, FONT3, FG_COLOR, BG_COLOR);
    #endif

#endif

#undef STOP

    return CURVE;
}

int main(void) {
    BSP_init(T_SAMPLE_PERIOD_ms);
    Oven_set_PID(Storage_get_PID());

    AppState current_state = MAIN_MENU;
    AppState next_state = MAIN_MENU;

    // Make sure we run main_menu_setup on first run
    uint8_t changed_state = 1;

    uint8_t curve_index = 0;
    while (1) {
        switch (current_state) {
            case MAIN_MENU: next_state = main_menu(changed_state, &curve_index); break;
            case MEASURE_T: next_state = measure_temp(changed_state); break;
            case CURVE: next_state = curve(changed_state, curve_index); break;
            case PID_EDIT: next_state = edit_pid(changed_state); break;
            case EXT_CONTROL: next_state = external_control(changed_state, &curve_index); break;
        }

        struct PowerState state = BSP_Power_get();
        if (state.on) {
            uint32_t power = state.power;
            ON_CHANGE(power, {
                char buf[10];
                nformat_u32(buf, 10, power);
                BSP_Display_fill_rect(280, 10, SCREEN_WIDTH - 280, MENU_FONT[2], WHITE);
                BSP_Display_write_text(buf, 280, 10, MENU_FONT, BLACK, WHITE);
            });
        } else if (changed_state) {
            // We put this logic before changed_state is assigned to run after setup on a changed state event
            // to avoid being cleared by the state handler function
            BSP_Display_fill_rect(280, 10, SCREEN_WIDTH - 280, MENU_FONT[2], WHITE);
            BSP_Display_write_text("OFF", 280, 10, MENU_FONT, RED, WHITE);
        }

        changed_state = next_state != current_state;
        current_state = next_state;
    }
}

void BSP_T_on_conversion(uint32_t temperature) {
    if (temperature > 300) {
        Error_Handler("Horno muy caliente");
    }
    Oven_set_temperature((uint16_t) temperature);
    if (BSP_Power_get().on) {
        Oven_control((uint16_t) temperature);
    }
}
