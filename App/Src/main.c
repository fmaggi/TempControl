#include "bsp.h"
#include "control.h"
#include "display.h"
#include "fp16.h"
#include "io.h"
#include "power.h"
#include "storage.h"
#include "temperature.h"
#include "ui.h"

#include <stdint.h>
#include <stdio.h>

enum Command {
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
    static const char* menu_entries[] = { "Medir T", "Curva 0", "Curva 1", "Curva 2", "Editar PID" };
    static struct UI menu = UI_INIT(menu_entries);

    static uint8_t header = 0;

    if (first_entry) {
        BSP_Comms_receive_expect(&header, 1);
        UI_Enter(&menu, "Menu Principal");
    }

    UI_Move_cursor(&menu);

    if (UI_Selected(&menu)) {
        BSP_Comms_abort();
        switch (menu.pos) {
            case 0: return MEASURE_T;
            case 1:
            case 2:
            case 3: *curve_index = menu.pos - 1; return CURVE;
            case 4: return PID_EDIT;
        }
        return (AppState) (menu.pos + 1);
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
        BSP_Comms_receive_expect((uint8_t*) &command, sizeof(uint8_t));
        BSP_Display_clear(GREEN);
    }

    if (BSP_Comms_received()) {
        switch (command) {
            case Talk: break;
            case CurveSet: {
                uint8_t index = 0;
                BSP_Comms_receive_block(&index, 1);
                Curve curve;
                BSP_Comms_receive_block((uint8_t*) curve, sizeof(Curve));
                Storage_set_curve(index, curve);
                BSP_Display_write_text("Curve setted", 10, 10, MENU_FONT, BLACK, GREEN);
                break;
            };
            case CurveSend: {
                uint8_t index = 0;
                BSP_Comms_receive_block(&index, 1);
                Curve curve;
                Storage_get_curve(index, curve);
                BSP_Comms_transmit_block((uint8_t*) curve, sizeof(Curve));
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
                char err[50];
                sprintf(err, "Unknown message %d", (uint32_t) command);
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
        FP_format(p_buf + 2, pid.p, 1000);
        FP_format(i_buf + 2, pid.i, 1000);
        FP_format(d_buf + 2, pid.d, 1000);

        UI_Enter(&menu, "Editar PID");
    }

    uint8_t clicked = UI_Selected(&menu);
    uint8_t index = menu.selected;
    if (clicked) {
        if (menu.selected == 3) {
            Storage_set_PID(pid);
            return MAIN_MENU;
        } else {
            if (index == 1) {
                BSP_IO_set_rotary(pid.coeffs[index] << 8);
            } else {
                BSP_IO_set_rotary(FP_toInt(pid.coeffs[index]));
            }
        }
    }

    if (menu.selected == UI_UNSELECTED) {
        UI_Move_cursor(&menu);
    } else {
        FP16 val_fp;
        if (index == 1) {
            uint32_t val = BSP_IO_get_rotary(0, UINT32_MAX);
            val_fp = (FP16) val << 8; // I coeff needs to be small in comparison to others, so we cast
        } else {
            uint16_t val = (uint16_t) BSP_IO_get_rotary(0, UINT16_MAX);
            val_fp = FP_fromInt(val);
        }

        if (val_fp != pid.coeffs[index]) {
            pid.coeffs[index] = val_fp;
            FP_format((char*) menu_entries[index] + 2, val_fp, 1000);
            UI_Update_entry(&menu, index, 2);
        }

        if (UI_Unselected(&menu)) {
            Oven_set_PID(pid);
        }
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

    static uint16_t last_t = 0;
    uint16_t current_t = Oven_temperature();
    if (current_t != last_t) {
        last_t = current_t;
        sprintf(measurement + 5, "%d", current_t);
        UI_Update_entry(&ui, 0, 5);
    }

    if (BSP_IO_ok_clicked()) {
        BSP_T_stop();
        return MAIN_MENU;
    }

    return MEASURE_T;
}

static AppState curve(uint8_t first_entry, uint8_t curve_index) {
    static uint16_t temp = 0;
    static char set_point_buf[50] = "Set point(C)=";
    static char temp_buf[50] = "T(C)=";
    static const char* menu_entries[] = { set_point_buf, temp_buf, "< Volver" };
    static struct UI ui = UI_INIT(menu_entries);

    static uint32_t start_time = 0;
    static uint16_t last_target = 0;

    static Curve curve = { 0 };

    static enum Command com = 0;

    if (first_entry) {
        last_target = 0;
        sprintf(set_point_buf, "Set point(C)=%d", last_target);

        char title[] = "Curva 0";
        title[6] = '0' + curve_index;
        UI_Enter(&ui, title);

        Storage_get_curve(curve_index, curve);

        BSP_Comms_receive_expect((uint8_t*) &com, 1);

        Oven_start();

        start_time = BSP_millis();
    }

    uint32_t current_time = BSP_millis();
    uint16_t elapsed_seconds = (uint16_t) ((current_time - start_time) / 1000);

    uint16_t target = curve[elapsed_seconds];
    Oven_set_target(target);
    if (target != last_target) {
        last_target = target;
        sprintf(set_point_buf + 13, "%d", target);
        UI_Update_entry(&ui, 0, 13);
    }

    uint16_t current_temp = Oven_temperature();
    if (current_temp != temp) {
        temp = current_temp;
        sprintf(temp_buf + 5, "%d", current_temp);
        UI_Update_entry(&ui, 1, 5);
    }

    /* if (send_data) { */
    static uint32_t last_seconds = 0;
    if (last_seconds != elapsed_seconds) {
        last_seconds = elapsed_seconds;
        const uint16_t header = 0xAAAA;
        BSP_Comms_transmit_block((uint8_t*) &header, sizeof(uint16_t));
        BSP_Comms_transmit_block((uint8_t*) &elapsed_seconds, sizeof(uint16_t));
        BSP_Comms_transmit_block((uint8_t*) &temp, sizeof(uint16_t));
        BSP_Comms_transmit_block((uint8_t*) &target, sizeof(uint16_t));
    }
    /* } */

#ifdef DEBUG
    struct PowerState power_state = BSP_Power_get();
    char pbuf[20] = { 0 };
    sprintf(pbuf, "%d %d %d", power_state.power, power_state.period1, power_state.period2);
    BSP_Display_write_text("AAAAAAAAAAAAAA", 36, 190, FONT3, BG_COLOR, BG_COLOR);
    BSP_Display_write_text(pbuf, 36, 190, FONT3, FG_COLOR, BG_COLOR);
#endif

    uint8_t stop = elapsed_seconds >= CURVE_LENGTH;

    if (BSP_Comms_received()) {
        stop = com == Stop;
        BSP_Comms_receive_expect((uint8_t*) &com, 1);
    }

    if (UI_Selected(&ui)) {
        stop = ui.selected == ui.num - 1;
    }

    if (ui.selected == UI_UNSELECTED) {
        UI_Move_cursor(&ui);
    } else {
        UI_Unselected(&ui);
    }

    if (stop) {
        Oven_stop();
        const uint16_t ending = 0xABAB;
        BSP_Comms_transmit_block((uint8_t*) &ending, sizeof(uint16_t));
        return MAIN_MENU;
    }

    return CURVE;
}

int main(void) {
    BSP_init();
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

        changed_state = next_state != current_state;
        current_state = next_state;

        /* BSP_delay(10); */
    }
}
