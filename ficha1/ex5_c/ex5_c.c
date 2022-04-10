#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#define MAX 256

int _tmain(int argc, LPTSTR argv[]) {

	//UNICODE: Por defeito, a consola Windows não processa caracteres wide.
	//A maneira mais fácil para ter esta funcionalidade é chamar _setmode:
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	unsigned int size;
	TCHAR programa[MAX];
	TCHAR argumentos[MAX];

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	do {

		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);

		_tprintf(TEXT("Insira o nome de um programa: "));
		_fgetts(programa, MAX, stdin);
		programa[_tcslen(programa) - 1] = '\0';
		
		if (_tcsicmp(programa, TEXT("fim"))){

			_tprintf(TEXT("Insira os argumentos do programa: "));
			_fgetts(argumentos, MAX, stdin);
			argumentos[_tcslen(argumentos) - 1] = '\0';

			_tcscat_s(programa, MAX, TEXT(" "));
			_tcscat_s(programa, MAX, argumentos);

			if (CreateProcess(NULL, programa, NULL, NULL, 0, 0, NULL, NULL, &si, &pi)) {
				_tprintf(TEXT("O programa foi executado!\n"));
				WaitForSingleObject(pi.hProcess, INFINITE);
				_tprintf(TEXT("O programa acabou de executar!\n\n"));
			}
			else {
				_tprintf(TEXT("O programa não foi executado!\n\n"));
			}
		}

	} while (_tcsicmp(programa, TEXT("fim")));

	return 0;
}