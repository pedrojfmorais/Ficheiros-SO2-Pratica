#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define TAM 200

typedef double (*PFUNC) (double);

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	
	HINSTANCE hLibrary;
	hLibrary = LoadLibrary(_T("G:\\Outros computadores\\Desktop\\Escola\\ISEC\\Licenciatura de Engenharia Informática\\Cadeiras\\2º Ano\\2º Semestre\\Sistemas Operativos II\\Práticas\\aula3\\aula3\\x64\\Debug\\SO2_F3_DLL.dll"));
		
	if (hLibrary == NULL) {
		_tprintf(_T("A DLL não abriu!\n"));
		return -1;
	}

	double* a;
	a = (double *)GetProcAddress(hLibrary, "factor");


	PFUNC pFunc;
	pFunc = (PFUNC)GetProcAddress(hLibrary, "applyFactor");

	double b;

	while (*a != -1.0) {

		_tprintf(_T("\nInsira o valor para A: "));
		_tscanf_s(_T("%lf"), a);

		if (*a == -1)
			break;

		_tprintf(_T("Insira o valor para B: "));
		_tscanf_s(_T("%lf"), &b);

		_tprintf(_T("Resultado: %lf\n"), pFunc(b));
	}
	FreeLibrary(hLibrary);

	return 0;
}