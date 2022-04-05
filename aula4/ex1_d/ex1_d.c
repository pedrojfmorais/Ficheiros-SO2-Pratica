#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define N 2

typedef struct {

	int somatorio;
	int limInf;
	int limSup;

} Dados;

DWORD WINAPI pares(void* dados) {
	Dados* data = (Dados*)dados;

	int somados = 0;

	for (int i = data->limInf; i <= data->limSup; i++) {

		if (i % 2 == 0) {
			data->somatorio += i;
			somados++;

			if (somados % 200 == 0) {
				//Sleep(1);
				//_tprintf(_T("Uma pausa nos pares\n"));
			}
		}
	}
}
DWORD WINAPI primos(void* dados) {
	Dados* data = (Dados*)dados;

	int somados = 0;

	for (int i = data->limInf; i <= data->limSup; i++) {
		// Skip 0 and 1 as they are
		// neither prime nor composite
		if (i == 1 || i == 0)
			continue;

		// flag variable to tell
		// if i is prime or not
		DWORD flag = 1;

		for (int j = 2; j <= i / 2; ++j) {
			if (i % j == 0) {
				flag = 0;
				break;
			}
		}

		// flag = 1 means i is prime
		// and flag = 0 means i is not prime
		if (flag == 1) {
			data->somatorio += i;
			somados++;

			if (somados % 15 == 0) {
				//Sleep(1);
				//_tprintf(_T("Pausa dos primos\n"));
			}
		}
	}
}

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	Dados dados[N];

	HANDLE hThread[N];

	_tprintf(_T("Insira o limite inferior dos números pares: "));
	_tscanf_s(_T("%d"), &dados[0].limInf);

	do {
	_tprintf(_T("Insira o limite superior dos números pares: "));
	_tscanf_s(_T("%d"), &dados[0].limSup);
	} while (dados[0].limSup < dados[0].limInf);

	dados[0].somatorio = 0;

	_tprintf(_T("Insira o limite inferior dos números primos: "));
	_tscanf_s(_T("%d"), &dados[1].limInf);


	do {
		_tprintf(_T("Insira o limite superior dos números primos: "));
		_tscanf_s(_T("%d"), &dados[1].limSup);
	} while (dados[1].limSup < dados[1].limInf);
	dados[1].somatorio = 0;
	

	hThread[0] = CreateThread(NULL, 0, &pares, &dados[0], 0, NULL);
	hThread[1] = CreateThread(NULL, 0, &primos, &dados[1], 0, NULL);


	DWORD res = WaitForMultipleObjects(N, hThread, FALSE, INFINITE);

	if (res - WAIT_OBJECT_0 == 0) {
		_tprintf(_T("Terminei a thread %d\n"), res - WAIT_OBJECT_0);
		_tprintf(_T("Pares: %d\n"), dados[0].somatorio);
		DWORD res = WaitForSingleObject(hThread[1], INFINITE);
		_tprintf(_T("Primos: %d\n"), dados[1].somatorio);
	}
	else {
		_tprintf(_T("Terminei a thread %d\n"), res - WAIT_OBJECT_0);
		_tprintf(_T("Primos: %d\n"), dados[1].somatorio);
		DWORD res = WaitForSingleObject(hThread[0], INFINITE);
		_tprintf(_T("Pares: %d\n"), dados[0].somatorio);
	}

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	return 0;
}