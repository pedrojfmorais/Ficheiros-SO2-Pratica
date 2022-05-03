#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define SHM_NAME _T("SHM_PC") //nome memória partilhada
#define MUTEX_NAME _T("MUTEX") // nome mutex
#define SEM_WRITE_NAME _T("SEM_WRITE") // nome semaforo escrita
#define SEM_READ_NAME _T("SEM_READ") // nome semaforo leitura
#define BUFFER_SIZE 10

typedef struct _BufferCell {
	unsigned int id; //id
	unsigned int val; // valor
}BufferCell;

typedef struct _MSG {
	unsigned int p; //contador numero produtores
	unsigned int c; //contador numero consumidores
	unsigned int wP; //posicao buffer circular para escrita
	unsigned int rP; //posicao buffer circular para leitura
	BufferCell buffer[BUFFER_SIZE]; //buffer circular
} SharedMem;

typedef struct _ControlData {
	unsigned int shutdown; //trinco
	unsigned int id; //id
	unsigned int count; //contador de numero de vezes
	HANDLE hMapFile;
	SharedMem* sharedMem;
	HANDLE hMutex;
	HANDLE hSemaforoLeitura;
	HANDLE hSemaforoEscrita;
} ControlData;

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
		cdata->sharedMem->p = 0;
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

	cdata->hSemaforoEscrita = CreateSemaphore(
		NULL,
		BUFFER_SIZE,
		BUFFER_SIZE,
		SEM_WRITE_NAME
	);
	if (cdata->hSemaforoEscrita == NULL) {
		_tprintf(_T("ERROR: CreateSemaphore (%d)\n"), GetLastError());
		UnmapViewOfFile(cdata->sharedMem);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hMutex);
		return FALSE;
	}

	cdata->hSemaforoLeitura = CreateSemaphore(
		NULL,
		0,
		BUFFER_SIZE,
		SEM_READ_NAME
	);
	if (cdata->hSemaforoLeitura == NULL) {
		_tprintf(_T("ERROR: CreateSemaphore (%d)\n"), GetLastError());
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hMutex);
		CloseHandle(cdata->hSemaforoEscrita);
		return FALSE;
	}
}

DWORD WINAPI consume(LPVOID p) {

	ControlData* cdata = (ControlData*)p;

	while (!cdata->shutdown) {

		WaitForSingleObject(cdata->hSemaforoLeitura, INFINITE);
		WaitForSingleObject(cdata->hMutex, INFINITE);

		_tprintf(_T("P%d produziu %d.\n"),
			cdata->sharedMem->buffer[cdata->sharedMem->rP].id,
			cdata->sharedMem->buffer[cdata->sharedMem->rP].val
		);

		(cdata->sharedMem->rP) += 1;

		if (cdata->sharedMem->rP == 10)
			cdata->sharedMem->rP = 0;

		ReleaseMutex(cdata->hMutex);

		(cdata->count)++;
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
		_tprintf(_T("Error creating/opening shared memory and synchronize.\n"));
		exit(1);
	}

	WaitForSingleObject(cdata.hMutex, INFINITE);
	cdata.id = ++(cdata.sharedMem->c);
	ReleaseMutex(cdata.hMutex);

	hThread = CreateThread(NULL, 0, consume, &cdata, 0, NULL);
	_tprintf(_T("Type in 'exit' to leave.\n"));

	do {
		_getts_s(command, 100);
	} while (_tcscmp(command, _T("exit")) != 0);

	cdata.shutdown = 1; //altera o trinco para terminar a thread

	WaitForSingleObject(hThread, INFINITE);
	_tprintf(_T("C%d consumed %d itens.\n"), cdata.id, cdata.count);

	CloseHandle(hThread);
	UnmapViewOfFile(cdata.sharedMem);
	CloseHandle(cdata.hMapFile);
	CloseHandle(cdata.hMutex);
	CloseHandle(cdata.hSemaforoEscrita);
	CloseHandle(cdata.hSemaforoLeitura);

	return 0;
}
