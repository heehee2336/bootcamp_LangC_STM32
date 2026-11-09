/*
 * que.h
 *
 *  Created on: 2026. 11. 2.
 *      Author: kccistc
 */

#ifndef SRC_AP_MAIN_SERVICE_QUE_QUE_H_
#define SRC_AP_MAIN_SERVICE_QUE_QUE_H_

#include <stdbool.h>


typedef enum {BTN_FUNCT, BTN_CLEAR} event_btn_t;
typedef enum {ACT_PUSHED, ACT_RELEASED} event_btn_act_t;
typedef struct {
	event_btn_t evt_btn;
	event_btn_act_t evt_btn_act;
} btn_event_t;




void que_init(void);
void que_putMode(event_btn_act_t* act);
void que_putFunctBtn(btn_event_t* btn_evt);
bool que_isEmptyMode(void);
bool que_isEmptyFunctBtn(void);
bool que_isFullMode(void);
bool que_isFullFunctBtn(void);
void que_getMode(event_btn_act_t* act);
void que_getFunctBtn(event_btn_act_t* btn_evt);


#endif /* SRC_AP_MAIN_SERVICE_QUE_QUE_H_ */
