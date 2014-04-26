/*
 * AXO.c
 *
 * Created: 4/21/2014 8:00:01 PM
 *  Author: Disgust
 */ 

#define F_CPU 8000000UL
#define BAUD 115200UL
#define BAUD_DIVIDER ((F_CPU/(BAUD*8))-1)

#define BYTE_TO_VOLTS(x) ((x * 5.0)/1024)
#define BYTE_TO_MILLIVOLTS(x) ((x * 5000.0)/1024)
#define BYTE_TO_TEMP(x) (x * 0.19) // ������������ �������, ����������� ��� ������� TMP036: T= (Vin(mV) - 500)/10


#include <avr/io.h>
#include <avr/interrupt.h>
#include "Defines.h"
#include "LCD.h"

///���������� ����������///
volatile static uint8_t runSeconds;
uint8_t timeOut= 0x0A;
volatile static double temperatureValue;
volatile static double targetTemp= 20.0;
volatile static double Tolerance= 0.0;
volatile static uint8_t measureRate= 0x0100; // ����������� - ������� ������ (F_CPU/1024)/2
uint8_t progFlags= 0b00000000;

inline static void turnOnCooler() 
{
    if(!BIT_READ(progFlags, COOLING))
    {
	    BIT_ON(CONTROL_PORT, LOAD);
        BIT_ON(progFlags, COOLING);
    }
}

inline static void turnOffCooler() 
{
    if(BIT_READ(progFlags, COOLING))
    {
	    BIT_OFF(CONTROL_PORT, LOAD);
        BIT_OFF(progFlags, COOLING);
    }
}

void turnOnSleep()
//TODO: ��������� ���������� INT1
{
    //BIT_ON(PRR, PRADC); // ����� ������ �� ���
    //BIT_ON(SMCR, SM0);
    //SMCR |= 1 << SE; // ��������
}

void turnOffSleep()
//TODO: ��������� ���������� INT1
{
    //BIT_OFF(PRR, PRADC);
    //BIT_OFF(SMCR, SM0);
}

void LCD_DisplayAll()
{   
	LCD_Write("TEMP :", 0, 0);
    LCD_Write((char)temperatureValue, 0, 8);
    if (BIT_READ(progFlags, COOLING))
    {
        LCD_Write("COOLING ", 1, 0);
        LCD_Write((char)((temperatureValue - targetTemp)/Tolerance)*100, 1, 8);
    }
}

inline void menuStop()
{
    LCD_Clear();
    LCD_DisplayAll();
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
	int values[4]= {targetTemp, Tolerance, measureRate, (BIT_READ(progFlags, ECONOMY))};
    LCD_Clear();
    while (1){
        if (!BIT_READ(CONTROL_PORT, BUTTON_OK)){
            BIT_OFF(progFlags, INACTIVE);
            int value= values[pos];
            while(1){
                BIT_OFF(progFlags, INACTIVE); // ����� �� ������ ������������
                LCD_Write(values[pos],1,0);
                if (!BIT_READ(CONTROL_PORT, BUTTON_P))
                {
                    BIT_OFF(progFlags, INACTIVE);
                    //values[pos]++;
                    values[pos]= inRange(pos, ++values[pos]);
                }
                if (!BIT_READ(CONTROL_PORT, BUTTON_M))
                {
                    BIT_OFF(progFlags, INACTIVE);
                    //values[pos]--;
                    values[pos]= inRange(pos, --values[pos]);

                }
                if (!BIT_READ(CONTROL_PORT, BUTTON_BACK)) break;
                if (!BIT_READ(CONTROL_PORT, BUTTON_OK)) {
                    if(pos!=3){
                        values[pos]= (uint8_t)value;
                        break;
                    }
                    else{
                        BIT_WRITE(progFlags, ECONOMY, value);
                    }          
                }                    
            }
        }
        LCD_Write(menu[pos],0,0);
        LCD_Write(values[pos],1,0);
        if (!BIT_READ(CONTROL_PORT, BUTTON_P))
        {
            BIT_OFF(progFlags, INACTIVE);
            pos++;
            pos= CIRCLE(pos, 0, 3);
        }
        if (!BIT_READ(CONTROL_PORT, BUTTON_M))
        {
            BIT_OFF(progFlags, INACTIVE);
            pos--;
            pos= CIRCLE(pos, 0, 3);
        }
        if (!BIT_READ(CONTROL_PORT, BUTTON_BACK)) break;
        if(BIT_READ(progFlags, INACTIVE)) break;
    }
    BIT_OFF(progFlags, MENU_ON);
    BIT_OFF(progFlags, INACTIVE);
    menuStop();
}

