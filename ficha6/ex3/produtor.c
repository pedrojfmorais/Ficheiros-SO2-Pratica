#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>



#define SHM_NAME TEXT("SHM_PC")
#define MUTEX_NAME TEXT("MUTEX")
#define SEM_WRITE_NAME TEXT("SEM_WRITE")//semaforo escrita
#define SEM_READ_NAME TEXT("SEM_READ")//semaforo leitura
#define BUFFER_SIZE 10


typedef struct _BufferCell {
	unsigned int id;
	unsigned int val;
} BufferCell;

typedef struct _SharedMem {
	BufferCell buffer[BUFFER_SIZE];
	unsigned int p;//num produtores
	unsigned int c;//num consumidores
	unsigned int wP;//posição buffer escrita
	unsigned int rP;//posição buffer leitura

} SharedMem;

typedef struct _ControlData {
	unsigned int shutdown; //trinco
	unsigned int id; //id do processo
	unsigned int count; //num vezes
	//unsigned sum; // somatorio
	HANDLE hMapFile;
	SharedMem* sharedMem;
	HANDLE hWriteSem;
	HANDLE hMutex;
	HANDLE hReadSem;
} ControlData;
//escrita sinalizado max = min, leitura não sinalizado

BOOL initMemAndSync(ControlData* cdata) {

	cdata->hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(SharedMem),
		SHM_NAME
	);

	if (cdata->hMapFile == NULL) {
		_tprintf(_T("ERROR: CreateFileMapping (%d)\n"), GetLastError());
		return FALSE;
	}

	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		_tprintf(_T("asd"));
		cdata->sharedMem->p = 0;
		_tprintf(_T("asd"));
		cdata->sharedMem->c = 0;
		cdata->sharedMem->wP = 0;
		cdata->sharedMem->rP = 0;
	}

	cdata->sharedMem = (SharedMem*)MapViewOfFile(
		cdata->hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(SharedMem)
	);

	if (cdata->sharedMem == NULL) {
		_tprintf(_T("ERROR: MapViewOfFile (%d)\n"), GetLastError());
		CloseHandle(cdata->hMapFile);
		return FALSE;
	}

	cdata->hMutex = CreateMutex(
		NULL,
		FALSE,
		MUTEX_NAME
	);
	if (cdata->hMutex == NULL) {
		_tprintf(_T("ERROR: CreateMutex (%d)\n"), GetLastError());
		UnmapViewOfFile(cdata->sharedMem);
		CloseHandle(cdata->hMapFile);
		return FALSE;
	}

	cdata->hWriteSem = CreateSemaphore(
		NULL,
		BUFFER_SIZE,
		BUFFER_SIZE,
		SEM_WRITE_NAME
	);
	if (cdata->hWriteSem == NULL) {
		_tprintf(_T("ERROR: CreateSemaphore (%d)\n"), GetLastError());
		UnmapViewOfFile(cdata->sharedMem);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hMutex);
		return FALSE;
	}

	cdata->hReadSem = CreateSemaphore(
		NULL,
		0,
		BUFFER_SIZE,
		SEM_READ_NAME
	);
	if (cdata->hReadSem == NULL) {
		_tprintf(_T("ERROR: CreateSemaphore (%d)\n"), GetLastError());
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hMutex);
		CloseHandle(cdata->hWriteSem);
		return FALSE;
	}
}

DWORD WINAPI produce(LPVOID p) {

	ControlData* cdata = (ControlData*) p;

	while (!cdata->shutdown) {

		int num = rand();

		WaitForSingleObject(cdata->hWriteSem, INFINITE);
		WaitForSingleObject(cdata->hMutex, INFINITE);

		cdata->sharedMem->buffer[cdata->sharedMem->wP].id = cdata->id;
		cdata->sharedMem->buffer[cdata->sharedMem->wP].val = num;

		(cdata->sharedMem->wP) += 1;

		if (cdata->sharedMem->wP == 10)
			cdata->sharedMem->wP = 0;

		ReleaseMutex(cdata->hMutex);
		ReleaseSemaphore(cdata->hReadSem, 1, NULL);

		(cdata->count)++;

		Sleep(5000);
	}
}

int _tmain(int argc, TCHAR* argv[]) {


#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	HANDLE hThread;
	ControlData cdata;
	TCHAR command[100];

	srand((unsigned int)time(NULL));
	cdata.shutdown = 0; //trinco
	cdata.count = 0; //numero itens

	if (!initMemAndSync(&cdata)) {

		_tprintf(_T("Error creating/opening shared memory and synchronize"));
		exit(1);
	}

	WaitForSingleObject(cdata.hMutex, INFINITE);
	cdata.id = ++(cdata.sharedMem->p);
	ReleaseMutex(cdata.hMutex);

	hThread = CreateThread(NULL, 0, produce, &cdata, 0, NULL);
	_tprintf(_T("Type in 'exit' to leave.\n"));

	do {
		_getts_s(command, 100);
	} while (_tcscmp(command, _T("exit")) != 0);
	
	cdata.shutdown = 1; //altera o trinco para terminar a thread

	WaitForSingleObject(hThread, INFINITE);
	_tprintf(_T("P%d poduced %d itens.\n"), cdata.id, cdata.count);

	CloseHandle(hThread);
	UnmapViewOfFile(cdata.sharedMem);
	CloseHandle(cdata.hMapFile);
	CloseHandle(cdata.hMutex);
	CloseHandle(cdata.hReadSem);
	CloseHandle(cdata.hWriteSem);

	return 0;
}
