#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define SHM_NAME _T("fmMsgSpace")
#define SEMAPHORE_NAME _T("SEMAFORO_EX2_B")
#define MUTEX_NAME _T("RWMUTEX")
#define EVENT_NAME _T("NEWMSG")
#define MSGTEXT_SZ 100
#define N 5 //máximo utilizadores ativos em simultaneo

typedef struct _MSG {
	TCHAR szMessage[MSGTEXT_SZ];
} SharedMsg;

#define MSGBUFSIZE sizeof(SharedMsg)

typedef struct _ControlData {
	HANDLE hMapFile;
	SharedMsg* sharedMsg;
	int threadMustContinue;
	HANDLE newMsg;
	HANDLE hRWMutex;
	HANDLE hSemaforo;
} ControlData;

BOOL initMemAndSync(ControlData* cdata) {

	cdata->hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		MSGBUFSIZE,
		SHM_NAME
	);

	if (cdata->hMapFile == NULL) {
		_tprintf(_T("ERROR: CreateFileMapping (%d)\n"), GetLastError());
		return FALSE;
	}

	cdata->sharedMsg = (SharedMsg*)MapViewOfFile(
		cdata->hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		MSGBUFSIZE
	);


	if (cdata->hMapFile == NULL) {
		_tprintf(_T("ERROR: MapViewOfFile (%d)\n"), GetLastError());
		CloseHandle(cdata->hMapFile);
		return FALSE;
	}

	cdata->hRWMutex = CreateMutex(
		NULL,
		FALSE,
		MUTEX_NAME
	);

	if (cdata->hRWMutex == NULL) {
		_tprintf(_T("ERROR: CreateMutex (%d)\n"), GetLastError());
		UnmapViewOfFile(cdata->sharedMsg);
		CloseHandle(cdata->hMapFile);
		return FALSE;
	}

	cdata->newMsg = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		EVENT_NAME
	);

	if (cdata->newMsg == NULL) {
		_tprintf(_T("ERROR: CreateEvent (%d)\n"), GetLastError());
		UnmapViewOfFile(cdata->sharedMsg);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hRWMutex);
		return FALSE;
	}

	cdata->hSemaforo = CreateSemaphore(
		NULL, 
		N, 
		N, 
		SEMAPHORE_NAME
	);
	if (cdata->hSemaforo == NULL) {
		_tprintf(_T("ERROR: CreateSemaphore (%d)\n"), GetLastError());
		UnmapViewOfFile(cdata->sharedMsg);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hRWMutex);
		CloseHandle(cdata->newMsg);
		return FALSE;
	}
}

BOOL WINAPI sendMsg(ControlData* pcd) {
	SharedMsg msg;
	while (1) {
		_getts_s(msg.szMessage, MSGTEXT_SZ);

		WaitForSingleObject(pcd->hRWMutex, INFINITE);

		CopyMemory(pcd->sharedMsg, &msg, sizeof(SharedMsg));
		ReleaseMutex(pcd->hRWMutex);

		if (_tcscmp(msg.szMessage, _T("exit")) == 0)
			pcd->threadMustContinue = 0;

		SetEvent(pcd->newMsg);
		Sleep(500);
		ResetEvent(pcd->newMsg);

		if (!pcd->threadMustContinue)
			break;
	}
	return TRUE;
}

DWORD WINAPI receiveMsg(LPVOID p) {
	ControlData* pcd = (ControlData*)p;
	SharedMsg msg;

	while (1) {
		WaitForSingleObject(pcd->newMsg, INFINITE);
		if (!pcd->threadMustContinue)
			return 0;
		WaitForSingleObject(pcd->hRWMutex, INFINITE);
		CopyMemory(&msg, pcd->sharedMsg, sizeof(SharedMsg));
		ReleaseMutex(pcd->hRWMutex);
		_tprintf(_T("Message Received: %s\n"), msg.szMessage);
		Sleep(1000);
	}
	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {


#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	HANDLE hThread;
	ControlData cdata;

	if (!initMemAndSync(&cdata)) {
		_tprintf(_T("Error creating/opening shared memory and synchronize"));
		exit(1);
	}
	_tprintf(_T("aqui\n"));
	WaitForSingleObject(cdata.hSemaforo, INFINITE);
	_tprintf(_T("aqui\n"));

	cdata.threadMustContinue = 1;
	hThread = CreateThread(NULL, 0, receiveMsg, &cdata, 0, NULL);
	_tprintf(_T("Send messages to other users. ..."));

	sendMsg(&cdata);
	_tprintf(_T("Client is exiting\n"));

	WaitForSingleObject(hThread, INFINITE);

	ReleaseSemaphore(cdata.hSemaforo, 1, NULL);

	CloseHandle(hThread);
	CloseHandle(cdata.hSemaforo);

	return 0;
}
