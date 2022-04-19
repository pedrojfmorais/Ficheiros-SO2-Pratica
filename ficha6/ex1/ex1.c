#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

int _tmain(int argc, TCHAR* argv[]) {


#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	HANDLE hFile = CreateFile(
		_T("letras.txt"),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if(hFile == INVALID_HANDLE_VALUE)
		_tprintf(_T("ERRO - %d\n"), GetLastError());
	else
		_tprintf(_T("Consegui abrir o ficheiro \n"));


	HANDLE hMap = CreateFileMapping(
		hFile, 
		NULL, 
		PAGE_READWRITE, 
		0, 
		26, 
		NULL);

	if(hMap == NULL)
		_tprintf(_T("ERRO - %d\n"), GetLastError());
	else
		_tprintf(_T("Consegui criar o map do ficheiro \n"));

	char* pVistaFicheiro = (char*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 26);


	_tprintf(_T("\nAntes: "));
	for (DWORD i = 0; i < 26; i++)
		_tprintf(_T("%c"), pVistaFicheiro[i]);

	_tprintf(_T("\n"));

	char aux; int j = 25;
	for (DWORD i = 0; i < 13; i++) {
		aux = pVistaFicheiro[i];
		pVistaFicheiro[i] = pVistaFicheiro[j];
		pVistaFicheiro[j] = aux;
		j--;
	}

	_tprintf(_T("\nDepois: "));
	for (DWORD i = 0; i < 26; i++)
		_tprintf(_T("%c"), pVistaFicheiro[i]);

	_tprintf(_T("\n"));

	UnmapViewOfFile(pVistaFicheiro);

	CloseHandle(hFile);
	CloseHandle(hMap);

	return 0;
}
// Este código é apenas uma ajuda para o exercício. Se quiser, mude-o
