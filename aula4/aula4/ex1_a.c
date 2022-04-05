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
				Sleep(1);
				_tprintf(_T("Uma pausa nos pares\n"));
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
				Sleep(1);

				_tprintf(_T("Pausa dos primos\n"));
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


	for (int i = 0; i < N; i++) {
		dados[i].limInf = 0;
		dados[i].limSup = 1000;
		dados[i].somatorio = 0;
	}

	hThread[0] = CreateThread(NULL, 0, &pares, &dados[0], 0, NULL);
	hThread[1] = CreateThread(NULL, 0, &primos, &dados[1], 0, NULL);
	
	DWORD res = WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	_tprintf(_T("Pares: %d, Primos: %d\n"), dados[0].somatorio, dados[1].somatorio);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	return 0;
}