int main(void)
{   
    ///������������� �����///
    UBRR0 = ROUND(BAUD_DIVIDER);//( F_CPU /( baud * 16 ) ) - 1; // ��������� ��������
    BIT_WRITE(UCSR0C, UPM01, 0);  // �������� �������� ���������
    BIT_WRITE(UCSR0C, UPM00, 0);  // ----||----
    BIT_WRITE(UCSR0B, UCSZ02, 0);    // 8 ����� ������
    BIT_WRITE(UCSR0C, UCSZ01, 1);    // ----||----
    BIT_WRITE(UCSR0C, UCSZ00, 1);    // ----||----
    BIT_WRITE(UCSR0C, USBS0, 0);  // 1 �������� ���
    BIT_WRITE(UCSR0B, TXEN0, 1);  // �������� ���������
    BIT_WRITE(UCSR0B, RXEN0, 1);  // ����� ��������
    BIT_WRITE(UCSR0B, RXCIE0, 1); // ���������� ������ ���������
    BIT_WRITE(UCSR0B, TXCIE0, 1); // ���������� ����� �������� ���������
    BIT_WRITE(UCSR0B, UDRIE0, 0); // ���������� ����������� ������� �������� ��������� - ��� ���������� ��� ��������
    //////////////////////////////////////////////////////////////////////////
    
    ///������������� ������///
    SENSOR_REG&= ~(1 << TEMP_SENSOR); // ����������� �� ����
    CONTROL_REG= (1 << LCD_LED)|(1 << LOAD); // ���������� ���������� ������ � ��������� �� �����
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
    //DIDR0 |= 1 << ADC0D; // ��������� �������� ���� ADC0D
    
    //////////////////////////////////////////////////////////////////////////
    
    ///������������� ������� 0///
    TCCR0A|= 2 << WGM00; // �������� ����� CTC - ����� �������� �� ����������
    OCR0A= measureRate;
    TCCR0B |= 4 << CS00; // �������� ������ 1 � ������������� 256
    TIMSK0 |= 1 << OCIE0A; // ��������� ���������� ������� �� ��������� � ��������� B
    //////////////////////////////////////////////////////////////////////////
    
    ///������������� ������������ ������� 2///
    TCCR2B= 5 << CS20; // �������� ������ 2 � ������������� 128 (1 ������������ � �������)
    ASSR|= (1 << AS2); // ��������� ����������� �����
    TIMSK2 |= 1 << TOIE2; // ��������� ���������� ������� �� ������������
    //////////////////////////////////////////////////////////////////////////
    
    LCD_Init();

    ADCSRA |= 1 << ADSC;
    
    sei();
    
    ///������� ����///
    while(1)
    {
        //////////////////////////////////////////////////////////////////////////
        // ������ : �������� ��������� ���� ������ �����-���� ������
        //////////////////////////////////////////////////////////////////////////
        if ((!BIT_READ(PIND, BUTTON_M))||(!BIT_READ(PIND, BUTTON_P))||(!BIT_READ(PIND, BUTTON_BACK))) // ���� ������ ����� ������
        {
            BIT_OFF(progFlags, INACTIVE); // ����� �� ������ ������������
            BIT_ON(progFlags, LCD_ON);
            LCD_turnOn();
        }
        //////////////////////////////////////////////////////////////////////////
        // ������ : ������� � ���� ���� ������ ������ OK/MENU
        //////////////////////////////////////////////////////////////////////////
        if (!BIT_READ(PIND,BUTTON_OK)) // ���� ������ ������ OK/MENU                        //TODO: � ������ P-save ������ OK/MENU ������ ������ �� ���������� INT1
        {
            BIT_OFF(progFlags, INACTIVE); // ����� �� ������ ������������
            if (!BIT_READ(progFlags, LCD_ON))
            {
                BIT_ON(progFlags, LCD_ON); // �������� ��������� �������
                LCD_turnOn();
            }
            BIT_ON(progFlags, MENU_ON); // �������� ����
            menuRun(); // ��������� ������ ����
        }
        //////////////////////////////////////////////////////////////////////////
        // ������ : ��������� ��������� �� ��������� ��������, ��������
        //////////////////////////////////////////////////////////////////////////
        if(BIT_READ(progFlags, INACTIVE))
        {
            if(BIT_READ(progFlags, LCD_ON))
            {
                BIT_OFF(progFlags, LCD_ON);
                LCD_turnOff();
            }
            if (BIT_READ(progFlags, INACTIVE))
            {
                turnOnSleep();
            }
        }
        //////////////////////////////////////////////////////////////////////////
        // ������ : ��������� ������ ���� ��������� ��������
        //////////////////////////////////////////////////////////////////////////
        if (BIT_READ(progFlags, LCD_ON))
        {
            //LCD_Clear();
            LCD_DisplayAll();
        }
    }
}
//////////////////////////////////////////////////////////////////////////

/// ����������� ����������///

ISR(ADC_vect){                                                      //TODO: ������ ������ ��������� � ������ P-save
    //////////////////////////////////////////////////////////////////////////
    // ������ : ��������� �������� ������� � ��������� ���������
    //////////////////////////////////////////////////////////////////////////
    temperatureValue= BYTE_TO_TEMP((ADCH << 2));                           //TODO: ������ ���������� �� �����������
    if (temperatureValue >= (targetTemp + Tolerance))
    {
        turnOnCooler(); // �������� ����������
    }
    else if(temperatureValue <= targetTemp)
    {
        turnOffCooler(); // ��������� ����������
    }
}

ISR(TIMER2_OVF_vect){                                               //TODO: ������ ������ ��������� � ������ P-save
    runSeconds++;
    if (runSeconds==timeOut)
    {
        runSeconds= 0; // ���������� ������� ������
        BIT_ON(progFlags, INACTIVE);
    }
    return;
}

ISR(TIMER0_COMPA_vect){                                             //TODO: ������ ������ ��������� � ������ P-save
    return;
}

ISR(INT1_vect){                                                     //TODO: ������ ������ ��������� � ������ P-save
    turnOffSleep();
    BIT_ON(progFlags, LCD_ON);
    LCD_turnOn();
}