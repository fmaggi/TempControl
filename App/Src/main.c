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
    Curve = 0xCC,
    PIDSend = 0xDD,
    PIDSet = 0xDE,
    Start = 0xFF,
    Stop = 0xFE,
};

typedef enum {
    MAIN_MENU = 0,
    MEASURE_T,
    MANUAL_SET,
    LINEAR_CURVE,
    PID_EDIT,
    EXT_CONTROL,
    LAST_STATE = EXT_CONTROL
} AppState;

static AppState main_menu(uint8_t first_entry) {
    static const char* menu_entries[] = { "Medir T", "Control Manual", "Curva Lineal", "Editar PID" };
    static struct UI menu = UI_INIT(menu_entries);

    static uint8_t header = 0;

    if (first_entry) {
        BSP_Comms_receive_expect(&header, 1);
        UI_Enter(&menu, "Menu Principal");
    }

    UI_Move_cursor(&menu);

    if (UI_Selected(&menu)) {
        BSP_Comms_abort();
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

static AppState external_control(uint8_t first_entry) {
    static enum Command command = 0;

    if (first_entry) {
        BSP_Comms_receive_expect((uint8_t*) &command, sizeof(uint8_t));
        BSP_Display_clear(GREEN);
    }

    if (BSP_Comms_received()) {
        switch (command) {
            case Talk: break;
            case Curve: {
                BSP_Display_write_text("Curve set", 10, 10, MENU_FONT, BLACK, GREEN);
                uint8_t index = 0;
                BSP_Comms_receive_block(&index, 1);
                uint16_t curve[CURVE_LENGTH];
                BSP_Comms_receive_block((uint8_t*) curve, sizeof(uint16_t) * CURVE_LENGTH);
                Storage_set_curve(index, curve);
                break;
            };
            case PIDSend: {
                /* BSP_Display_write_text("PID get", 10, 10, MENU_FONT, BLACK, GREEN); */
                PID pid = Oven_get_PID();
                BSP_Comms_transmit_block((uint8_t*) pid.coeffs, sizeof(uint32_t) * 3);
                break;
            };
            case PIDSet: {
                /* BSP_Display_write_text("PID set", 10, 10, MENU_FONT, BLACK, GREEN); */
                PID pid = { 0 };
                BSP_Comms_receive_block((uint8_t*) &pid.p, sizeof(uint32_t));
                BSP_Comms_receive_block((uint8_t*) &pid.i, sizeof(uint32_t));
                BSP_Comms_receive_block((uint8_t*) &pid.d, sizeof(uint32_t));
                Oven_set_PID(pid);
                Storage_set_PID(pid);
                break;
            };
            case Start: {
                uint8_t index = 0;
                BSP_Comms_receive_block(&index, 1);
                // set index and return curve;
                return LINEAR_CURVE;
            };
            case Stop: return MAIN_MENU;
            default: {
                char err[50];
                sprintf(err, "Unknown message %d", (uint32_t)command);
                Error_Handler(err);
            }
        }
        BSP_Comms_receive_expect((uint8_t*) &command, sizeof(uint8_t));
    }

    return EXT_CONTROL;
}

static AppState edit_pid(uint8_t first_entry) {
    static char p_buf[8] = "P=";
    static char i_buf[30] = "I=";
    static char d_buf[8] = "D=";
    static const char* menu_entries[] = { p_buf, i_buf, d_buf, "< Volver" };
    static struct UI menu = UI_INIT(menu_entries);

    static PID pid = { 0 };

    if (first_entry) {
        pid = Oven_get_PID();
        FP_format(p_buf+2, pid.p, 1000);
        FP_format(i_buf+2, pid.i, 1000);
        FP_format(d_buf+2, pid.d, 1000);

        UI_Enter(&menu, "Editar PID");
    }

    uint8_t clicked = UI_Selected(&menu);
    uint8_t index = menu.selected;
    if (clicked) {
        if (menu.selected == 3) {
            Storage_set_PID(pid);
            return MAIN_MENU;
        } else {
            BSP_IO_set_rotary(pid.coeffs[index]);
        }
    }

    if (menu.selected == UI_UNSELECTED) {
        UI_Move_cursor(&menu);
    } else {
        FP16 val = (FP16)BSP_IO_get_rotary(0, (uint32_t)FP_fromInt(250));
        if (val != pid.coeffs[index]) {
            pid.coeffs[index] = val;
            FP_format((char*)menu_entries[index]+2, val, 1000);
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

static AppState manual_set(uint8_t first_entry) {
    static uint16_t last_t = 0;
    static uint16_t current_target = 50;
    static char set_point_buf[50] = "Set point(C)=";
    static char temp_buf[50] = "T(C)=";
    static const char* menu_entries[] = { set_point_buf, temp_buf, "< Volver" };
    static struct UI ui = UI_INIT(menu_entries);

    if (first_entry) {
        current_target = 50;
        BSP_IO_set_rotary(50);
        sprintf(set_point_buf, "Set point(C)=50");

        UI_Enter(&ui, "Control manual");

        Oven_set_target(50);
        Oven_start();
    }

    uint16_t current_t = Oven_temperature();
    if (current_t != last_t) {
        last_t = current_t;
        sprintf(temp_buf + 5, "%d", current_t);
        UI_Update_entry(&ui, 1, 5);
    }

#ifdef DEBUG
    struct PowerState power_state = BSP_Power_get();
    char pbuf[20] = { 0 };
    sprintf(pbuf, "%d %d %d", power_state.power, power_state.period1, power_state.period2);
    BSP_Display_write_text("AAAAAAAAAAAAAA", 36, 190, FONT3, BG_COLOR, BG_COLOR);
    BSP_Display_write_text(pbuf, 36, 190, FONT3, FG_COLOR, BG_COLOR);
#endif

    uint8_t clicked = UI_Selected(&ui);
    if (clicked) {
        if (ui.selected == ui.num - 1) {
            Oven_stop();
            return MAIN_MENU;
        } else {
            BSP_IO_set_rotary(current_target);
        }
    }

    if (ui.selected == UI_UNSELECTED) {
        UI_Move_cursor(&ui);
    } else if (ui.selected == 0) {
        uint16_t target = (uint16_t) BSP_IO_get_rotary(0, 250);
        if (target != current_target) {
            current_target = target;
            sprintf(set_point_buf + 13, "%d", target);
            UI_Update_entry(&ui, 0, 13);
        }

        if (UI_Unselected(&ui)) {
            Oven_set_target(target);
        }
    } else {
        UI_Unselected(&ui);
    }

    return MANUAL_SET;
}

static AppState linear_curve(uint8_t first_entry) {
    static uint16_t temp = 0;
    static char set_point_buf[50] = "Set point(C)=";
    static char temp_buf[50] = "T(C)=";
    static const char* menu_entries[] = { set_point_buf, temp_buf, "< Volver" };
    static struct UI ui = UI_INIT(menu_entries);

    static uint32_t start_time = 0;
    static uint16_t last_target = 0;

    if (first_entry) {
        last_target = 0;
        sprintf(set_point_buf, "Set point(C)=%d", last_target);

        UI_Enter(&ui, "Curva lineal");

        Oven_set_target(last_target);
        Oven_start();

        start_time = BSP_millis();
    }

    uint32_t current_time = BSP_millis();
    uint16_t current_temp = Oven_temperature();
    if (current_temp != temp) {
        temp = current_temp;
        sprintf(temp_buf + 5, "%d", current_temp);
        UI_Update_entry(&ui, 1, 5);
    }

    uint16_t elapsed_seconds = (uint16_t) ((current_time - start_time) / 1000);

    uint16_t target = 20 + elapsed_seconds / 5;
    target = target > 150 ? 150 : target;
    Oven_set_target(target);

    if (target != last_target) {
        last_target = target;
        sprintf(set_point_buf + 13, "%d", target);
        UI_Update_entry(&ui, 0, 13);
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

    uint8_t clicked = UI_Selected(&ui);
    if (clicked) {
        if (ui.selected == ui.num - 1) {
            Oven_stop();
            const uint16_t ending = 0xABAB;
            BSP_Comms_transmit_block((uint8_t*) &ending, sizeof(uint16_t));
            return MAIN_MENU;
        }
    }

    if (ui.selected == UI_UNSELECTED) {
        UI_Move_cursor(&ui);
    } else {
        UI_Unselected(&ui);
    }

    return LINEAR_CURVE;
}

int main(void) {
    BSP_init();
    Oven_set_PID(Storage_get_PID());

    AppState current_state = MAIN_MENU;
    AppState next_state = MAIN_MENU;

    // Make sure we run main_menu_setup on first run
    uint8_t changed_state = 1;

    while (1) {
        switch (current_state) {
            case MAIN_MENU: next_state = main_menu(changed_state); break;
            case MEASURE_T: next_state = measure_temp(changed_state); break;
            case MANUAL_SET: next_state = manual_set(changed_state); break;
            case LINEAR_CURVE: next_state = linear_curve(changed_state); break;
            case PID_EDIT: next_state = edit_pid(changed_state); break;
            case EXT_CONTROL: next_state = external_control(changed_state); break;
        }

        changed_state = next_state != current_state;
        current_state = next_state;

        /* BSP_delay(10); */
    }
}
