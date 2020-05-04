#include <io.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
 
char date[6]; // данные часов
unsigned int delayTime = 0;
const unsigned int registerArray[] = { 0x00, 0x02, 0x04, 0x07, 0x08, 0x09 };
struct VIDEO
{
	unsigned char symb;
	unsigned char attr;
};
VIDEO  far* screen = (VIDEO far*)MK_FP(0xB800, 0);
 
void interrupt newTime(...);  // новый обработчик прерываний часов
void interrupt newAlarm(...); // новый обработчик прерываний будильника
void interrupt(*lastTime)(...); // старое прерывание часов
void interrupt(*lastAlarm) (...); // старое прерывание будильника

const char* months[] =
    {
        "JANUARY",
        "FEBRUARY",
        "MARCH",
        "APRIL",
        "MAY",
        "JUNE",
        "JULY",
        "AUGUST",
        "SEPTEMBER",
        "OCTOBER",
        "NOVEMBER",
        "DECEMBER"
    };
 
void menu();
void setTime();
void showTime();
void setAlarm();
void enterTime();
void resetAlarm();
void delay(unsigned int);
int convertToDecimal(int);
int getValueBetweenAsBCD(int, int, const char*);
 
int main()
{
    while (1)
    {
        menu();
        switch (getch())
        {
        case '1':
            system("cls");
            showTime();
            break;
 
        case '2':
            system("cls");
            delay(convertToDecimal(getValueBetweenAsBCD(0, 10000, "ENTER DELAY(MS): ")));
            break;
        case '3':
            system("cls");
            setAlarm();
            break;
        case '4':
            system("cls");
            setTime();
            break;
        case '0':
            return 0;
        default:
            system("cls");
            break;
        }
    }
}
 
void menu()
{
    puts("1. SHOW TIME");
    puts("2. SET DELAY");
    puts("3. SET ALARM");
    puts("4. SET TIME");
    puts("0. EXIT");
   }
 
void showTime(){
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        outp(0x70, registerArray[i]); // выбор адреса в памяти CMOS
        date[i] = inp(0x71); // считывание значения по адресу в массив
    }
 
    int decimalDate[6]; // перевод значений в десятичный вид
    for (i = 0; i < 6; i++)
    {
        decimalDate[i] = convertToDecimal(date[i]);
    }
 
    // вывод на экран
    printf("%2d:%2d:%2d\n", decimalDate[2], decimalDate[1], decimalDate[0]); // час, минута, секунда
    printf("%2d %s 20%2d\n", decimalDate[3], months[decimalDate[4] - 1], decimalDate[5]); // день, месяц, год
}
 
void setTime()
{
    enterTime();
    disable(); // запрет на прерывание
   
    do
    {
        outp(0x70, 0xA); // выбор регистра А
    } while (inp(0x71) & 0x80); // Если 7й бит установлен в 0, то это значит, что данные готовы для чтения
 
    //Остановка счётчика RTC
    outp(0x70, 0xB);
    outp(0x71, inp(0x71) | 0x80); // Если 7й бит установлен в 1, то счётчик остановлен
 
    for (int i = 0; i < 6; i++)
    {
        outp(0x70, registerArray[i]);
        outp(0x71, date[i]);
    }
 
    outp(0x70, 0xB);
    outp(0x71, inp(0x71) & 0x7F); // Если 7й бит установлен в 0, то счётчик запущен
 
    enable(); // разрешение на прерывание
    system("cls");
}

void delay(unsigned int delayms)
{
    // установка нового обработчика прерываний
    disable();
    lastTime = getvect(0x70);
    setvect(0x70, newTime);
    enable();
 
    // Размаскировка линии сигнала запроса от RTC
    outp(0xA1, inp(0xA1) & 0xFE); // 
    // 0-й бит в 0 для разрешения прерывания от ЧРВ 
 
    outp(0x70, 0xB); // выбор регистра В
    outp(0x71, inp(0x71) | 0x40); // Если 6й бит установлен в 1, то разрешаем периодические прерывания
 
    delayTime = 0;
    while (delayTime <= delayms);
    puts("DELAY IS OVER");
    setvect(0x70, lastTime);
}
 
