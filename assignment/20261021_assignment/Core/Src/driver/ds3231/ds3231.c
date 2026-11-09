///*
// * ds3231.c
// *
// *  Created on: 2026. 10. 22.
// *      Author: kccistc
// */
//
//#include "ds3231.h"
//
//
//typedef struct {
//	uint8_t year;
//	uint8_t mon;
//	uint8_t day;
//	uint8_t dow;
//} calendar_t;
//
//typedef struct {
//	uint8_t hour;
//	uint8_t min;
//	uint8_t sec;
//	uint8_t msec;
//} time_t;
//
//calendar_t calendar;
//time_t time;
//
//uint8_t dsData_send;
//uint8_t dsData_read[3];
//
//void sendData(void)
//{
//	HAL_I2C_Master_Transmit(hi2c, 0x68, &dsData_send, 1, 100);
//}
//
//void readData(void)
//{
//	HAL_I2C_Master_Receive(hi2c, 0x68, &dsData_read, 4, 100);
//}
//
//void Ds_writeByte(uint8_t data)
//{
//	Ds_sendData(data);
//}
//
//void Ds_writeCmdData(uint8_t data)
//{
//	Lcd_writeByte(data);
//}
//
//void Ds_writeCharData(uint8_t data)
//{
//	Lcd_writeByte(data);
//}
//
//void Ds_writeString(char *str)
//{
//	for (int i=0; str[i]; i++)
//	{
//		Ds_writeCharData(str[i]);
//	}
//}
//
//void Ds_gotoXY(uint8_t row, uint8_t col)	// row : 줄  col : 칸
//{
//	col %= 16;
//	row %= 2;
//
//	uint8_t dsRegAddr = (0x40 * row) + col;
//	uint8_t cmd = 0x00 + dsRegAddr;
//	Ds_writeCmdData(cmd);
//}
//
//void Ds_writeStringXY(uint8_t row, uint8_t col, char *str)
//{
//	Ds_gotoXY(row,col);
//	Ds_writeString(str);
//}
//
//void Ds_modifyTime(void)
//{
//	time.hour = 0x01;
//	dsData_send = time.hour;
//	Ds_sendData();
//
//	time.min = 0x02;
//	dsData_send = time.min;
//	Ds_sendData();
//
//	time.sec = 0x03;
//	dsData_send = time.sec;
//	Ds_sendData();
//
//	time.msec = 0x04;
//	dsData_send = time.msec;
//	Ds_sendData();
//
//
//}
