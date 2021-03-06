/*
 * Defines.h
 *
 * Created: 4/25/2014 10:17:40 PM
 *  Author: Peter
 */ 


#ifndef DEFINES_H_
#define DEFINES_H_

///���������///
#define MAX_TEMP 20.0
#define MIN_TEMP 15.0
#define MAX_TOL 1.0
#define MIN_TOL 0.0

///�������///
#define BIT_set(x,y,z) x|=(z<<y)
#define BIT_clear(x,y) x&=~(1<<y)
#define BIT_read(x,y) (((x)>>(y))&0x01)
#define BIT_write(x,y,z) ((z)?(BIT_set(x,y,z)):(BIT_clear(x,y)))

#define HI(x) (x>>8)
#define LO(x) (x^0xFF)

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define CONSTRAIN(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define CIRCLE(amt, low, high) ((amt)<(low)?(high):((amt)>(high)?(low):(amt)))
#define ROUND(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

///���� B///
#define LDDR DDRB
#define LDP PORTB
#define LCD_D0 PINB0
#define LCD_D1 PINB1
#define LCD_D2 PINB2
#define LCD_D3 PINB3
#define LCD_D4 PINB0
#define LCD_D5 PINB1
#define LCD_D6 PINB2
#define LCD_D7 PINB3
///���� C///
#define SENSOR_REG DDRC
#define TEMP_SENSOR PINC0
//-----------
#define LCDR DDRC
#define LCP PORTC
#define LCD_E PINC3
#define LCD_RW PINC2
#define LCD_RS PINC1
///���� D///
#define CONTROL_REG DDRD
#define CONTROL_PORT PORTD
#define LCD_LED PIND7
#define LOAD PIND6
#define BUTTON_P PIND5
#define BUTTON_M PIND4
#define BUTTON_OK PIND3
#define BUTTON_BACK PIND2
#define FAN PIND0
///�����///
#define LCD_ON 0
#define MENU_ON 1
#define ECONOMY 2
#define COOLING 3
#define INACTIVE 4
#define FAN_ON 5




#endif /* DEFINES_H_ */