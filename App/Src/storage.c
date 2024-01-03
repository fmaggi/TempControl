#include "storage.h"

#include "bsp.h"
#include <string.h>

struct Data {
    PID pid;
    CurvePoint points[CURVE_LENGTH][NUM_CURVES];
};

FLASH_STORAGE struct Data data;

void Storage_get_curve(uint32_t index, CurvePoint points[CURVE_LENGTH]) {
    if (index >= NUM_CURVES) return;
    memcpy(points, data.points[index], sizeof(Curve));
}

void Storage_set_curve(uint32_t index, const CurvePoint points[CURVE_LENGTH]) {
    struct Data temp;    
    memcpy(&temp, &data, sizeof(struct Data));
    memcpy(temp.points[index], points, sizeof(Curve));
    BSP_Flash_write(&data, sizeof(struct Data) / sizeof(uint32_t), (uint32_t*)&temp);
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
    BSP_Flash_write(&data, sizeof(struct Data) / sizeof(uint32_t), (uint32_t*)&temp);
}
