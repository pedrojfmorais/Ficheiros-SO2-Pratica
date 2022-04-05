#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>


// funcionalidade relacionada com temporização

static double PerfCounterFreq; // n ticks por seg.

void initClock() {
	LARGE_INTEGER aux;
	if (!QueryPerformanceFrequency(&aux))
		_tprintf(TEXT("\nSorry - No can do em QueryPerfFreq\n"));
	PerfCounterFreq = (double)(aux.QuadPart); // / 1000.0;
	_tprintf(TEXT("\nTicks por sec.%f\n"), PerfCounterFreq);
}

__int64 startClock() {
	LARGE_INTEGER aux;
	QueryPerformanceCounter(&aux);
	return aux.QuadPart;
}

double stopClock(__int64 from) {
	LARGE_INTEGER aux;
	QueryPerformanceCounter(&aux);
	return (double)(aux.QuadPart - from) / PerfCounterFreq;
}

// estrutura de dados para controlar as threads

typedef struct {
	unsigned int limiteSuperior;
	unsigned int limiteInferior;
	unsigned int* contadorMultiplos;
	boolean* continuar;
	CRITICAL_SECTION* cs;
} TDados;

// função da(s) thread(s)
DWORD WINAPI MultiplosDeTres(LPVOID lpParam) {
	TDados* dados = (TDados*)lpParam;
	for (unsigned int i = dados->limiteInferior; i < dados->limiteSuperior; i++) {
		if (i % 3 == 0) {
			EnterCriticalSection(dados->cs);
			(*dados->contadorMultiplos)++;
			LeaveCriticalSection(dados->cs);
		}

		if ((*dados->continuar) == FALSE)
			break;
	}
}

// número * máximo * de threads
// podem (e devem) ser menos
#define MAX_THREADS 20


int _tmain(int argc, TCHAR* argv[]) {

	// matriz de handles das threads
	HANDLE hThreads[MAX_THREADS];

	// Matriz de dados para as threads;
	TDados tdados[MAX_THREADS];

	// número efectivo de threads
	int numthreads;

	// limite superior
	unsigned int limsup;

	// variáveis para cronómetro
	__int64 clockticks;
	double duracao;

	unsigned int range;
	unsigned int inter;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif 

	initClock();
	_tprintf(TEXT("\nLimite sup. -> "));
	_tscanf_s(TEXT("%u"), &limsup);
	_tprintf(TEXT("\nNum threads -> "));
	_tscanf_s(TEXT("%u"), &numthreads);
	if (numthreads > MAX_THREADS)
		numthreads = MAX_THREADS;

	// FAZER prepara e cria threads
	unsigned int contadorMultiplos = 0;
	boolean continuarThreads = TRUE;

	CRITICAL_SECTION cs;
	BOOL ii = InitializeCriticalSectionAndSpinCount(&cs, 200);

	for (int i = 0; i < numthreads; i++) {
		tdados[i].cs = &cs;
		tdados[i].continuar = &continuarThreads;
		tdados[i].contadorMultiplos = &contadorMultiplos;
		tdados[i].limiteInferior = 1 + (limsup / numthreads) * i;
		tdados[i].limiteSuperior = (limsup / numthreads) * (i + 1);
		hThreads[i] = CreateThread(NULL, 0, MultiplosDeTres, &tdados[i], CREATE_SUSPENDED, NULL);
	}

	//       manda as threads começar

	clockticks = startClock();

	for (int i = 0; i < numthreads; i++)
		ResumeThread(hThreads[i]);

	// FAZER aguarda / controla as threads 

	WaitForMultipleObjects(numthreads, hThreads, TRUE, INFINITE);

	//       manda as threads parar

	duracao = stopClock(clockticks);
	_tprintf(TEXT("\nSegundos=%f\n"), duracao);

	// FAZER apresenta resultados
	_tprintf(_T("Foram encontrados %d múltiplos de 3.\n"), contadorMultiplos);

	// Cód. ref. para aguardar por uma tecla – caso faça falta
	// _tprintf(TEXT("\nCarregue numa tecla"));
	// _gettch();

	for (int i = 0; i < numthreads; i++)
		CloseHandle(hThreads[i]);

	DeleteCriticalSection(&cs);

	return 0;
}
// Este código é apenas uma ajuda para o exercício. Se quiser, mude-o
