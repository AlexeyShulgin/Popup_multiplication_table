#include <Windows.h>
#include <tchar.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#define ID_TIMER ( WM_USER + 1 ) // ID таймера
#define ID_EDIT ( WM_USER + 2 ) // ID Edit
#define ID_BUTTON ( WM_USER + 3 ) // ID Button
#define CLOSE_WND ( WM_USER + 14 )
#define SIZE_PASSWORD 20

static HINSTANCE hinstance = NULL;
static LPCWSTR CLASS_NAME = L"Pinocchio_Question"; // Имя класса окна
static LPCWSTR WND_NAME = L"Pinocchio"; // Заголовок окна
HWND hWnd = NULL; // Дескриптор окна
HWND hEdit = NULL; // Дескриптор Edit
HWND hButton = NULL; // Дескриптор Button
static UINT_PTR hTimer = NULL; // Дескриптор таймера
WCHAR path_exe[MAX_PATH] = { 0 }; // Путь к исполняемому файлу

DWORD GetError(DWORD);
bool Auth(void);

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI _tWinMain(HINSTANCE _hinstance, HINSTANCE _hPrevInstance, LPTSTR _lpCommandLine, int nCmdShow)
{
	::hinstance = _hinstance;

	LPWSTR *szArglist = NULL; // Массив, подобный ARGV
	int nArgs = 0; // Количество элементов возвращаемого массива

	// Получение пути к исполняемому файлу для его запуска в случае попытки закрыть окно
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (NULL == szArglist)
		return GetLastError();
	wcscpy_s(::path_exe, sizeof(::path_exe) / sizeof(WCHAR), szArglist[0]);
	LocalFree(szArglist);

	WNDCLASSEX wc = { 0 }; // Структура с информацией о классе окна

	wc.cbSize = sizeof(WNDCLASSEX);							// Размер структуры в байтах
	wc.style = CS_HREDRAW | CS_VREDRAW;						// Стиль класса окна
	wc.lpfnWndProc = WndProc;								// Указатель на оконную процедуру
	wc.cbClsExtra = 0;										// Число дополнительных байт за структурой класса окна
	wc.cbWndExtra = 0;										// Число дополнительных байт за экземпляром окна
	wc.hInstance = ::hinstance;								// Дескриптор экземпляра, который содержит оконную процедуру для класса
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);				// Дескриптор значка класса
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);				// Дескриптор курсора класса
	wc.hbrBackground = CreateSolidBrush(RGB(0, 139, 0));	// Дескриптор кисти фона класса
	wc.lpszClassName = CLASS_NAME;							// Имя класса окна
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);			// Дескриптор маленького значка

	RegisterClassEx(&wc); // Регистрация класса окна

	::hWnd = CreateWindow(CLASS_NAME, WND_NAME, WS_OVERLAPPED | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, 200, 150, NULL, NULL, ::hinstance, NULL); // Создание окна
	if (!::hWnd)
		return GetError(GetLastError());

	::hEdit = CreateWindowEx(NULL, L"EDIT", NULL, ES_PASSWORD | WS_CHILD | WS_VISIBLE | WS_BORDER, 20, 20, 150, 20, ::hWnd, (HMENU)ID_EDIT, ::hinstance, NULL); // Создание поля ввода для ответа
	if (!::hEdit)
		return GetError(GetLastError());

	::hButton = CreateWindowEx(NULL, L"BUTTON", L"Выключить", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, 20, 60, 150, 20, ::hWnd, (HMENU)ID_BUTTON, ::hinstance, NULL); // Создание кнопки
	if (!::hButton)
		return GetError(GetLastError());

	SendMessage(::hEdit, EM_SETPASSWORDCHAR, (WPARAM)L'\u2716', NULL);

	if (!AnimateWindow(::hWnd, 300, AW_ACTIVATE | AW_VER_POSITIVE | AW_SLIDE)) // Анимация при появлении окна
		return GetLastError();

	ShowWindow(::hWnd, SW_MINIMIZE); // Отображение окна

	UpdateWindow(::hWnd); // Обновление окна

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		// Обработка нажатия Enter в Edit
		if ((WM_KEYDOWN == msg.message) && (VK_RETURN == msg.wParam) && (msg.hwnd == hEdit))
		{
			if (!Auth()) // Вызов функции проверки ответа
			{
				return GetError(GetLastError());
			}
			continue;      // Запрет дальнейшей обработки
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(::hEdit); // Закрытие Edit
	DestroyWindow(::hButton); // Закрытие Button
	DestroyWindow(::hWnd); // Закрытие окна

	return (int)msg.wParam;
}
LRESULT CALLBACK WndProc(HWND _hWnd, UINT _Msg, WPARAM _wParam, LPARAM	_lParam)
{
	static bool exit = false; // Переменная для определения необходимости закрыть программу

	_SHELLEXECUTEINFOW sei = { NULL };
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_DEFAULT;
	sei.lpVerb = L"open";
	sei.lpFile = L"\"C:\\Program Files\\Pinocchio\\Question.exe\"";
	sei.nShow = SW_SHOW;
	switch (_Msg)
	{
	case WM_CREATE:
		::hTimer = SetTimer(_hWnd, ID_TIMER, 300000, NULL); // Таймер
		if (!::hTimer)
		{
			exit = true;
			PostQuitMessage(GetError(GetLastError()));
			return GetLastError();
		}
		MessageBox(::hWnd, L"Pinocchio включен!", L"Pinocchio", MB_ICONINFORMATION);
		break;
	case WM_NCPAINT:
		SendMessage(::hWnd, WM_PAINT, _wParam, _lParam); // Вызов WM_PAINT
		break;
	case WM_TIMER:
		ShellExecuteEx(&sei); // Запуск программы Question.exe
		break;
	case WM_DESTROY:
		if (_wParam != CLOSE_WND && GetLastError() != NO_ERROR) // Если окно попытались зактыть с помощью Alt+F4
		{
			if (!exit) // Если закрывать програму не надо
			{
				STARTUPINFO si = { 0 };
				si.cb = sizeof(STARTUPINFO); // Размер структуры STARTUPINFO в байтах
				PROCESS_INFORMATION pi = { 0 };
				if (!CreateProcess(NULL, ::path_exe, NULL, NULL, FALSE, CREATE_NEW, NULL, NULL, &si, &pi)) // Запустить программу снова
				{
					exit = true;
					PostQuitMessage(GetError(GetLastError()));
					return GetLastError();
				}
			}
		}
		else
			exit = true; // Разрешить закрытие программы
		PostQuitMessage(0); // Закрытие программы
		break;
	case WM_COMMAND:
		if (_wParam == ID_BUTTON)
			if (!Auth()) // Вызов функции проверки ответа
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
	default:
		return DefWindowProc(_hWnd, _Msg, _wParam, _lParam);
	}
	return 0;
}
bool Auth(void)
{
	if (::hTimer)
		if (!KillTimer(::hWnd, ID_TIMER)) // Уничтожение таймера чтоб не мешал читать MessageBox
			return false;

	int len = GetWindowTextLength(::hEdit); // Длина текста в поле ввода
	if (len)
	{
		wchar_t passw[SIZE_PASSWORD] = { 0 }; // Ответ, введённый с клавиатуры
		SendMessage(::hEdit, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)&passw[0]); // Получение ответа из поля ввода
		wchar_t *loc = _wsetlocale(LC_ALL, L"");
		int r = _wcsicmp(L"gfhjkm", passw); // Сравнение ответа, введённого с клавиатуры с правильны ответом
		_wsetlocale(LC_ALL, loc);
		if (!r)
		{
			MessageBox(::hWnd, L"ОК!", L"\u263a\u263a\u263a", NULL);
			SendMessage(::hWnd, WM_DESTROY, (WPARAM)CLOSE_WND, NULL); // Закрытие окна
		}
		else
		{
			MessageBox(::hWnd, L"Неверный пароль!\r\nНичего страшного!", L"\u2639\u2639\u2639", NULL);
			::hTimer = SetTimer(::hWnd, ID_TIMER, 300000, NULL); // Таймер
			if (!::hTimer)
				return false;
		}
	}
	else
	{
		if (!MessageBox(::hWnd, L"Пароль не введен!", L"", MB_ICONWARNING)) // Сообщение об ошибке
			return false;

		::hTimer = SetTimer(::hWnd, ID_TIMER, 300000, NULL); // Таймер
		if (!::hTimer)
			return false;
	}
	return true;
}
DWORD GetError(DWORD _Error)
{
	if (::hTimer)
		KillTimer(::hWnd, ID_TIMER); // Уничтожение таймера чтоб не мешал закрыть MessageBox
	LPVOID lperr = NULL;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, _Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lperr, NULL, NULL); // Преобразование кода ошибки в строку
	MessageBox(::hWnd, (LPCWSTR)lperr, NULL, MB_ICONERROR); // Вывод сообщения об ошибке
	LocalFree(lperr);
	return _Error;
}