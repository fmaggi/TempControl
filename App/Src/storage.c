#include "storage.h"

#include "bsp.h"
#include <string.h>

struct Data {
    PID pid;
    uint16_t curves[CURVE_LENGTH][NUM_CURVES];
};

FLASH_STORAGE struct Data data;

void Storage_get_curve(uint32_t index, uint16_t curve[CURVE_LENGTH]) {
    if (index >= NUM_CURVES) return;
    memcpy(curve, data.curves[index], sizeof(uint16_t)*CURVE_LENGTH);
}

void Storage_set_curve(uint32_t index, const uint16_t curve[CURVE_LENGTH]) {
    struct Data temp;    
    memcpy(&temp, &data, sizeof(struct Data));
    memcpy(temp.curves[index], curve, sizeof(uint16_t) * CURVE_LENGTH);
    BSP_Flash_write(&data, sizeof(struct Data) / 4, (uint32_t*)&temp);
}

PID Storage_get_PID(void) {
    return data.pid;
}

void Storage_set_PID(PID pid) {
    struct Data temp = data;
    memcpy(&temp, &data, sizeof(struct Data));
    temp.pid.p = pid.p;
    temp.pid.i = pid.i;
    temp.pid.d = pid.d;
    BSP_Flash_write(&data, sizeof(struct Data) / 4, (uint32_t*)&temp);
}
