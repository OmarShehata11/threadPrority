#include <ntddk.h>
#include "ThreadPriorityHeader.h"

void UnloadRotine(PDRIVER_OBJECT);
NTSTATUS ReqCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS ChangeThreadPriority(PDEVICE_OBJECT, PIRP);


class AutoEnterLeave
{
	LPCSTR function;

public:	
	AutoEnterLeave(LPCSTR _function)
	{
		function = _function;
		KdPrint(("entering the function: %s\n", function));
	}

	void HelloOmar()
	{
		KdPrint(("hello omar\n"));
	}

	~AutoEnterLeave()
	{
		KdPrint(("leaving the function: %s\n", function));
	}
};

#define AUTO_MACRO() AutoEnterLeave omar(__FUNCTION__) // or u can use __FUNCTION__ instead of __func__.

extern "C" 
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING)
{
	AUTO_MACRO();
	
	omar.HelloOmar();
	
	PDEVICE_OBJECT DeviceObject;
	UNICODE_STRING DeviceName, SympolicLinkName;

	RtlInitUnicodeString(&DeviceName, L"\\Device\\OmarDevice");
	RtlInitUnicodeString(&SympolicLinkName, L"\\??\\OmarSympolicLink");

	NTSTATUS status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, 0, false, &DeviceObject);

	if (status != STATUS_SUCCESS)
	{
		KdPrint(("[-]ERROR CREATING DEVICE : %d", status));
		return status;
	}

	IoCreateSymbolicLink(&SympolicLinkName, &DeviceName);
	
	DriverObject->DriverUnload = UnloadRotine;

	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverObject->MajorFunction[IRP_MJ_CREATE] = ReqCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ChangeThreadPriority;

	return STATUS_SUCCESS;
}

void UnloadRotine(PDRIVER_OBJECT DriverObject)
{
	AUTO_MACRO();

	UNICODE_STRING SympolicLinkName;
	
	RtlInitUnicodeString(&SympolicLinkName, L"\\??\\OmarSympolicLink");
	
	IoDeleteSymbolicLink(&SympolicLinkName);
	IoDeleteDevice(DriverObject->DeviceObject);
}


NTSTATUS ReqCreateClose(PDEVICE_OBJECT, PIRP Irp)
{
	AUTO_MACRO();
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS ChangeThreadPriority(PDEVICE_OBJECT, PIRP Irp)
{
	AUTO_MACRO();
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	auto stack = IoGetCurrentIrpStackLocation(Irp);

	switch (stack->Parameters.DeviceIoControl.IoControlCode)
	{ 
	case IOCTL_THREAD_PRIORITY:
		if (stack->Parameters.DeviceIoControl.InputBufferLength != sizeof(PriorityData))
		{
			KdPrint(("you size is wrong fucker\n exiting"));
			break;
		}
		PriorityData* data = (PriorityData*)Irp->AssociatedIrp.SystemBuffer;
		PKTHREAD ThreadRef;
		status = ObReferenceObjectByHandle(data->hThread, THREAD_SET_INFORMATION, *PsThreadType, UserMode, (PVOID*)&ThreadRef, NULL);
		if (status != STATUS_SUCCESS)
		{
			KdPrint(("can't get the reference object baby\n"));
			break;
		}
		KeSetPriorityThread(ThreadRef, data->newPriority);
		KdPrint(("[+]SUCCESS, THE changed to %d\n", data->newPriority));
		status = STATUS_SUCCESS;
		ObDereferenceObject(ThreadRef);
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

/*
NOTES UNDER THE HOOD:
	// SEARCH FOR THE LPCSTR DATA TYPE & __FUNCTION__ .
	
	LPCSTR ==> LONG POINTER CONSTANT STRING
	LPCTSTR => LONG POINTER CONSTANT TCHAR STRING
	LPTSTR ==> LONG POINTER TCHAR STRING

	(TCHAR) CHARACTER OF THE UNICODE
	
	**********************************
	__func__ is an implicitly declared identifier that expands to a character array variable containing the function name 
	when it is used inside of a function. It was added to C in C99. From C99 §6.4.2.2/1:

	The identifier __func__ is implicitly declared by the translator as if, immediately following the 
	opening brace of each function definition, the declaration

	static const char __func__[] = "function-name";
	appeared, where function-name is the name of the lexically-enclosing function.
	This name is the unadorned name of the function.

	Note that it is not a macro and it has no special meaning during preprocessing.

	__func__ was added to C++ in C++11, where it is specified as containing "an implementation-deﬁned string" 
	(C++11 §8.4.1[dcl.fct.def.general]/8), which is not quite as useful as the specification in C. 
	(The original proposal to add __func__ to C++ was N1642).

	__FUNCTION__ is a pre-standard extension that some C compilers support (including gcc and Visual C++); in general, 
	you should use __func__ where it is supported and only use __FUNCTION__ if you are using a compiler that does not support it 
	(for example, Visual C++, which does not support C99 and does not yet support all of C++0x, does not provide __func__).

	__PRETTY_FUNCTION__ is a gcc extension that is mostly the same as __FUNCTION__, 
	except that for C++ functions it contains the "pretty" name of the function including the signature of the function. 
	Visual C++ has a similar (but not quite identical) extension, __FUNCSIG__.
	
	
*/


	