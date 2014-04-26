/*
 * LCD.c
 *
 * Created: 4/25/2014 10:06:24 PM
 *  Author: Disgust
 */ 
 #include "LCD.h"

 void LCD_Init()
 {
    LCD_REG= (1 << LCD_EN)|(1 << LCD_RS); // ����������� �� �����
    LCD_REG|= (1 << LCD_D0)|(1 << LCD_D1)|(1 << LCD_D2)|(1 << LCD_D3); // ������ �� �����
    while(LCD_Busy());
    BIT_OFF(LCD_PORT, LCD_RS); // ���� 4 ����, 2 ������, ������ 5x8 �����
    BIT_ON(LCD_PORT, LCD_EN);
    BIT_WRITE(LCD_PORT, LCD_D3, 0);
    BIT_WRITE(LCD_PORT, LCD_D2, 0);
    BIT_WRITE(LCD_PORT, LCD_D1, 1);
    BIT_WRITE(LCD_PORT, LCD_D0, 0);
    BIT_OFF(LCD_PORT, LCD_EN);
    BIT_ON(LCD_PORT, LCD_EN);
    BIT_WRITE(LCD_PORT, LCD_D3, 1);
    BIT_WRITE(LCD_PORT, LCD_D2, 0);
    BIT_WRITE(LCD_PORT, LCD_D1, 0);
    BIT_WRITE(LCD_PORT, LCD_D0, 0);
    BIT_OFF(LCD_PORT, LCD_EN);
    while(LCD_Busy());
    BIT_ON(LCD_PORT, LCD_EN); // ��������� ��������, ������ ������ ���
    BIT_WRITE(LCD_PORT, LCD_D3, 0);
    BIT_WRITE(LCD_PORT, LCD_D2, 0);
    BIT_WRITE(LCD_PORT, LCD_D1, 0);
    BIT_WRITE(LCD_PORT, LCD_D0, 0);
    BIT_OFF(LCD_PORT, LCD_EN);
    BIT_ON(LCD_PORT, LCD_EN);
    BIT_WRITE(LCD_PORT, LCD_D3, 0);
    BIT_WRITE(LCD_PORT, LCD_D2, 1);
    BIT_WRITE(LCD_PORT, LCD_D1, 1);
    BIT_WRITE(LCD_PORT, LCD_D0, 0);
    BIT_OFF(LCD_PORT, LCD_EN);
    void LCD_Clear();
 }

 void LCD_Clear()
 {
    while(LCD_Busy());
    BIT_OFF(LCD_PORT, LCD_RS);
    BIT_ON(LCD_PORT, LCD_EN);
    BIT_WRITE(LCD_PORT, LCD_D3, 0);
    BIT_WRITE(LCD_PORT, LCD_D2, 0);
    BIT_WRITE(LCD_PORT, LCD_D1, 0);
    BIT_WRITE(LCD_PORT, LCD_D0, 0);
    BIT_OFF(LCD_PORT, LCD_EN);
    BIT_ON(LCD_PORT, LCD_EN);
    BIT_WRITE(LCD_PORT, LCD_D3, 0);
    BIT_WRITE(LCD_PORT, LCD_D2, 0);
    BIT_WRITE(LCD_PORT, LCD_D1, 0);
    BIT_WRITE(LCD_PORT, LCD_D0, 1);
    BIT_OFF(LCD_PORT, LCD_EN);
 }

 void LCD_Write(char* data, uint8_t posY, uint8_t posX ) //TODO: ������������, ������ ���������� ����� � �������
 {
     LCD_SetCursor(posY, posX)
     //cli();
     while(LCD_Busy());
     BIT_OFF(LCD_PORT, LCD_RS);
     BIT_ON(LCD_PORT, LCD_EN);
     BIT_WRITE(LCD_PORT, LCD_D3, 0);
     BIT_WRITE(LCD_PORT, LCD_D2, 0);
     BIT_WRITE(LCD_PORT, LCD_D1, 0);
     BIT_WRITE(LCD_PORT, LCD_D0, 0);
     BIT_OFF(LCD_PORT, LCD_EN);
     BIT_ON(LCD_PORT, LCD_EN);
     BIT_WRITE(LCD_PORT, LCD_D3, 0);
     BIT_WRITE(LCD_PORT, LCD_D2, 0);
     BIT_WRITE(LCD_PORT, LCD_D1, 0);
     BIT_WRITE(LCD_PORT, LCD_D0, 1);
     BIT_OFF(LCD_PORT, LCD_EN);
     //sei();
 }

 void LCD_turnOn()
 {
     BIT_ON(CONTROL_PORT, LCD_LED);
     while(LCD_Busy());
     BIT_OFF(LCD_PORT, LCD_RS); // �������� �����, ������� ���
     BIT_ON(LCD_PORT, LCD_EN);
     BIT_WRITE(LCD_PORT, LCD_D3, 0);
     BIT_WRITE(LCD_PORT, LCD_D2, 0);
     BIT_WRITE(LCD_PORT, LCD_D1, 0);
     BIT_WRITE(LCD_PORT, LCD_D0, 0);
     BIT_OFF(LCD_PORT, LCD_EN);
     BIT_ON(LCD_PORT, LCD_EN);
     BIT_WRITE(LCD_PORT, LCD_D3, 1);
     BIT_WRITE(LCD_PORT, LCD_D2, 1);
     BIT_WRITE(LCD_PORT, LCD_D1, 0);
     BIT_WRITE(LCD_PORT, LCD_D0, 0);
     BIT_OFF(LCD_PORT, LCD_EN);
 }

 void LCD_turnOff()
 {
     BIT_OFF(CONTROL_PORT, LCD_LED);
     while(LCD_Busy());
     BIT_OFF(LCD_PORT, LCD_RS); // ��������� �����, ������� ���
     BIT_ON(LCD_PORT, LCD_EN);
     BIT_WRITE(LCD_PORT, LCD_D3, 0);
     BIT_WRITE(LCD_PORT, LCD_D2, 0);
     BIT_WRITE(LCD_PORT, LCD_D1, 0);
     BIT_WRITE(LCD_PORT, LCD_D0, 0);
     BIT_OFF(LCD_PORT, LCD_EN);
     BIT_ON(LCD_PORT, LCD_EN);
     BIT_WRITE(LCD_PORT, LCD_D3, 1);
     BIT_WRITE(LCD_PORT, LCD_D2, 0);
     BIT_WRITE(LCD_PORT, LCD_D1, 0);
     BIT_WRITE(LCD_PORT, LCD_D0, 0);
     BIT_OFF(LCD_PORT, LCD_EN);
 }
 
 void LCD_SetCursor(int row, int col)
 {  
    while(LCD_Busy());
    BIT_OFF(LCD_PORT, LCD_RS); // ������������ ������ �� ����� 0 � DDRAM
    BIT_ON(LCD_PORT, LCD_EN);                                           //TODO: ����������� ������ ������ � ������� � �����
    BIT_WRITE(LCD_PORT, LCD_D3, 1);
    BIT_WRITE(LCD_PORT, LCD_D2, 0);
    BIT_WRITE(LCD_PORT, LCD_D1, 0);
    BIT_WRITE(LCD_PORT, LCD_D0, 0);
    BIT_OFF(LCD_PORT, LCD_EN);
    BIT_ON(LCD_PORT, LCD_EN);
    BIT_WRITE(LCD_PORT, LCD_D3, 0);
    BIT_WRITE(LCD_PORT, LCD_D2, 0);
    BIT_WRITE(LCD_PORT, LCD_D1, 0);
    BIT_WRITE(LCD_PORT, LCD_D0, 0);
    BIT_OFF(LCD_PORT, LCD_EN);
 }

 int LCD_Busy()
 {
    LCD_REG&= ~(1 << LCD_D0)&~(1 << LCD_D1)&~(1 << LCD_D2)&~(1 << LCD_D3);
    LCD_PORT|= (1 << LCD_D0)|(1 << LCD_D1)|(1 << LCD_D2)|(1 << LCD_D3);
    BIT_ON(LCD_PORT, LCD_EN);
    // �����
    BIT_OFF(LCD_PORT, LCD_EN);
    BIT_ON(LCD_PORT, LCD_EN);
    // �����
    BIT_OFF(LCD_PORT, LCD_EN);
    if(BIT_READ(LCD_IN, LCD_D3))
    {
        LCD_REG|= (1 << LCD_D0)|(1 << LCD_D1)|(1 << LCD_D2)|(1 << LCD_D3);
        return 1;
    }
    LCD_REG|= (1 << LCD_D0)|(1 << LCD_D1)|(1 << LCD_D2)|(1 << LCD_D3);
    return 0;
 }
 