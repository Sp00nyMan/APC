#include "C://Users//mrsto//YandexDisk//Учёба//4 семестр//APK//Lab7//COMPort.h"

void CLIENT_COM3()
{
	HANDLE readEnd = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"readEnd");
	HANDLE writeEnd = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"writeEnd");
	HANDLE hExit = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"exit");
	HANDLE hWork = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"work");

	if (hExit == nullptr || hWork == nullptr || readEnd == nullptr || writeEnd == nullptr)
		return;

	const comPort port(L"COM3", "COM3");
	startMessageTranistion(port, readEnd, writeEnd, hExit, hWork);

}

int main()
{
	CLIENT_COM3();
	system("pause");
}