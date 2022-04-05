#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "SO2_F3_DLL.h"

#define TAM 200

typedef double (*PFUNC) (double);

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	double b;

	while (factor != -1.0) {

		_tprintf(_T("\nInsira o valor para A: "));
		_tscanf_s(_T("%lf"), &factor);

		if (factor == -1)
			break;

		_tprintf(_T("Insira o valor para B: "));
		_tscanf_s(_T("%lf"), &b);

		_tprintf(_T("Resultado: %lf\n"), applyFactor(b));
	}

	return 0;
}