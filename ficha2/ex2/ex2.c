#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define TAM 200

int _tmain(int argc, TCHAR* argv[]) {
	HKEY chave;
	TCHAR chave_nome[TAM], par_nome[TAM], par_valor[TAM], resposta[TAM];
	TCHAR caminho[TAM] = _T("xpto");
	TCHAR caminhoCompleto[TAM] = _T("software\\aula\\");
	LSTATUS returnCreate, returnSetValue, returnQuerryValue, returnDeleteValue;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	//alinea b)
	_tprintf(_T("Insira o nome da chave: "));
	_fgetts(caminho, TAM, stdin);

	_tcscat_s(caminhoCompleto, TAM, caminho);

	returnCreate = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		caminhoCompleto,
		0, 
		NULL, 
		REG_OPTION_VOLATILE, 
		KEY_ALL_ACCESS, 
		NULL, 
		&chave, 
		&resposta
	);

	if (returnCreate != ERROR_SUCCESS) {
		DWORD error = GetLastError();
		_tprintf(_T("Erro a criar a chave [%d]\n"), error);
	}
	else {
		if (resposta == REG_CREATED_NEW_KEY) {
			_tprintf(_T("\n [%s] chave não existia e foi criada com sucesso!"), caminhoCompleto);
		}
		else if (resposta == REG_OPENED_EXISTING_KEY) {
				_tprintf(_T("\n [%s] chave foi aberta com sucesso!"), caminhoCompleto);
		}
	}

	//alinea d)

	_tprintf(_T("Insira uma das opções: \n"));
	_tprintf(_T("1 - Criar um valor\n"));
	_tprintf(_T("2 - Consultar um valor\n"));
	_tprintf(_T("3 - Eliminar valor\n"));
	_tprintf(_T("Opção: "));

	DWORD opcao;
	_tscanf_s(_T("%d\n"), &opcao);

	TCHAR nome[TAM];
	TCHAR val[TAM];
	DWORD sizeVal;

	switch (opcao) {
	case 1:

		_tprintf(_T("Insira o nome do value: "));
		_fgetts(nome, TAM, stdin);
		_tprintf(_T("Insira o valor de %s: "), nome);
		_fgetts(val, TAM, stdin);
		
		returnSetValue = RegSetValueEx(
			chave,
			nome,
			0,
			REG_SZ,
			(LPBYTE) val,
			(DWORD)(_tcslen(val) + 1) * sizeof(TCHAR)
		);
		if (returnSetValue != ERROR_SUCCESS) {
			DWORD error = GetLastError();
			_tprintf(_T("Erro a criar o value [%d]\n"), error);
		}
		break;
	case 2:
		_tprintf(_T("Insira o nome do value a procurar: "));
		_fgetts(nome, TAM, stdin);

		returnQuerryValue = RegQueryValueEx(
			chave,
			nome,
			NULL,
			NULL,
			(LPBYTE)val,
			&sizeVal
		);
		if (returnQuerryValue != ERROR_SUCCESS) {
			DWORD error = GetLastError();
			_tprintf(_T("Erro a ler o value [%d]\n"), error);
		}
		else
		{
			_tprintf(_T("%s tem valor %s"), nome, val);
		}
		break;
	case 3:
		_tprintf(_T("Insira o nome do value a apagar: "));
		_fgetts(nome, TAM, stdin);

		returnDeleteValue = RegDeleteValue(
			chave,
			nome
		);
		if (returnDeleteValue != ERROR_SUCCESS) {
			DWORD error = GetLastError();
			_tprintf(_T("Erro a apagar o value [%d]\n"), error);
		}
		break;
	default:
		_tprintf(_T("Opção inserida é inválida!\n"));
	}

	RegCloseKey(chave);
	return 0;
}