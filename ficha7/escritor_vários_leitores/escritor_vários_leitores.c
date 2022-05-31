#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#define TAM 5
#define BUFF_SIZE 5
#define PIPE_NAME TEXT("\\\\.\\pipe\\teste")
#define N_LEITORES 2

typedef struct tdata {
	int tam_leitores;
	HANDLE array_np[TAM], hMutex;


}TDATA;

DWORD WINAPI escritor(LPVOID p) {
	TDATA* dados = (TDATA*)p;
	TCHAR buf[BUFF_SIZE];
	DWORD n;

	_tprintf(TEXT("[ESCRITOR] Esperar ligação de um leitor...(ConnectNamedPipe)\n"));

	do {
		_tprintf(TEXT("[ESCRITOR] Frase: "));
		_fgetts(buf, 256, stdin);
		buf[_tcslen(buf) - 1] = '\0';
		WaitForSingleObject(dados->hMutex, INFINITE);
		for (int i = 0; i < dados->tam_leitores; ++i) {
			if (!WriteFile(dados->array_np[i], buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
				exit(-1);
			}
		}
		ReleaseMutex(dados->hMutex);
		_tprintf(TEXT("[ESCRITOR] Enviei %d bytes ao leitor... (WriteFile)\n"), n);
	} while (_tcscmp(buf, TEXT("fim")));
	_tprintf(TEXT("[ESCRITOR] Desligar o pipe (DisconnectNamedPipe)\n"));

	return 1;
}

int _tmain(int argc, LPTSTR argv[]) {
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif


	TCHAR buf[256];
	HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro a criar o mutex\n"));
		return FALSE;
	}
	TDATA tdata;
	for (int i = 0; i < N_LEITORES; ++i) {
		_tprintf(TEXT("[ESCRITOR] Criar uma cópia do pipe '%s' ...(CreateNamedPipe)\n"), PIPE_NAME);
		tdata.array_np[i] = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_OUTBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 2, sizeof(buf), sizeof(buf), 1000, NULL);
		if (tdata.array_np[i] == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
			exit(-1);
		}
		if (!ConnectNamedPipe(tdata.array_np[i], NULL)) {
			_tprintf(TEXT("[ERRO] Ligação ao leitor! (ConnectNamedPipe\n"));
			exit(-1);
		}
	}
	tdata.hMutex = hMutex;
	tdata.tam_leitores = N_LEITORES;

	HANDLE hThread = CreateThread(NULL, 0, escritor, &tdata, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);

	Sleep(2000);
	for (int i = 0; i < N_LEITORES; ++i) {
		if (!DisconnectNamedPipe(tdata.array_np[i])) {
			_tprintf(TEXT("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
			exit(-1);
		}
		CloseHandle(tdata.array_np[i]);
	}
	exit(0);
}