void setAlarm()
{
    enterTime();
    disable();
 
    do{
        outp(0x70, 0xA);
    } while (inp(0x71) & 0x80); // Если 7й бит установлен в 0, то это значит, что данные готовы для чтения
 
    // установка часов в регистр будильника
    outp(0x70, 0x05);
    outp(0x71, date[2]);
 
    // установка минут в регистр будильника
    outp(0x70, 0x03);
    outp(0x71, date[1]);
 
    // установка секунд в регистр будильника
    outp(0x70, 0x01);
    outp(0x71, date[0]);
 
    outp(0x70, 0xB); // выбор регистра B
    outp(0x71, (inp(0x71) | 0x20)); //Если 5й бит установлен в 1, то разрешаем прерывания будильника
 
    // переопределение прерывания будильника
    lastAlarm = getvect(0x4A);
    setvect(0x4A, newAlarm);
    outp(0xA1, (inp(0xA0) & 0xFE)); // Если 0й бит в 0, то разрешаем прерывания от RTC
 
    enable();
    puts("ALARM IS ON");
}
 
void resetAlarm()
{
    if (lastAlarm == NULL)
        return;
 
    disable();
 
    // возврат старого прерывания
    setvect(0x4A, lastAlarm);
    outp(0xA1, (inp(0xA0) | 0x01)); // 0x01 - 0000 0001 (пересчет частоты прерывания)
 
    // проверка на доступность значений для чтения/записи
    do{
        outp(0x70, 0xA);
    } while (inp(0x71) & 0x80); // Если 7й бит установлен в 0, то это значит, что данные готовы для чтения
 
    outp(0x70, 0x05); // 0x05 - часы
    outp(0x71, 0x00);
 
    outp(0x70, 0x03); // 0x03 - минуты
    outp(0x71, 0x00);
 
    outp(0x70, 0x01); // 0x01 - секунды
    outp(0x71, 0x00);
 
    outp(0x70, 0xB);
    outp(0x71, (inp(0x71) & 0xDF)); //Если 5й бит установлен в 0, то запрещаем прерывания будильника
 
    enable();
}
int getValueBetweenAsBCD(int min, int max, const char* message)
{
    int value;
    do
    {
       rewind(stdin);
       printf(message);
       scanf("%d", &value);
    } while (value > max || value < min);
    return value / 10 * 16 + value % 10;
}
void enterTime()
{
    date[5] = getValueBetweenAsBCD(0, 2100, "ENTER YEAR: ");
    date[4] = getValueBetweenAsBCD(1, 12, "ENTER MONTH: ");
    date[3] = getValueBetweenAsBCD(1, 365, "ENTER DAY: ");
    date[2] = getValueBetweenAsBCD(0, 23, "ENTER HOUR: ");
    date[1] = getValueBetweenAsBCD(0, 59, "ENTER MINUTE: ");
    date[0] = getValueBetweenAsBCD(0, 59, "ENTER SECOND: ");
}
 
int convertToDecimal(int BCD)
{
    return ((BCD / 16 * 10) + (BCD % 16));
}

void interrupt newTime(...) // новый обработчик прерываний часов
{
    delayTime++;
    outp(0x70, 0xC);
    inp(0x71);
    
    // посыл сигнала контроллерам прерываний об окончании прерывания
    outp(0x20, 0x20);
    outp(0xA0, 0x20);
}

void interrupt newAlarm(...) // новый обработчик прерываний будильника
{
    const char[8] ALARM = "ALARM!!!";
    for (int i = 0; i < 8; i++, screen++)
    {
        screen->symb = ALARM[i];
        screen->attr = 0x5Eh;
    }
    
    lastAlarm();
    resetAlarm();
}