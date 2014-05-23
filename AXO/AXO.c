/*
 * AXO.c
 *
 * Created: 4/21/2014 8:00:01 PM
 *  Author: Peter
 */ 

#ifndef F_CPU
    #define F_CPU 8000000UL
#endif


#define BAUD 19200UL
#define BAUD_DIVIDER ((F_CPU/(BAUD*8))-1)
#define LCD_LENGTH 16;
#define LCD_WIDTH 2;
#define LCD_4bit

#define BYTE_TO_VOLTS(x) ((x * 5.0)/1024)
#define BYTE_TO_MILLIVOLTS(x) ((x * 5000.0)/1024)
#define BYTE_TO_TEMP(x) (x * 0.19) // ������������ �������, ����������� ��� ������� TMP036: T= (Vin(mV) - 500)/10

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Defines.h"
#include "d2c.h"
//#include "LCD.h"
#include "lcd_lib.h"
//#include "BIN2ASCII.h"

//#define MEGA_16
#define MEGA_168

///���������� ���������///
const uint8_t timeOut= 0x0A;
///���������� ����������///
volatile static uint8_t runSeconds;
volatile static double temperatureValue;
volatile static double targetTemp= 20.0;
volatile static double Tolerance= 0.0;
volatile static uint8_t measureRate= 0x80; // ��-��������� - ������� ������ (F_CPU/1024)/2
volatile char USART_buffer[8];
volatile static int USART_index;
uint8_t progFlags= 0b00000100;

void doubleToChar(char* c, double d) {
    int i;
    enum { LEN=8 };
    char res[LEN];
    for (i=0; i<8; ++i){
        snprintf (res, LEN, "%4.8f", d);
    }
}

inline static void turnOnFan() 
{
	BIT_write(CONTROL_PORT, FAN, 1);
	BIT_write(progFlags, FAN_ON, 1);
}

inline static void turnOffFan()
{
    BIT_clear(CONTROL_PORT, FAN);
    BIT_clear(progFlags, FAN_ON);
}

inline static void turnOnCooler() 
{
    if(!BIT_read(progFlags, COOLING))
    {   
        turnOnFan();
	    BIT_write(CONTROL_PORT, LOAD, 1);
        BIT_write(progFlags, COOLING, 1);
    }
}

inline static void turnOffCooler() 
{
    if(BIT_read(progFlags, COOLING))
    {
        turnOffFan();
	    BIT_clear(CONTROL_PORT, LOAD);
        BIT_clear(progFlags, COOLING);
    }
}

inline static void sendData( volatile double a) 
{
    //BIN8toASCII3(USART_buffer[0], USART_buffer[1], USART_buffer[2],a); //TODO: ������������
    //memcpy(&USART_buffer,&a, 8);
    USART_buffer[7]= '\n';
    #ifdef MEGA_16
    UDR= *USART_buffer;
    USART_index= 1;
    BIT_write(UCSRB, UDRIE, 1);
    #elif defined MEGA_168
    UDR0= *USART_buffer;
    USART_index= 1;
    BIT_write(UCSR0B, UDRIE0, 1);
    #endif
    
}

void turnOnSleep()
{
    //TODO: ��������� ���������� INT1
    //TODO: ��������� ����������� �������������� ���
    //TODO: ��������� ���������� �� ������������ ������� 2
    //BIT_ON(PRR, PRADC); // ����� ������ �� ���
    //BIT_ON(SMCR, SM0);
    //SMCR |= 1 << SE; // ��������
}

void turnOffSleep()
{
    //TODO: ��������� ���������� INT1
    //TODO: �������� ����������� �������������� ���
    //TODO: ��������� ���������� �� ������������ ������� 2
    //BIT_OFF(PRR, PRADC);
    //BIT_OFF(SMCR, SM0);
}


inline void LCD_DisplayInfo()
{   
    LCDclr();
    LCDGotoXY(0, 0);
    LCDstring("TEMP: ", 6);
    char arr[5];
    LCDGotoXY(6, 0);
    double2char(arr, temperatureValue);
    LCDstring(arr, 8);
    if (BIT_read(progFlags, COOLING))
    {
        LCDGotoXY(0, 1);
        LCDstring("COOLING ", 8);
        LCDGotoXY(8, 1);
        double2char(arr, ((temperatureValue - targetTemp)/Tolerance)*100);
        LCDstring(arr, 8);
    }
}


