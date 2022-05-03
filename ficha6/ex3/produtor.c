#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>


/*
 Produtor:
 Espera ( pode_contar )
 id . nprod
 Assinala (Pode_contar)

 loop:
 1) Gera numero num
 2) Espera(pode_escrever)
 3) Espera(mutex)
 4) buffer [w] <- num
 5) w <- (++w) % N
 6) Assinala (mutex)
 7) Assinala (pode_ler)
 8) Espera


 Servidor:
 pode_contar = boolean
 pode_escrever = n
 pode_ler = N-n
 Mutex = boolean


 Consumidor:

 ...

 loop:
 1) Espera(pode_ler)
 2) x <- buffer[r]
 3) r <- (++r) % N
 4) Assinala ( pode_escrever )
 5) acumula x, conta
*/
#define BUFFER_SIZE 10
#define SHM_NAME TEXT("SHM_PC")
#define MUTEX_NAME TEXT("MUTEX")
#define SEM_WRITE_NAME TEXT("SEM_WRITE")
#define SEM_READ_NAME TEXT("SEM_READ")

typedef struct _BufferCell {
	unsigned int id; // id
	unsigned int val; // valor
} BufferCell;
typedef struct _SharedMem {
	unsigned int p; // Contador partilhado com o numero de produtores
	unsigned int c; // Contador partilhado com o numero de consumidores
	unsigned int wP; // Posição do buffer circular para escrita
	unsigned int rP; // Posição do buffer circular para leitura
	BufferCell buffer[BUFFER_SIZE]; // Buffer Circular
} SharedMem;

typedef struct _ControlData {
	unsigned int shutdown; // Trinco
	unsigned int id; // Id do processo
	unsigned int count; // Contador do numero de vezes
	unsigned int sum; // Somatório
	HANDLE hMapFile; // Ficheiro de memória
	SharedMem* sharedMem; // Memória Partilhada
	HANDLE hMutex; // Mutex
	HANDLE hWriteSem; // Semáforo escrita
	HANDLE hReadSem; // Semáforo leitura
} ControlData;

BOOL  initMemAndSync(ControlData* cdata) {
	BOOL firstProcess = FALSE;
	cdata->hMapFile = OpenFileMapping( // Se já existir
		FILE_MAP_ALL_ACCESS, // Read - Write access
		FALSE, // Se este parâmetro for TRUE, um processo criado pela função CreateProcess 
		SHM_NAME // Nome da memória partilhada
	);
	// Se ainda ninguém tiver criado a memória, ele vai criar
	if (cdata->hMapFile == NULL) {
		firstProcess = TRUE;
		_tprintf(TEXT("Criar novo fich mapeado"));
		cdata->hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			sizeof(SharedMem),
			SHM_NAME
		);
		if (cdata->hMapFile == NULL) {
			_tprintf(TEXT("ERRO A  Mapear, %d\n"), GetLastError());
			return FALSE;
		}
	}

	cdata->sharedMem = (SharedMem*)MapViewOfFile(cdata->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMem));
	if (cdata->sharedMem == NULL) {
		_tprintf(TEXT("ERRO A  Mapear, %d\n"), GetLastError());
		CloseHandle(cdata->hMapFile);
		return FALSE;
	}
	if (firstProcess) {
		cdata->sharedMem->p = 0; // Contador de produtores
		cdata->sharedMem->c = 0; // Contador de consumidores
		cdata->sharedMem->wP = 0; // posição de 0  a BUFFER_SIZE
		cdata->sharedMem->rP = 0; // posição de 0  a BUFFER_SIZE
	}
	// Criar o Mutex
	cdata->hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
	if (cdata->hMutex == NULL) {
		_tprintf(TEXT("ERRO A  Mutexar, %d\n"), GetLastError());
		UnmapViewOfFile(cdata->sharedMem);
		CloseHandle(cdata->hMapFile);
		return FALSE;
	}


	cdata->hReadSem = CreateSemaphore(NULL, 0, BUFFER_SIZE, SEM_READ_NAME);
	if (cdata->hReadSem == NULL) {
		_tprintf(TEXT("ERRO A criar semáforos1, %d\n"), GetLastError());
		UnmapViewOfFile(cdata->sharedMem);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hMutex);
		return FALSE;
	}

	cdata->hWriteSem = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, SEM_WRITE_NAME);
	if (cdata->hWriteSem == NULL) {
		_tprintf(TEXT("ERRO A criar semáforos2, %d\n"), GetLastError());
		UnmapViewOfFile(cdata->sharedMem);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hMutex);
		CloseHandle(cdata->hReadSem);
		return FALSE;
	}
	return TRUE;
}

DWORD WINAPI produce(LPVOID p) {
	ControlData* cData = (ControlData*)p;
	BufferCell cell;
	int r;
	while (1) {
		if (cData->shutdown)
			return 0;
		//Gerar 1 item
		r = rand() % 10 + 99;
		cell.id = cData->id; // id do produtor, incrementado no main
		cell.val = r; // Numero produzido
		WaitForSingleObject(cData->hWriteSem, INFINITE); // Produtor verifica se pode 
		WaitForSingleObject(cData->hMutex, INFINITE); // Inicio de zona critica

		CopyMemory(&(cData->sharedMem->buffer[(cData->sharedMem->wP)++]), &cell, sizeof(BufferCell)); // Copia para o buffer circular
		if (cData->sharedMem->wP == BUFFER_SIZE)
			cData->sharedMem->wP = 0; // Volta ao inicio

		ReleaseMutex(cData->hMutex); // Fim da zona critica
		ReleaseSemaphore(cData->hReadSem, 1, NULL); // Produtor assinala ao consumidor para ..

		_tprintf(TEXT("P%d produced %d. \n"), cell.id, cell.val);
		r = rand() % 4 + 2;
		Sleep(r * 1000);
		cData->count++; // Numero de itens
	}
	return 0;
}


int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 
	ControlData cData;
	HANDLE hThread;
	TCHAR command[100];

	srand((unsigned int)time(NULL));
	cData.shutdown = 0; // trinco
	cData.count = 0; // Numero de itens

	if (!initMemAndSync(&cData)) {
		_tprintf(TEXT("Error creating / Opening shared memory and sync mechanism"));
		exit(1);
	}
	WaitForSingleObject(cData.hMutex, INFINITE);
	cData.id = ++(cData.sharedMem->p); // Incrementa o contador partilhado com o num
	ReleaseMutex(cData.hMutex);
	hThread = CreateThread(NULL, 0, produce, &cData, 0, NULL);
	_tprintf(TEXT("Type in 'exit' to leave.\n"));
	do { _getts_s(command, 100); } while (_tcscmp(command, TEXT("exit")) != 0);
	cData.shutdown = 1; // Altera o trinco para que a thread termine
	WaitForSingleObject(hThread, INFINITE);
	_tprintf(TEXT("P%d produced %d items.\n"), cData.id, cData.count);
	CloseHandle(hThread);
	UnmapViewOfFile(cData.sharedMem);
	CloseHandle(cData.hMapFile);
	CloseHandle(cData.hMutex);
	CloseHandle(cData.hWriteSem);
	CloseHandle(cData.hReadSem);


	return 0;

}