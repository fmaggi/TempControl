#include "storage.h"

#include "bsp.h"
#include <string.h>

struct Data {
    PID pid;
    Curve curves[NUM_CURVES];
};

FLASH_STORAGE struct Data data;

void Storage_get_curve(uint32_t index, Curve curve) {
    if (index >= NUM_CURVES) return;
    memcpy(curve, data.curves[index], sizeof(Curve));
}

void Storage_set_curve(uint32_t index, const Curve curve) {
    struct Data temp;    
    memcpy(&temp, &data, sizeof(struct Data));
    memcpy(temp.curves[index], curve, sizeof(Curve));
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