inline int inRange(int pos, int value)
{
    switch (pos)
    {
    case 1:
        value= CONSTRAIN(value, MIN_TEMP, MAX_TEMP);
        break;
    case 2:
        value= CONSTRAIN(value, MIN_TOL, MAX_TOL);
        break;
    case 3:
        value= CIRCLE(value, 0, 1023);
        break;
    case 4:
        value= CONSTRAIN(value, 0, 1);
        break;
    }
    return value;
}

void menuRun()              //TODO: ���������� ������ ���� ����� ���������, ���������� ���, �������� � ������� ��������
                                // �������� ������ ���� ������ � �������� ���������� �������
                                // ������������� ������� inRange()
{
    int pos= 0;
    char menu[4][16]= {"Target temp  (1)", "Tolerance    (2)", "Measure rate (3)", "P-save mode  (4)"};
	int values[4]= {targetTemp, Tolerance, measureRate, (BIT_read(progFlags, ECONOMY))};
    LCDclr();
    while (!(BIT_read(progFlags, INACTIVE)||(!BIT_read(CONTROL_PORT, BUTTON_BACK)))){
        if (!BIT_read(CONTROL_PORT, BUTTON_OK)){
            BIT_clear(progFlags, INACTIVE);
            runSeconds= 0;
            int value= values[pos];
            while(!(BIT_read(progFlags, INACTIVE)||(!BIT_read(CONTROL_PORT, BUTTON_BACK)))){
                //LCD_Write(values[pos],1,0);
                if (!BIT_read(CONTROL_PORT, BUTTON_P))
                {
                    BIT_clear(progFlags, INACTIVE);
                    runSeconds= 0;
                    //values[pos]++;
                    values[pos]= inRange(pos, ++values[pos]);
                }
                if (!BIT_read(CONTROL_PORT, BUTTON_M))
                {
                    BIT_clear(progFlags, INACTIVE);
                    runSeconds= 0;
                    //values[pos]--;
                    values[pos]= inRange(pos, --values[pos]);

                }
                if (!BIT_read(CONTROL_PORT, BUTTON_OK)) {
                    if(pos!=3){
                        values[pos]= (uint8_t)value;
                        break;
                    }
                    else{
                        BIT_write(progFlags, ECONOMY, value);
                        break;
                    }          
                }
                LCDclr();
                LCDGotoXY(0, 0);
                LCDstring(menu[pos],16);
                LCDGotoXY(0, 1);
                LCDstring(values[pos],1);               
            }
        }
        LCDclr();
        LCDGotoXY(0, 0);
        LCDstring(menu[pos],16);
        LCDGotoXY(0, 1);
        LCDstring(values[pos],1);
        if (!BIT_read(CONTROL_PORT, BUTTON_P))
        {
            BIT_clear(progFlags, INACTIVE);
            runSeconds= 0;
            pos++;
            pos= CIRCLE(pos, 0, 3);
        }
        if (!BIT_read(CONTROL_PORT, BUTTON_M))
        {
            BIT_clear(progFlags, INACTIVE);
            runSeconds= 0;
            pos--;
            pos= CIRCLE(pos, 0, 3);
        }
    }
    BIT_clear(progFlags, MENU_ON);
    BIT_clear(progFlags, INACTIVE);
    runSeconds= 0;
    LCD_DisplayInfo();
}

