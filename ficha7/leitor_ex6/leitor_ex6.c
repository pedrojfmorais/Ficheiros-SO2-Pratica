#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#define PIPE_NAME TEXT("\\\\.\\pipe\\teste")

int _tmain(int argc, LPTSTR argv[]) {
    TCHAR buf[256];
    HANDLE hPipe;
    int i = 0;
    BOOL ret;
    DWORD n;

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    _tprintf(TEXT("[LEITOR] Esperar pelo pipe '%s' (WaitNamedPipe)\n"),
        PIPE_NAME);
    if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_NAME);
        exit(-1);
    }
    _tprintf(TEXT("[LEITOR] Liga��o ao pipe do escritor... (CreateFile)\n"));
    hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hPipe == NULL) {
        _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
        exit(-1);
    }
    _tprintf(TEXT("[LEITOR] Liguei-me...\n"));

    while (1) {
        ret = ReadFile(hPipe, buf, sizeof(buf), &n, NULL);
        buf[n / sizeof(TCHAR)] = '\0';
        if (!ret || !n) {
            _tprintf(TEXT("[LEITOR] %d %d... (ReadFile)\n"), ret, n);
            break;
        }
        _tprintf(TEXT("[LEITOR] Recebi %d bytes: '%s'... (ReadFile)\n"), n, buf);

        for (DWORD i = 0; i < _tcslen(buf); i++) {
            buf[i] = _toupper(buf[i]);
        }

        if (!WriteFile(hPipe, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL))
            _tprintf(TEXT("[Erro] Escrever Pipe (WriteFile)\n"));
        else
            _tprintf(TEXT("[LEITOR] Escrevi %d bytes ao escritor: '%s'... (WriteFile)\n"), n, buf);

    }
    CloseHandle(hPipe);
    Sleep(200);
    return 0;
}
