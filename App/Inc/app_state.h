#ifndef _APP_STATE_H
#define _APP_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	MAIN_MENU = 0,
	CURVE1,
	CURVE2,
    CURVE3,
	LAST_STATE = CURVE3
} AppState;

#ifdef __cplusplus
}
#endif

#endif
