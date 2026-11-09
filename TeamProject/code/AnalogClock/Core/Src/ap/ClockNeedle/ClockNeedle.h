#ifndef SRC_AP_CLOCKNEEDLE_CLOCKNEEDLE_H_
#define SRC_AP_CLOCKNEEDLE_CLOCKNEEDLE_H_

void ClockNeedle_init(void);
void ClockNeedle_ISR(void);
void ClockNeedle_SecStep(void);
void ClockNeedle_MinStep(void);
void ClockNeedle_HourStep(void);

#endif /* SRC_AP_CLOCKNEEDLE_CLOCKNEEDLE_H_ */
