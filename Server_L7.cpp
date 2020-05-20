#include "COMPort.h"
void CLIENT_COM2()
{
	HANDLE readEnd = CreateEvent(nullptr, FALSE, FALSE, L"readEnd");
	HANDLE writeEnd = CreateEvent(nullptr, FALSE, FALSE, L"writeEnd");
	HANDLE hExit = CreateEvent(nullptr, FALSE, FALSE, L"exit");
	HANDLE hWork = CreateEvent(nullptr, FALSE, FALSE, L"work");

	if (hExit == nullptr || hWork == nullptr || readEnd == nullptr || writeEnd == nullptr)
		return;

	const comPort port(L"COM2", "COM2");
	startMessageTranistion(port, readEnd, writeEnd, hExit, hWork);
}
int main()
{
	CLIENT_COM2();
	system("pause");
}
