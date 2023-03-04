#pragma once

#define IOCTL_THREAD_PRIORITY CTL_CODE(0X8000, 0X800, METHOD_BUFFERED, FILE_READ_DATA)

struct PriorityData
{
	HANDLE hThread;
	int newPriority;
};