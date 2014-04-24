/*
 * AXO.c
 *
 * Created: 4/21/2014 8:00:01 PM
 *  Author: Disgust
 */ 

#define F_CPU 8000000UL
#define BAUD 115200UL
#define BAUD_DIVIDER ((F_CPU/(BAUD*8))-1)

///���������///
#define MAX_TEMP 20.0
#define MIN_TEMP 15.0
#define MAX_TOL 1.0
#define MIN_TOL 0.0

///�������///
#define BIT_ON(x,y) x|=(1<<y)
#define BIT_OFF(x,y) x&=~(1<<y)
#define BIT_READ(x,y) (((x)>>(y))&0x01)
#define BIT_WRITE(x,y,z) ((z)?(BIT_ON(x,y)):(BIT_OFF(x,y)))

#define HI(x) (x>>8)
#define LO(x) (x^0xFF)

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define CONSTRAIN(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define ROUND(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

#define BYTE_TO_VOLTS(x) ((x * 5.0)/256.0)
#define BYTE_TO_MILLIVOLTS(x) ((x * 5000.0)/256.0)
#define BYTE_TO_TEMP(x) ((BYTE_TO_MILLIVOLTS(x) - 500.0)/10.0) // �������, ����������� ��� ������� TMP036: T= (Vin(mV) - 500)/10

///���� B///
#define LCD_REG DDRB
#define LCD_PORT PORTB
#define LCD_D0 PINB0
#define LCD_D1 PINB1
#define LCD_D2 PINB2
#define LCD_D3 PINB3
#define LCD_EN PINB4
#define LCD_RS PINB5
///���� C///
#define SENSOR_REG DDRC
#define TEMP_SENSOR PINC0
///���� D///
#define CONTROL_REG DDRD
#define CONTROL_PORT PORTD
#define LCD_LED PIND7
#define LOAD PIND6
#define BUTTON_P PIND5
#define BUTTON_M PIND4
#define BUTTON_OK PIND3
#define BUTTON_BACK PIND2
///�����///
#define LCD_ON 0
#define MENU_ON 1
#define ECONOMY 2
#define COOLING 3
#define INACTIVE 4

#include <avr/io.h>
#include <avr/interrupt.h>

///���������� ����������///
volatile static uint8_t runSeconds;
uint8_t timeOut= 0x0A;
volatile static double temperatureValue;
volatile static double targetTemp= 20.0;
volatile static double Tolerance= 0.0;
volatile static uint16_t measureRate= 0x0100; // ����������� - ������� ������ (F_CPU/1024)/2
uint8_t progFlags= 0b00000000;

inline static void turnOnCooler() 
{
	BIT_ON(CONTROL_PORT, LOAD);
    BIT_ON(progFlags, COOLING);
}

inline static void turnOffCooler() 
{
	BIT_OFF(CONTROL_PORT, LOAD);
    BIT_OFF(progFlags, COOLING);
}

void turnOnPowerSave()
//TODO: ��������� ���������� INT1
{
    //BIT_ON(PRR, PRADC); // ����� ������ �� ���
    //BIT_ON(SMCR, SM0);
}

void turnOffPowerSave()
//TODO: ��������� ���������� INT1
{
    //BIT_OFF(PRR, PRADC);
    //BIT_OFF(SMCR, SM0);
}

void LCD_Clear()
{
    //TODO: ����������
}

void LCD_Write(uint8_t data, uint8_t posY, uint8_t posX ) 
{
    cli();
	//TODO: ����������
    sei();
}

void LCD_turnOn() 
{
	BIT_OFF(CONTROL_PORT, LCD_LED); // ????????? ?????????
}

void LCD_turnOff() 
{
	BIT_ON(CONTROL_PORT, LCD_LED); // �������� ��������� �������
}

void LCD_DisplayAll() 
{   
	LCD_Write("TEMP :", 0, 0);
    LCD_Write((uint8_t)temperatureValue, 0, 8);
    if (BIT_READ(progFlags, COOLING))
    {
        LCD_Write("COOLING ", 1, 0);
        LCD_Write((uint8_t)((temperatureValue - targetTemp)/Tolerance)*100, 1, 8);
    }
}

inline void menuStop()
{
    LCD_Clear();
    LCD_DisplayAll();
}

void menuRun()
{   int pos= 0;
    char menu[4][16]= {"Target temp  (1)", "Tolerance    (2)", "Measure rate (3)", "P-save mode  (4)"};
	uint8_t values[4]= {targetTemp, Tolerance, measureRate, (BIT_READ(progFlags, ECONOMY))};
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
                    values[pos]++;
                }
                if (!BIT_READ(CONTROL_PORT, BUTTON_M))
                {
                    BIT_OFF(progFlags, INACTIVE);
                    values[pos]--;
                }
                if (!BIT_READ(CONTROL_PORT, BUTTON_BACK)) break;
                switch (pos)
                {
                case 1:
                    if(value > MAX_TEMP) value= MIN_TEMP;
                    if(value < MIN_TEMP) value= MAX_TEMP;
                	break;
                case 2:
                    if(value > MAX_TOL) value= MIN_TOL;
                    if(value < MIN_TOL) value= MAX_TOL;
                	break;
                case 3:
                    if(value > 1023) value= 0;
                    if(value < 0) value= 1023;
                	break;
                case 4:
                    if(value > 1) value= 0;
                    if(value < 0) value= 1;
                	break;
                }
                if (!BIT_READ(CONTROL_PORT, BUTTON_OK)) {
                    if(pos!=3){
                        values[pos]= value;
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
        }
        if (!BIT_READ(CONTROL_PORT, BUTTON_M))
        {
            BIT_OFF(progFlags, INACTIVE);
            pos--;
        }
        if (!BIT_READ(CONTROL_PORT, BUTTON_BACK)) break;
        if(pos > 3) pos= 0;
        if(pos < 0) pos= 3;
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
    LCD_REG= (1 << LCD_D0)|(1 << LCD_D1)|(1 << LCD_D2)|(1 << LCD_D3)|(1 << LCD_EN)|(1 << LCD_RS); // LCD �� �����
    SENSOR_REG&= ~(1 << TEMP_SENSOR); // ����������� �� ����
    CONTROL_REG= (1 << LCD_LED)|(1 << LOAD); // ���������� ���������� ������ � ��������� �� �����
    CONTROL_REG&= ~(1 << BUTTON_M) & ~(1 << BUTTON_P) & ~(1 << BUTTON_OK) & ~(1 << BUTTON_BACK); // ������ �� ����
    CONTROL_PORT= (1 << BUTTON_M)|(1 << BUTTON_OK)|(1 << BUTTON_P)|(1 << BUTTON_BACK); // ���������� ������������� ��������� � �������
    //////////////////////////////////////////////////////////////////////////
    
    ///������������� ���///
    ADMUX |= 1<<REFS0; // ������� �������� �������� ���������� - ���� AVCC
    //ADCSRB |= 5 << ADTS0; // ������� ����� ������������ ��� - �� ���������� ������� 1 � ��������� B
    ADCSRA |= 6 << ADPS0; // ������� ������� ������� (������������) - F_CPU/ADPS = 8000000/64=125kHz
    ADMUX |= 1 << ADLAR; // ������������ ����������� �� ����� �������
    //ADCSRA |= 1 << ADATE; // �������� ����������� ��������������
    ADCSRA |= 1 << ADIE; // ��������� ���������� ���
    ADCSRA |= 1 << ADEN; // ��������� ������ ���
    //DIDR0 |= 1 << ADC0D; // ��������� �������� ���� ADC0D 
    //////////////////////////////////////////////////////////////////////////
    
    ///������������� ������� 1///
    TCCR1B|= 1 << WGM13; // �������� ����� CTC - ����� �������� �� ����������
    OCR1A= measureRate;
    TCCR1B |= 5 << CS10; // �������� ������ 1 � ������������� 1024
    TIMSK1 |= 1 << OCIE1A; // ��������� ���������� ������� �� ��������� � ��������� A
    //////////////////////////////////////////////////////////////////////////
    
    ///������������� ������������ ������� 2///
    TCCR2B= 5 << CS20; // �������� ������ 2 � ������������� 128 (1 ������������ � �������)
    ASSR|= (1 << AS2); // ��������� ����������� �����
    TIMSK2 |= 1 << TOIE2; // ��������� ���������� ������� �� ������������
    //////////////////////////////////////////////////////////////////////////
    
    sei();
    
    ///������� ����///
    while(1)
    {
        //////////////////////////////////////////////////////////////////////////
        // ������ 1: ���������� �������� ������� � ��������� ���������
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // ������ 3: �������� ��������� ���� ������ �����-���� ������
        //////////////////////////////////////////////////////////////////////////
        if ((!BIT_READ(PIND, BUTTON_M))||(!BIT_READ(PIND, BUTTON_P))||(!BIT_READ(PIND, BUTTON_BACK))) // ���� ������ ����� ������
        {
            BIT_OFF(progFlags, INACTIVE); // ����� �� ������ ������������
            BIT_ON(progFlags, LCD_ON);
            LCD_turnOn();
        }
        //////////////////////////////////////////////////////////////////////////
        // ������ 4: ������� � ���� ���� ������ ������ OK/MENU
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
        // ����� �� ���� � ��������� ��������� �� ��������� ��������
        //////////////////////////////////////////////////////////////////////////
        if(BIT_READ(progFlags, INACTIVE)){ // ???? ?? ???????
            if(BIT_READ(progFlags, LCD_ON)){ // ???? ????????? ????????
                BIT_OFF(progFlags, LCD_ON);
                LCD_turnOff();
            }
        }
        if ((BIT_READ(progFlags, ECONOMY))&&(BIT_READ(progFlags, INACTIVE)))
        {
            //SMCR |= 1 << SE; // ��������
        }
        //////////////////////////////////////////////////////////////////////////
        // ������ 5: ��������� ������ ���� ��������� ��������
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
    temperatureValue= BYTE_TO_TEMP(ADCH);
    if (temperatureValue >= targetTemp + Tolerance)
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
        BIT_ON(progFlags, INACTIVE);
    }
    runSeconds= 0; // ���������� ������� ������
    return;
}

ISR(TIMER1_COMPA_vect){                                             //TODO: ������ ������ ��������� � ������ P-save
    ADCSRA |= 1 << ADSC;
    return;
}

ISR(INT1_vect){                                                     //TODO: ������ ������ ��������� � ������ P-save
    turnOffPowerSave();
    LCD_turnOn();
    LCD_DisplayAll();
}