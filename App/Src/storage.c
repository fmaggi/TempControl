#include "storage.h"

#include "bsp.h"
#include "utils.h"
#include <string.h>

struct Data {
    PID pid;
    struct Curve curves[NUM_CURVES];
};

FLASH_STORAGE struct Data data;

void Storage_get_curve(uint32_t index, struct Curve* curve) {
    if (index >= NUM_CURVES) return;
    memcpy(curve, &data.curves[index], sizeof(struct Curve));
}

void Storage_set_curve(uint32_t index, const struct Curve* curve) {
    struct Data temp;    
    memcpy(&temp, &data, sizeof(struct Data));
    memcpy(&temp.curves[index], curve, sizeof(struct Curve));
    BSP_Flash_write((uint32_t*)&data, sizeof(struct Data) / sizeof(uint32_t), (uint32_t*)&temp);
}

PID Storage_get_PID(void) {
    return data.pid;
}

void Storage_set_PID(PID pid) {
    struct Data temp = {0};
    memcpy(&temp, &data, sizeof(struct Data));
    temp.pid.p = pid.p;
    temp.pid.i = pid.i;
    temp.pid.d = pid.d;
    BSP_Flash_write((uint32_t*)&data, sizeof(struct Data) / sizeof(uint32_t), (uint32_t*)&temp);
}
