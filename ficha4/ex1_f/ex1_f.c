#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define N 4

typedef struct {

	DWORD somatorio;
	DWORD limInf;
	DWORD limSup;

} Dados;

DWORD WINAPI pares(void* dados) {
	Dados* data = (Dados*)dados;

	DWORD somados = 0;

	_tprintf(_T("Sou a thread que soma pares e vou de %d a %d\n"), data->limInf, data->limSup);

	for (DWORD i = data->limInf; i <= data->limSup; i++) {

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

	DWORD somados = 0;

	_tprintf(_T("Sou a thread que soma primos e vou de %d a %d\n"), data->limInf, data->limSup);

	for (DWORD i = data->limInf; i <= data->limSup; i++) {
		// Skip 0 and 1 as they are
		// neither prime nor composite
		if (i == 1 || i == 0)
			continue;

		// flag variable to tell
		// if i is prime or not
		DWORD flag = 1;

		for (DWORD j = 2; j <= i / 2; ++j) {
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


	_tprintf(_T("Insira o limite inferior dos números primos: "));
	_tscanf_s(_T("%d"), &dados[2].limInf);


	do {
		_tprintf(_T("Insira o limite superior dos números primos: "));
		_tscanf_s(_T("%d"), &dados[2].limSup);
	} while (dados[2].limSup < dados[2].limInf);


	dados[0].somatorio = 0;
	dados[1].somatorio = 0;
	dados[2].somatorio = 0;
	dados[3].somatorio = 0;

	dados[1].limInf = dados[0].limSup - ((dados[0].limSup - dados[0].limInf) / 2);
	dados[1].limSup = dados[0].limSup;
	dados[0].limSup = dados[1].limInf - 1;

	dados[3].limInf = dados[2].limSup - ((dados[2].limSup - dados[2].limInf) / 2);
	dados[3].limSup = dados[2].limSup;
	dados[2].limSup = dados[3].limInf - 1;

	hThread[0] = CreateThread(NULL, 0, &pares, &dados[0], 0, NULL);
	hThread[1] = CreateThread(NULL, 0, &pares, &dados[1], 0, NULL);
	hThread[2] = CreateThread(NULL, 0, &primos, &dados[2], 0, NULL);
	hThread[3] = CreateThread(NULL, 0, &primos, &dados[3], 0, NULL);


	DWORD res = WaitForMultipleObjects(N, hThread, TRUE, INFINITE);


	_tprintf(_T("Pares: %d, Primos: %d\n"),
		dados[0].somatorio + dados[1].somatorio,
		dados[2].somatorio + dados[3].somatorio
	);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	CloseHandle(hThread[2]);
	CloseHandle(hThread[3]);

	return 0;
}