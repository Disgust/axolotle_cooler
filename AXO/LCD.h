/*
 * LCD.h
 *
 * Created: 4/25/2014 10:02:37 PM
 *  Author: Disgust
 */ 


#ifndef LCD_H_
#define LCD_H_

/****************************************************************************/
/* LCD_EN - �����. 1 - ������ ������ ��� �������, 0 - �������               */
/* LCD_RS - �������� ���� ������������ ����������. 1 - ������, 0 - �������  */
/* LCD_RW - ����������� ������. 1 - ������, 0 - ������. �������� � ������.  */
/* LCD_DX - ���� ������.                                                    */
/****************************************************************************/

// DB7	DB6	DB5	DB4	DB3	DB2	DB1	DB0	��������
// 0	0	0	0	0	0	0	1	������� ������. ������� ������ �� 0 ������� DDRAM
// 0	0	0	0	0	0	1	-	��������� �� DDRAM ����� �������, ������� ������ �� 0
// 0	0	0	0	0	1	I/D	S	��������� ������ ������ � �������
// 0	0	0	0	1	D	C	B	��������� ������ �����������
// 0	0	0	1	S/C	R/L	-	-	����� ������� ��� ������, � ����������� �� �����
// 0	0	1	DL	N	F	-	-	����� ����� �����, ������ ���� � ������� �������
// 0	1	AG	AG	AG	AG	AG	AG	����������� ��������� �� SGRAM � ������ ����� � SGRAM
// 1	AD	AD	AD	AD	AD	AD	AD	����������� ��������� �� DDRAM � ������ ����� � DDRAM

// I/D � ��������� ��� ��������� �������� ������. 0 � ���������. �.�. ������ ��������� ���� ����� ������� � n-1 ������. ���� ��������� 1 � ����� ���������.
// S - ����� ������. 1 - � ������ ����� �������� ���������� ���� ������. �� ����� DDRAM.
// D � �������� �������. ���� ��������� ���� 0 �� ����������� ��������, � �� � ��� ����� ����� � ����������� ������� ������ ������������ � ��� �� ����� �������� �����. � ����� �������� ��������� � ��� ������� ���� �������� 1.
// � - �������� ������ � ���� ��������. ��� ������, �������� ���� 1 � ��������� ������.
// B � ������� ������ � ���� ��������� ������� ��������.
// S/C ����� ������� ��� ������. ���� ����� 0, �� ���������� ������. ���� 1, �� �����. �� ������ ���� �� �������
// R/L � ���������� ����������� ������ ������� � ������. 0 � �����, 1 � ������.
// D/L � ��� ������������ ������ ���� ������. 1-8 ���, 0-4 ����
// N � ����� �����. 0 � ���� ������, 1 � ��� ������.
// F - ������ ������� 0 � 5�8 �����. 1 � 5�10 ����� (����������� ������ �����)
// AG - ����� � ������ CGRAM
// �D � ����� � ������ DDRAM

#ifndef LCD_DREG
#define LCD_DREG DDRB
#endif

#ifndef LCD_DPORT
#define LCD_DPORT PORTB
#endif

#ifndef LCD_CREG
#define LCD_CREG DDRC
#endif

#ifndef LCD_CPORT
#define LCD_CPORT PORTC
#endif

#ifndef LCD_IN
#define LCD_IN PINB
#endif

#ifndef LCD_D0
#define LCD_D0 PINB0
#endif

#ifndef LCD_D1
#define LCD_D1 PINB1
#endif

#ifndef LCD_D2
#define LCD_D2 PINB2
#endif

#ifndef LCD_D3
#define LCD_D3 PINB3
#endif

#ifndef LCD_EN
#define LCD_EN PINC3
#endif

#ifndef LCD_RS
#define LCD_RS PINC1
#endif

#ifndef LCD_RW
#define LCD_RW PINC2
#endif

 #define control BIT_OFF(LCD_CPORT,LCD_RS)
 #define data BIT_ON(LCD_CPORT, LCD_RS)
 #define enable BIT_ON(LCD_CPORT, LCD_EN)
 #define disable BIT_OFF(LCD_CPORT, LCD_EN)
 #define readlcd BIT_ON(LCD_CPORT, LCD_RW)
 #define writelcd BIT_OFF(LCD_CPORT, LCD_RW)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Defines.h"

void LCD_Clear();
void LCD_Write(char*, uint8_t, uint8_t);
void LCD_turnOn();
void LCD_turnOff();
void LCD_Init();
void LCD_SetCursor();
int LCD_Busy();
#endif /* LCD_H_ */