int main(void)
{
    ///������������� �����///
    #ifdef MEGA_16
    UBRRL= LO(ROUND(BAUD_DIVIDER));//( F_CPU /( baud * 16 ) ) - 1; // ��������� ��������
    UBRRH= HI(ROUND(BAUD_DIVIDER));
    BIT_write(UCSRC, UPM1, 0);  // �������� �������� ���������
    BIT_write(UCSRC, UPM0, 0);  // ----||----
    BIT_write(UCSRB, UCSZ2, 0);    // 8 ����� ������
    BIT_write(UCSRC, UCSZ1, 1);    // ----||----
    BIT_write(UCSRC, UCSZ0, 1);    // ----||----
    BIT_write(UCSRC, USBS, 0);  // 1 �������� ���
    BIT_write(UCSRB, TXEN, 1);  // �������� ���������
    BIT_write(UCSRB, RXEN, 0);  // ����� ��������
    BIT_write(UCSRB, RXCIE, 0); // ���������� ������ ���������
    BIT_write(UCSRB, TXCIE, 1); // ���������� ����� �������� ���������
    BIT_write(UCSRB, UDRIE, 0); // ���������� ����������� ������� �������� ��������� - ��� ���������� ��� ��������
    //////////////////////////////////////////////////////////////////////////
    #elif defined MEGA_168
    UBRR0= ROUND(BAUD_DIVIDER);//( F_CPU /( baud * 16 ) ) - 1; // ��������� ��������
    BIT_write(UCSR0C, UPM01, 0);  // �������� �������� ���������
    BIT_write(UCSR0C, UPM00, 0);  // ----||----
    BIT_write(UCSR0B, UCSZ02, 0);    // 8 ����� ������
    BIT_write(UCSR0C, UCSZ01, 1);    // ----||----
    BIT_write(UCSR0C, UCSZ00, 1);    // ----||----
    BIT_write(UCSR0C, USBS0, 0);  // 1 �������� ���
    BIT_write(UCSR0B, TXEN0, 1);  // �������� ���������
    BIT_write(UCSR0B, RXEN0, 0);  // ����� ��������
    BIT_write(UCSR0B, RXCIE0, 0); // ���������� ������ ���������
    BIT_write(UCSR0B, TXCIE0, 1); // ���������� ����� �������� ���������
    BIT_write(UCSR0B, UDRIE0, 0);
    #endif
    ///������������� ������///
    SENSOR_REG&= ~(1 << TEMP_SENSOR); // ����������� �� ����
    CONTROL_REG= (1 << LCD_LED)|(1 << LOAD)|(1<<FAN); // ���������� ���������� ������, ��������� � ������������ �� �����
    CONTROL_REG&= ~(1 << BUTTON_M) & ~(1 << BUTTON_P) & ~(1 << BUTTON_OK) & ~(1 << BUTTON_BACK); // ������ �� ����
    CONTROL_PORT= (1 << BUTTON_M)|(1 << BUTTON_OK)|(1 << BUTTON_P)|(1 << BUTTON_BACK); // ���������� ������������� ��������� � �������
    //////////////////////////////////////////////////////////////////////////
    
    ///������������� ���///
    ADMUX |= 1<<REFS0; // ������� �������� �������� ���������� - ���� AVCC
    ADCSRB |= 3 << ADTS0; // ������� ����� ������������ ��� - �� ���������� ������� 0 � ��������� A
    //ADCSRA |= 6 << ADPS0; // ������� ������� ������� (������������) - F_CPU/ADPS = 8000000/64=125kHz
    ADMUX |= 1 << ADLAR; // ������������ ����������� �� ����� �������
    ADCSRA |= 1 << ADATE; // �������� ����������� ��������������
    ADCSRA |= 1 << ADIE; // ��������� ���������� ���
    ADCSRA |= 1 << ADEN; // ��������� ������ ���
    DIDR0 |= 1 << ADC0D; // ��������� ������ ��������� ����� ADC0D
    
    //////////////////////////////////////////////////////////////////////////
    
    ///������������� ������� 0///
    TCCR0A|= 2 << WGM00; // �������� ����� CTC - ����� �������� �� ����������
    OCR0A= measureRate;
    TCCR0B |= 4 << CS00; // �������� ������ 0 � ������������� 256
    TIMSK0 |= 1 << OCIE0A; // ��������� ���������� ������� �� ��������� � ��������� B
    //////////////////////////////////////////////////////////////////////////
    
    ///������������� ������������ ������� 2///
    TCCR2B= 5 << CS20; // �������� ������ 2 � ������������� 128 (1 ������������ � �������)
    ASSR|= (1 << AS2); // ��������� ����������� �����
    TIMSK2 |= 1 << TOIE2; // ��������� ���������� ������� �� ������������
    //////////////////////////////////////////////////////////////////////////
    
    LCDinit();

    BIT_write(PRR, PRTWI, 1); // ��������� ������� TWI ��� ���������� �����������������
    BIT_write(PRR, PRTIM1, 1); // ��������� ������� ������� 1 ��� ���������� �����������������
    BIT_write(PRR, PRSPI, 1); // ��������� ������� SPI ��� ���������� �����������������
    BIT_write(ACSR, ACD, 1); // ��������� ���������� ����������

    ADCSRA |= 1 << ADSC;

    sei();
    
    ///������� ����///
    while(1)
    {
        //////////////////////////////////////////////////////////////////////////
        // ������ : �������� ��������� ���� ������ �����-���� ������
        //////////////////////////////////////////////////////////////////////////
        if ((!BIT_read(PIND, BUTTON_M))||(!BIT_read(PIND, BUTTON_P))||(!BIT_read(PIND, BUTTON_BACK))) // ���� ������ ����� ������
        {
            BIT_clear(progFlags, INACTIVE); // ����� �� ������ ������������
            runSeconds= 0;
            BIT_write(progFlags, LCD_ON, 1);
            LCDvisible();
        }
        //////////////////////////////////////////////////////////////////////////
        // ������ : ������� � ���� ���� ������ ������ OK/MENU
        //////////////////////////////////////////////////////////////////////////
        if (!BIT_read(PIND,BUTTON_OK)) // ���� ������ ������ OK/MENU                        //TODO: � ������ P-save ������ OK/MENU ������ ������ �� ���������� INT1
        {
            BIT_clear(progFlags, INACTIVE); // ����� �� ������ ������������
            runSeconds= 0;
            if (!BIT_read(progFlags, LCD_ON))
            {
                BIT_write(progFlags, LCD_ON, 1); // �������� ��������� �������7
                LCDvisible();
            }
            BIT_write(progFlags, MENU_ON, 1); // �������� ����
            menuRun(); // ��������� ������ ����
        }
        //////////////////////////////////////////////////////////////////////////
        // ������ : ���������� ������ ���� ��������� �������� ��� �������� ����������
        //////////////////////////////////////////////////////////////////////////
        if ((BIT_read(progFlags, COOLING)&&(!BIT_read(progFlags, ECONOMY))))
        {
            runSeconds= 0;
            BIT_clear(progFlags, INACTIVE);
            LCDvisible();
            LCD_DisplayInfo();
        }
        else if (BIT_read(progFlags, LCD_ON))
        {
            LCD_DisplayInfo();
        }
        //////////////////////////////////////////////////////////////////////////
        // ������ : ��������� ��������� �� ��������� ��������, ��������
        //////////////////////////////////////////////////////////////////////////
        if(BIT_read(progFlags, INACTIVE))
        {
            if(BIT_read(progFlags, LCD_ON))
            {
                BIT_clear(progFlags, LCD_ON);
                LCDblank();
            }
            else if (BIT_read(progFlags, ECONOMY))
            {
                turnOnSleep();
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////

/// ����������� ����������///

ISR(ADC_vect){
    //////////////////////////////////////////////////////////////////////////
    // ������ : ��������� �������� ������� � ��������� ���������
    //////////////////////////////////////////////////////////////////////////
    temperatureValue= BYTE_TO_TEMP((ADCH << 2));
    if (temperatureValue >= (targetTemp + Tolerance))
    {
        turnOnCooler(); // �������� ����������
    }
    else if(temperatureValue <= targetTemp)
    {
        turnOffCooler(); // ��������� ����������
    }
}



ISR(TIMER2_OVF_vect){
    //////////////////////////////////////////////////////////////////////////
    // ������ : ������� �������, �������� ������ � ���������������� ����, 
    // ���������� ���� ������������
    //////////////////////////////////////////////////////////////////////////
    sendData(temperatureValue);
    runSeconds++;
    if (runSeconds==timeOut)
    {
        runSeconds= 0; // ���������� ������� ������
        BIT_write(progFlags, INACTIVE, 1);
    }
    return;
}

ISR(TIMER0_COMPA_vect){
    return;
}

ISR(INT1_vect){
    //////////////////////////////////////////////////////////////////////////
    // ������ : �������� �� ���
    //////////////////////////////////////////////////////////////////////////
    turnOffSleep();
    BIT_write(progFlags, LCD_ON, 1);
    LCDvisible();
}

ISR(USART_UDRE_vect){
    //////////////////////////////////////////////////////////////////////////
    // ������ : �������� ������ ����� ��������� ���� ���������
    //////////////////////////////////////////////////////////////////////////
    UDR0= USART_buffer[USART_index];
    USART_index++;
    if(USART_index == 8) {
        BIT_write(UCSR0B, UDRIE0, 0);
    }
}

ISR(USART_TX_vect){
    return;
}