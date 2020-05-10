#include <dos.h>
#include <conio.h>
#include <stdio.h>

void interrupt(*oldInt9)(...);
const char indicatorsMask = 0x7;
int doResend = 1;
int escPressed = 0;
int blinkingPaused = 0;

void interrupt newInt9(...)
{
		oldInt9();
		unsigned char value = inp(0x60); //Получаем скан-код нажатой в текущий момент клавиши  (или код возврата)

		if (value == 0x01) //Если нажата клавиша Esc
            escPressed = 1;

		if (value == 0x26) //Если нажата клавиша L
            blinkingPaused = !blinkingPaused;

		doResend = !blinkingPaused && value != 0xFA; // (value != 0xFA) - если операция завершена c ошибкой и если моргание не на паузе

		printf("%X\n", value);

		outp(0x20, 0x20); //Сброс контроллера прерываний
}

void setMask(char mask)
{
		doResend = 1;
		while (doResend) //Если во время передачи произойдёт какая-либо ошибка, то необходимо повторить пересылку
		{
				while (inp(0x64) & 0x02); //Первый бит регистра состояния 0х64 установлен в 1 пока занят входной регистр контроллера клавиатуры. т.е. мы не можем записывать данные
				outp(0x60, 0xED); //ED - код для управления индикаторами клавиатуры
				delay(50);
		}
        doResend = 1;
		while (doResend) //Если во время передачи произойдёт какая-либо ошибка, то необходимо повторить пересылку
		{
				while (inp(0x64) & 0x02); //Первый бит регистра состояния 0х64 установлен в 1 пока занят входной регистр контроллера клавиатуры. т.е. мы не можем записывать данные
				outp(0x60, mask); //mask - маска индикаторов: (биты 7-3 не используются) | CapsLock | NumLock | ScrollLock
				delay(50);
		}
}

void main()
{
		oldInt9 = getvect(0x9);
		setvect(0x9, newInt9);
		while (!escPressed)
		{
				if (!blinkingPaused)
				{
                    for (int i = 0; i < 3; i++)
                    {
                        setMask(indicatorsMask);
						delay(1000);
						setMask(0);
						delay(400);
                    }
				}
		}
		setvect(0x9, oldInt9);
}