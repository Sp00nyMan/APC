#pragma once
#include <string>
#include <Windows.h>
#include <iostream>

class comPort
{
	HANDLE handle;
	const unsigned TIMEOUT = 1200;
	const unsigned BAUDRATE = 9600;
public:
	std::string name;

	comPort(LPCWSTR name, std::string name1)
	{
		this->name = name1;
		this->handle = CreateFile(name,
			GENERIC_READ | GENERIC_WRITE,
			0,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr);
		if (handle == INVALID_HANDLE_VALUE)
		{
			std::string error("Error creating file : ");
			if (GetLastError() == ERROR_PATH_NOT_FOUND)
				error += "Path not found!";
			throw std::exception(error.c_str());
		}

		SetupComm(handle, 1500, 1500);

		COMMTIMEOUTS CommTO;
		memset(&CommTO, 0, sizeof(CommTO));
		CommTO.ReadIntervalTimeout = UINT_MAX;
		CommTO.ReadTotalTimeoutMultiplier = 0;
		CommTO.ReadTotalTimeoutConstant = TIMEOUT;
		CommTO.WriteTotalTimeoutMultiplier = 0;
		CommTO.WriteTotalTimeoutConstant = TIMEOUT;
		if (!SetCommTimeouts(handle, &CommTO))
		{
			CloseHandle(handle);
			throw std::exception("Error setting timeouts");
		}

		DCB CommDCB;
		memset(&CommDCB, 0, sizeof(CommDCB));
		CommDCB.DCBlength = sizeof(DCB);
		CommDCB.BaudRate = BAUDRATE;
		CommDCB.fBinary = TRUE;
		CommDCB.fParity = TRUE;
		CommDCB.fOutxCtsFlow = TRUE;
		CommDCB.fOutxDsrFlow = TRUE;
		CommDCB.fDtrControl = DTR_CONTROL_HANDSHAKE;
		CommDCB.fDsrSensitivity = TRUE;
		CommDCB.fTXContinueOnXoff = TRUE;
		CommDCB.fInX = TRUE;
		CommDCB.fOutX = TRUE;
		CommDCB.fErrorChar = TRUE;
		CommDCB.fNull = TRUE;
		CommDCB.fRtsControl = RTS_CONTROL_HANDSHAKE;
		CommDCB.fAbortOnError = TRUE;
		CommDCB.fDummy2 = 17;
		CommDCB.XonLim = 128;
		CommDCB.XoffLim = 128;
		CommDCB.ByteSize = 8;
		CommDCB.Parity = NOPARITY;
		CommDCB.StopBits = ONESTOPBIT;
		CommDCB.XonChar = 0;
		CommDCB.XoffChar = static_cast<unsigned char>(0xFF);

		if (!SetCommState(handle, &CommDCB))
		{
			CloseHandle(handle);
			throw std::exception("Error setting state");
		}
	}

	std::string readData() const
	{
		DWORD readed = 0;
		char bufferSize = 0;
		char* buffer = nullptr;
		if (ReadFile(handle, &bufferSize, 1, &readed, nullptr))
		{
			buffer = static_cast<char*>(malloc(bufferSize + 1));

			if (!buffer)
				throw std::exception("Memory allocation Error");

			ReadFile(handle, buffer, bufferSize * sizeof(char), &readed, nullptr);
			buffer[readed] = '\0';

		}
		return buffer ? buffer : "";
	}

	void writeData(const std::string& data) const
	{
		if (handle == nullptr)
			throw std::exception("Invalid handle");
		DWORD written;
		char size = data.length();
		WriteFile(handle, &size, 1, &written, nullptr);
		WriteFile(handle, data.c_str(), size, &written, nullptr);
	}

	~comPort()
	{
		if (handle != nullptr)
			CloseHandle(handle);
	}
};

void startMessageTranistion(const comPort& port, HANDLE read_end, HANDLE write_end, HANDLE hExit, HANDLE hWork)
{
	while (WaitForSingleObject(hExit, 1) == WAIT_TIMEOUT)
	{
		if (WaitForSingleObject(hWork, 1) != WAIT_TIMEOUT)
		{
			std::cout << "Waiting for second client to finish writing...\n";
			WaitForSingleObject(write_end, INFINITE);
			if (WaitForSingleObject(hExit, 1) != WAIT_TIMEOUT)
				return;
			std::cout << "Message from "<< port.name << ": " << port.readData() << std::endl;
			SetEvent(read_end);
		}
		SetEvent(hWork);
		std::cout << "> ";
		std::string data;
		getline(std::cin, data);
		port.writeData(data);
		SetEvent(write_end);
		if (data == "exit")
		{
			SetEvent(hExit);
			return;
		}
		WaitForSingleObject(read_end, INFINITE);
	}
}