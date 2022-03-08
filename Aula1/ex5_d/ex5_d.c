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
	int vezes;

	if (argc = 1) {

		vezes = 3;
	}
	else if (argc > 1) {
		vezes = _ttoi(argv[1]);
	}

	TCHAR programa[MAX] = TEXT("ex5_d");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if(vezes > 0){

		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);

		if (CreateProcess(NULL, ar gv[0] + vezes, NULL, NULL, 0, 0, NULL, NULL, &si, &pi)) {
			_tprintf(TEXT("O programa foi executado! %d\n"), vezes);
			vezes--;
			WaitForSingleObject(pi.hProcess, INFINITE);
			_tprintf(TEXT("O programa acabou de executar!\n\n"));
		}
		else {
			_tprintf(TEXT("O programa não foi executado!\n\n"));
		}
	}

	return 0;
}