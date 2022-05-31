#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#define BUFF_SIZE 256

#define PIPE_NAME TEXT("\\\\.\\pipe\\teste")
#define N_LEITORES 10

typedef struct {
	HANDLE hInstancia;
	OVERLAPPED overlap;
	BOOL activo;
}DadosPipe;

typedef struct tdata {
	DadosPipe hPipes[N_LEITORES];
	HANDLE hEvents[N_LEITORES];
	HANDLE hMutex;
	int terminar;
}ThreadDados;

DWORD WINAPI escritor(LPVOID p) {
	ThreadDados* dados = (ThreadDados*)p;
	TCHAR buf[BUFF_SIZE];
	DWORD n;

	_tprintf(TEXT("[ESCRITOR] Esperar ligação de um leitor...(ConnectNamedPipe)\n"));

	do {
		_tprintf(TEXT("[ESCRITOR] Frase: "));
		_fgetts(buf, 256, stdin);
		buf[_tcslen(buf) - 1] = '\0';
		for (int i = 0; i < N_LEITORES; ++i) {
			WaitForSingleObject(dados->hMutex, INFINITE);
			if (dados->hPipes[i].activo) {
				if (!WriteFile(dados->hPipes[i].hInstancia, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
					_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
					exit(-1);
				}
			}
			ReleaseMutex(dados->hMutex);
		}
		_tprintf(TEXT("[ESCRITOR] Enviei %d bytes ao leitor... (WriteFile)\n"), n);
	} while (_tcscmp(buf, TEXT("fim")));

	_tprintf(TEXT("[ESCRITOR] Desligar o pipe (DisconnectNamedPipe)\n"));

	dados->terminar = 1;

	for (int i = 0; i < N_LEITORES; i++)
		SetEvent(dados->hEvents[i]);

	return 0;
}

int _tmain(int argc, LPTSTR argv[]) {
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	DWORD numClientes = 0, offset, nBytes;

	TCHAR buf[BUFF_SIZE];
	
	ThreadDados tdata;
	
	tdata.terminar = 0;

	HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro a criar o mutex\n"));
		return FALSE;
	}
	tdata.hMutex = hMutex;

	for (int i = 0; i < N_LEITORES; ++i) {

		HANDLE hPipe;
		HANDLE hEventTemp;

		_tprintf(TEXT("[ESCRITOR] Criar uma cópia do pipe '%s' ...(CreateNamedPipe)\n"), PIPE_NAME);
		hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED, 
			PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE
			, N_LEITORES, sizeof(buf), sizeof(buf), 1000, NULL
		);
		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
			exit(-1);
		}

		hEventTemp = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (hEventTemp == NULL) {
			_tprintf(TEXT("[ERRO] Criar Event! (CreateEvent)"));
			exit(-1);
		}
 
		ZeroMemory(&tdata.hPipes[i].overlap, sizeof(&tdata.hPipes[i].overlap));
		tdata.hPipes[i].hInstancia = hPipe;
		tdata.hPipes[i].overlap.hEvent = hEventTemp;
		tdata.hPipes[i].activo = FALSE;
		tdata.hEvents[i] = hEventTemp;

		if (ConnectNamedPipe(hPipe, &tdata.hPipes[i].overlap)) {
			_tprintf(TEXT("[ERRO] Ligação ao leitor! (ConnectNamedPipe\n"));
			exit(-1);
		}
	}

	HANDLE hThread = CreateThread(NULL, 0, escritor, &tdata, 0, NULL);
	if (hThread == NULL) {
		_tprintf(TEXT("[ERRO] Criar Thread! (CreateThread\n"));
		exit(-1);
	}

	while (!tdata.terminar && numClientes < N_LEITORES) {
		offset = WaitForMultipleObjects(N_LEITORES, tdata.hEvents, FALSE, INFINITE);
		DWORD i = offset - WAIT_OBJECT_0;

		if (i >= 0 && i < N_LEITORES) {
			if (GetOverlappedResult(tdata.hPipes[i].hInstancia, &tdata.hPipes[i].overlap, &nBytes, FALSE)) {
				ResetEvent(tdata.hEvents[i]);
				WaitForSingleObject(tdata.hMutex, INFINITE);
				tdata.hPipes[i].activo = TRUE;
				ReleaseMutex(tdata.hMutex);
			}
			numClientes++;
		}
	}

	WaitForSingleObject(hThread, INFINITE);

	Sleep(2000);
	for (int i = 0; i < N_LEITORES; ++i) {
		if (!DisconnectNamedPipe(tdata.hPipes[i].hInstancia)) {
			_tprintf(TEXT("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
			exit(-1);
		}
		CloseHandle(tdata.hPipes[i].hInstancia);
	}
	exit(0);
}