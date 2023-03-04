#include <iostream>
#include <Windows.h>
#include <string.h>
#include <tchar.h>
#include "../threadPrority/ThreadPriorityHeader.h"

using namespace std;

void usage(string);
void Error();

int main(int argc, char ** argv)
{
	if (argc < 3)
		usage(argv[0]);


	cout << "hello omar\n" << endl;
	HANDLE hDevice = CreateFileA("\\\\.\\OmarSympolicLink", GENERIC_ALL, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		cout << "error opening the handle, code :" << GetLastError() << endl;
		return 1;
	}

	cout << "handle opened succefuly with value of : " << hDevice << endl;

	PriorityData* Data = { 0 };
	Data->hThread = OpenThread(THREAD_SET_INFORMATION, TRUE, atoi(argv[1]));
	if (Data->hThread == INVALID_HANDLE_VALUE)
		Error();

	Data->newPriority = atoi(argv[2]);

	DWORD bytesReturend;
	int success = DeviceIoControl(hDevice, IOCTL_THREAD_PRIORITY, &Data, sizeof(Data), nullptr, 0, &bytesReturend, nullptr);
	
	if (!success)
		Error();

	return 0;
}

void usage(string AppName)
{
	cout << "USAGE: " << AppName << " <hThread> <newPriority>" << endl;
	exit(1);
}

void Error()
{
	cout << "Error : " << GetLastError() << endl;
	exit(1);
}