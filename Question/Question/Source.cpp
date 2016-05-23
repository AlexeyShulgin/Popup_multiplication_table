#include <Windows.h>
#include <tchar.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#define ID_TIMER ( WM_USER + 1 ) // ID таймера
#define ID_EDIT ( WM_USER + 2 ) // ID Edit
#define ID_BUTTON ( WM_USER + 3 ) // ID Button
#define CLOSE_WND ( WM_USER + 14 )
#define SIZE_QUESTION 10
#define SIZE_ANSWER 5

static HINSTANCE hinstance = NULL;
static LPCWSTR CLASS_NAME = L"Pinocchio_Question"; // Имя класса окна
static LPCWSTR WND_NAME = L"Pinocchio"; // Заголовок окна
HWND hWnd = NULL; // Дескриптор окна
HWND hEdit = NULL; // Дескриптор Edit
HWND hButton = NULL; // Дескриптор Button
static UINT_PTR hTimer = NULL; // Дескриптор таймера
WCHAR path_exe[MAX_PATH] = { 0 }; // Путь к исполняемому файлу

int x = NULL;
int y = NULL;
static bool is_first = true;

DWORD GetError(DWORD); // Функция обработки ошибок
bool func_otvet(void); // Функция проверки ответа

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

	srand(time(NULL)); // Устанвка начального значения для генератора псевдослучайных чисел
	x = 1 + rand() % 9;
	y = 1 + rand() % 9;

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

	HDC hDCScreen = GetDC(NULL); // Разрешение экрана
	int Horzres = GetDeviceCaps(hDCScreen, HORZRES); // По горизонтали
	int Vertres = GetDeviceCaps(hDCScreen, VERTRES); // По вертикали
	ReleaseDC(NULL, hDCScreen);

	::hWnd = CreateWindow(CLASS_NAME, WND_NAME, WS_OVERLAPPED | WS_CAPTION, 0, 0, Horzres, Vertres, NULL, NULL, ::hinstance, NULL); // Создание окна
	if (!::hWnd)
		return GetError(GetLastError());

	::hEdit = CreateWindowEx(NULL, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, Horzres / 2 - (Horzres - (Horzres - 220)) / 2, Vertres - (Vertres - 150), Horzres - (Horzres - 200), 110, ::hWnd, (HMENU)ID_EDIT, ::hinstance, NULL); // Создание поля ввода для ответа
	if (!::hEdit)
		return GetError(GetLastError());

	::hButton = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME, L"BUTTON", L"Ответить", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, Horzres / 2 - 180, Vertres - (Vertres - 300), 360, 110, ::hWnd, (HMENU)ID_BUTTON, ::hinstance, NULL); // Создание кнопки
	if (!::hButton)
		return GetError(GetLastError());

	HFONT hFnt = CreateFont(110, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Comic Sans MS"); // Шрифт для текста в окне
	if (!hFnt)
		return GetError(GetLastError());

	SendMessage(::hEdit, WM_SETFONT, (WPARAM)hFnt, TRUE); // Применение шрифта к EDIT
	SendMessage(::hButton, WM_SETFONT, (WPARAM)hFnt, TRUE); // Применение шрифта к BUTTON

	if (!SetWindowLong(::hWnd, GWL_EXSTYLE, GetWindowLong(::hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW)) // Скрытие значка приложения
		return GetLastError();

	if (!AnimateWindow(::hWnd, 500, AW_ACTIVATE | AW_VER_NEGATIVE | AW_SLIDE)) // Анимация при появлении окна
		return GetLastError();

	ShowWindow(::hWnd, SW_SHOW); // Отображение окна

	UpdateWindow(::hWnd); // Обновление окна

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		// Обработка нажатия Enter в Edit
		if ((WM_KEYDOWN == msg.message) && (VK_RETURN == msg.wParam) && (msg.hwnd == hEdit))
		{
			if (!func_otvet()) // Вызов функции проверки ответа
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

	HWND hWndTask = NULL; // Окно диспетчера задач
	HWND hScrKey = NULL; // Окно экранной клавиатуры
	PAINTSTRUCT ps = { NULL };
	RECT rs = { NULL };
	HDC hdc = NULL;
	HFONT hFont = NULL;
	wchar_t str[SIZE_QUESTION] = { NULL };

	_SHELLEXECUTEINFOW sei = { NULL };
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_DEFAULT;
	sei.lpVerb = L"open";
	sei.lpFile = L"osk.exe";
	sei.nShow = SW_SHOW;

	switch (_Msg)
	{
	case WM_CREATE:
		::hTimer = SetTimer(_hWnd, ID_TIMER, 500, NULL); // Таймер
		if (!::hTimer)
		{
			exit = true;
			PostQuitMessage(GetError(GetLastError()));
			return GetLastError();
		}
		ShellExecuteEx(&sei);
		break;
	case WM_NCPAINT:
			SendMessage(::hWnd, WM_PAINT, _wParam, _lParam); // Вызов WM_PAINT
		break;
	case WM_TIMER:
		hScrKey = FindWindow(L"OSKMainClass", NULL); // Поиск окна экранной клавиатуры
		if (!hScrKey)
		{
			ShellExecuteEx(&sei); // Запуск экранной клавиатуры
			hScrKey = FindWindow(L"OSKMainClass", NULL); // Поиск окна экранной клавиатуры
		}
		hWndTask = FindWindow(L"TaskManagerWindow", NULL); // Проверка открыт ли диспетчер задач
		if (hWndTask) // Если диспетчер задач открыт
		{
			// Закрыть диспетчер задач
			DWORD pid = NULL;
			if (!GetWindowThreadProcessId(hWndTask, &pid))
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
			if (!hProcess)
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
			if (!TerminateProcess(hProcess, NULL))
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
			if (!CloseHandle(hProcess))
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
		}
		if (GetForegroundWindow() != _hWnd && GetForegroundWindow() != hScrKey) // Если окно не верхнее
			if (!SetWindowPos(_hWnd, HWND_TOPMOST, 0, 0, NULL, NULL, SWP_NOSIZE | SWP_SHOWWINDOW)) // Сделать окно верхним
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
		break;
	case WM_SYSCOMMAND:
		if (_wParam == HTCAPTION) // Если окно попытались переместить
			if (!SetWindowPos(_hWnd, HWND_TOPMOST, 0, 0, NULL, NULL, SWP_NOSIZE | SWP_SHOWWINDOW)) // Вернуть окно в начальное положение
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
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
		if(_wParam == ID_BUTTON)
			if (!func_otvet()) // Вызов функции проверки ответа
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
	case WM_CHAR:
		if (_wParam == VK_RETURN) // Если нажата клавиша Enter
			if (!func_otvet()) // Вызов функции проверки ответа
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
		break;
	case WM_PAINT:
		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &rs);
		SetTextColor(hdc, RGB(255, 255, 255));
		hFont = CreateFont(150, 60, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, VARIABLE_PITCH, L"Comic Sans MS");
		SelectObject(hdc, hFont);
		SetBkColor(hdc, RGB(0, 139, 0));
		swprintf_s(str, L"%d\u00d7%d=", x, y);
		DrawText(hdc, str, wcslen(str), &rs, DT_CENTER);
		ReleaseDC(hWnd, hdc);
		break;
	default:
		return DefWindowProc(_hWnd, _Msg, _wParam, _lParam);
	}
	return 0;
}
bool func_otvet(void)
{
	if (::hTimer)
		if (!KillTimer(::hWnd, ID_TIMER)) // Уничтожение таймера чтоб не мешал читать MessageBox
			return false;
	int len = GetWindowTextLength(::hEdit); // Длина текста в поле ввода
	if (len)
	{
		wchar_t otvet[SIZE_ANSWER] = { 0 }; // Ответ, введённый с клавиатуры
		wchar_t otv[SIZE_ANSWER] = { 0 };
		SendMessage(::hEdit, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)&otvet[0]); // Получение ответа из поля ввода
		for (int i = 0; i < len; i++)
		{
			if (otvet[i] != L' ')
			{
				swprintf_s(otv, L"%s%c", otv, otvet[i]);
			}
		}
		wchar_t o[SIZE_ANSWER] = { 0 };
		_itow_s(x*y, o, 10);
		wchar_t *loc = _wsetlocale(LC_ALL, L"");
		int r = _wcsicmp(o, otv); // Сравнение ответа, введённого с клавиатуры с правильны ответом
		_wsetlocale(LC_ALL, loc);
		if (!r)
		{
			MessageBox(::hWnd, L"Великолепно!", L"\u263a\u263a\u263a", NULL);
			SendMessage(::hWnd, WM_DESTROY, (WPARAM)CLOSE_WND, NULL); // Закрытие окна
		}
		else
		{
			MessageBox(::hWnd, L"Неправильно!\r\nНичего страшного!", L"\u2639\u2639\u2639", NULL);
			::hTimer = SetTimer(::hWnd, ID_TIMER, 500, NULL); // Таймер
			if (!::hTimer)
				return false;
		}
	}
	else
	{
		if (!MessageBeep(MB_ICONERROR)) // Звук ошибки
			return false;
		if (!MessageBox(::hWnd, L"Ответ не введен", L"Ответ", MB_ICONWARNING)) // Сообщение об ошибке
			return false;

		::hTimer = SetTimer(::hWnd, ID_TIMER, 500, NULL); // Таймер
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