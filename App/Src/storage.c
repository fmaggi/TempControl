#include "storage.h"

#include "bsp.h"
#include <string.h>

struct Data {
    PID pid;
    uint32_t curves[CURVE_LENGTH][NUM_CURVES];
};

FLASH_STORAGE struct Data data;

uint32_t* Storage_get_curve(uint32_t index) {
    return data.curves[index];
}

void Storage_set_curve(uint32_t index, uint32_t curve[CURVE_LENGTH]) {
    struct Data temp = data;    
    memcpy(temp.curves[index], curve, sizeof(uint32_t) * CURVE_LENGTH);
    BSP_Flash_write(&data, sizeof(struct Data) / 4, (uint32_t*)&temp);
}

PID Storage_get_PID(void) {
    return data.pid;
}

void Storage_set_PID(PID pid) {
    struct Data temp = data;
    temp.pid = pid;
    BSP_Flash_write(&data, sizeof(struct Data) / 4, (uint32_t*)&temp);
}
