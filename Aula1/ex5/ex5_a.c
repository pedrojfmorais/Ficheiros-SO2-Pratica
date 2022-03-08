#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#define MAX 256

int _tmain(int argc, LPTSTR argv[]) {

	//UNICODE: Por defeito, a consola Windows n�o processa caracteres wide.
	//A maneira mais f�cil para ter esta funcionalidade � chamar _setmode:
	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
		_setmode(_fileno(stderr), _O_WTEXT);
	#endif

	unsigned int size;
	TCHAR nome[MAX];

	size = GetModuleFileName(NULL, nome, MAX);

	DWORD error = GetLastError();

	_tprintf(TEXT("[%d] O nome do execut�vel �: \n\n%s \n\n"), size, nome);

	return 0;
}