#include <Windows.h>
#include <tchar.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#define ID_TIMER ( WM_USER + 1 ) // ID �������
#define ID_EDIT ( WM_USER + 2 ) // ID Edit
#define ID_BUTTON ( WM_USER + 3 ) // ID Button
#define CLOSE_WND ( WM_USER + 14 )
#define SIZE_QUESTION 10
#define SIZE_ANSWER 5

static HINSTANCE hinstance = NULL;
static LPCWSTR CLASS_NAME = L"Pinocchio_Question"; // ��� ������ ����
static LPCWSTR WND_NAME = L"Pinocchio"; // ��������� ����
HWND hWnd = NULL; // ���������� ����
HWND hEdit = NULL; // ���������� Edit
HWND hButton = NULL; // ���������� Button
static UINT_PTR hTimer = NULL; // ���������� �������
WCHAR path_exe[MAX_PATH] = { 0 }; // ���� � ������������ �����

int x = NULL;
int y = NULL;
static bool is_first = true;

DWORD GetError(DWORD); // ������� ��������� ������
bool func_otvet(void); // ������� �������� ������

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI _tWinMain(HINSTANCE _hinstance, HINSTANCE _hPrevInstance, LPTSTR _lpCommandLine, int nCmdShow)
{
	::hinstance = _hinstance;

	LPWSTR *szArglist = NULL; // ������, �������� ARGV
	int nArgs = 0; // ���������� ��������� ������������� �������

	// ��������� ���� � ������������ ����� ��� ��� ������� � ������ ������� ������� ����
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (NULL == szArglist)
		return GetLastError();
	wcscpy_s(::path_exe, sizeof(::path_exe) / sizeof(WCHAR), szArglist[0]);
	LocalFree(szArglist);

	srand(time(NULL)); // �������� ���������� �������� ��� ���������� ��������������� �����
	x = 1 + rand() % 9;
	y = 1 + rand() % 9;

	WNDCLASSEX wc = { 0 }; // ��������� � ����������� � ������ ����

	wc.cbSize = sizeof(WNDCLASSEX);							// ������ ��������� � ������
	wc.style = CS_HREDRAW | CS_VREDRAW;						// ����� ������ ����
	wc.lpfnWndProc = WndProc;								// ��������� �� ������� ���������
	wc.cbClsExtra = 0;										// ����� �������������� ���� �� ���������� ������ ����
	wc.cbWndExtra = 0;										// ����� �������������� ���� �� ����������� ����
	wc.hInstance = ::hinstance;								// ���������� ����������, ������� �������� ������� ��������� ��� ������
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);				// ���������� ������ ������
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);				// ���������� ������� ������
	wc.hbrBackground = CreateSolidBrush(RGB(0, 139, 0));	// ���������� ����� ���� ������
	wc.lpszClassName = CLASS_NAME;							// ��� ������ ����
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);			// ���������� ���������� ������

	RegisterClassEx(&wc); // ����������� ������ ����

	HDC hDCScreen = GetDC(NULL); // ���������� ������
	int Horzres = GetDeviceCaps(hDCScreen, HORZRES); // �� �����������
	int Vertres = GetDeviceCaps(hDCScreen, VERTRES); // �� ���������
	ReleaseDC(NULL, hDCScreen);

	::hWnd = CreateWindow(CLASS_NAME, WND_NAME, WS_OVERLAPPED | WS_CAPTION, 0, 0, Horzres, Vertres, NULL, NULL, ::hinstance, NULL); // �������� ����
	if (!::hWnd)
		return GetError(GetLastError());

	::hEdit = CreateWindowEx(NULL, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, Horzres / 2 - (Horzres - (Horzres - 220)) / 2, Vertres - (Vertres - 150), Horzres - (Horzres - 200), 110, ::hWnd, (HMENU)ID_EDIT, ::hinstance, NULL); // �������� ���� ����� ��� ������
	if (!::hEdit)
		return GetError(GetLastError());

	::hButton = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME, L"BUTTON", L"��������", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, Horzres / 2 - 180, Vertres - (Vertres - 300), 360, 110, ::hWnd, (HMENU)ID_BUTTON, ::hinstance, NULL); // �������� ������
	if (!::hButton)
		return GetError(GetLastError());

	HFONT hFnt = CreateFont(110, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Comic Sans MS"); // ����� ��� ������ � ����
	if (!hFnt)
		return GetError(GetLastError());

	SendMessage(::hEdit, WM_SETFONT, (WPARAM)hFnt, TRUE); // ���������� ������ � EDIT
	SendMessage(::hButton, WM_SETFONT, (WPARAM)hFnt, TRUE); // ���������� ������ � BUTTON

	if (!SetWindowLong(::hWnd, GWL_EXSTYLE, GetWindowLong(::hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW)) // ������� ������ ����������
		return GetLastError();

	if (!AnimateWindow(::hWnd, 500, AW_ACTIVATE | AW_VER_NEGATIVE | AW_SLIDE)) // �������� ��� ��������� ����
		return GetLastError();

	ShowWindow(::hWnd, SW_SHOW); // ����������� ����

	UpdateWindow(::hWnd); // ���������� ����

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		// ��������� ������� Enter � Edit
		if ((WM_KEYDOWN == msg.message) && (VK_RETURN == msg.wParam) && (msg.hwnd == hEdit))
		{
			if (!func_otvet()) // ����� ������� �������� ������
			{
				return GetError(GetLastError());
			}
			continue;      // ������ ���������� ���������
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(::hEdit); // �������� Edit
	DestroyWindow(::hButton); // �������� Button
	DestroyWindow(::hWnd); // �������� ����

	return (int)msg.wParam;
}
LRESULT CALLBACK WndProc(HWND _hWnd, UINT _Msg, WPARAM _wParam, LPARAM	_lParam)
{
	static bool exit = false; // ���������� ��� ����������� ������������� ������� ���������

	HWND hWndTask = NULL; // ���� ���������� �����
	HWND hScrKey = NULL; // ���� �������� ����������
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
		::hTimer = SetTimer(_hWnd, ID_TIMER, 500, NULL); // ������
		if (!::hTimer)
		{
			exit = true;
			PostQuitMessage(GetError(GetLastError()));
			return GetLastError();
		}
		ShellExecuteEx(&sei);
		break;
	case WM_NCPAINT:
			SendMessage(::hWnd, WM_PAINT, _wParam, _lParam); // ����� WM_PAINT
		break;
	case WM_TIMER:
		hScrKey = FindWindow(L"OSKMainClass", NULL); // ����� ���� �������� ����������
		if (!hScrKey)
		{
			ShellExecuteEx(&sei); // ������ �������� ����������
			hScrKey = FindWindow(L"OSKMainClass", NULL); // ����� ���� �������� ����������
		}
		hWndTask = FindWindow(L"TaskManagerWindow", NULL); // �������� ������ �� ��������� �����
		if (hWndTask) // ���� ��������� ����� ������
		{
			// ������� ��������� �����
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
		if (GetForegroundWindow() != _hWnd && GetForegroundWindow() != hScrKey) // ���� ���� �� �������
			if (!SetWindowPos(_hWnd, HWND_TOPMOST, 0, 0, NULL, NULL, SWP_NOSIZE | SWP_SHOWWINDOW)) // ������� ���� �������
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
		break;
	case WM_SYSCOMMAND:
		if (_wParam == HTCAPTION) // ���� ���� ���������� �����������
			if (!SetWindowPos(_hWnd, HWND_TOPMOST, 0, 0, NULL, NULL, SWP_NOSIZE | SWP_SHOWWINDOW)) // ������� ���� � ��������� ���������
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
		break;
	case WM_DESTROY:
		if (_wParam != CLOSE_WND && GetLastError() != NO_ERROR) // ���� ���� ���������� ������� � ������� Alt+F4
		{
			if (!exit) // ���� ��������� �������� �� ����
			{
				STARTUPINFO si = { 0 };
				si.cb = sizeof(STARTUPINFO); // ������ ��������� STARTUPINFO � ������
				PROCESS_INFORMATION pi = { 0 };
				if (!CreateProcess(NULL, ::path_exe, NULL, NULL, FALSE, CREATE_NEW, NULL, NULL, &si, &pi)) // ��������� ��������� �����
				{
					exit = true;
					PostQuitMessage(GetError(GetLastError()));
					return GetLastError();
				}
			}
		}
		else
			exit = true; // ��������� �������� ���������
		PostQuitMessage(0); // �������� ���������
		break;
	case WM_COMMAND:
		if(_wParam == ID_BUTTON)
			if (!func_otvet()) // ����� ������� �������� ������
			{
				exit = true;
				PostQuitMessage(GetError(GetLastError()));
				return GetLastError();
			}
	case WM_CHAR:
		if (_wParam == VK_RETURN) // ���� ������ ������� Enter
			if (!func_otvet()) // ����� ������� �������� ������
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
		if (!KillTimer(::hWnd, ID_TIMER)) // ����������� ������� ���� �� ����� ������ MessageBox
			return false;
	int len = GetWindowTextLength(::hEdit); // ����� ������ � ���� �����
	if (len)
	{
		wchar_t otvet[SIZE_ANSWER] = { 0 }; // �����, �������� � ����������
		wchar_t otv[SIZE_ANSWER] = { 0 };
		SendMessage(::hEdit, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)&otvet[0]); // ��������� ������ �� ���� �����
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
		int r = _wcsicmp(o, otv); // ��������� ������, ��������� � ���������� � ��������� �������
		_wsetlocale(LC_ALL, loc);
		if (!r)
		{
			MessageBox(::hWnd, L"�����������!", L"\u263a\u263a\u263a", NULL);
			SendMessage(::hWnd, WM_DESTROY, (WPARAM)CLOSE_WND, NULL); // �������� ����
		}
		else
		{
			MessageBox(::hWnd, L"�����������!\r\n������ ���������!", L"\u2639\u2639\u2639", NULL);
			::hTimer = SetTimer(::hWnd, ID_TIMER, 500, NULL); // ������
			if (!::hTimer)
				return false;
		}
	}
	else
	{
		if (!MessageBeep(MB_ICONERROR)) // ���� ������
			return false;
		if (!MessageBox(::hWnd, L"����� �� ������", L"�����", MB_ICONWARNING)) // ��������� �� ������
			return false;

		::hTimer = SetTimer(::hWnd, ID_TIMER, 500, NULL); // ������
		if (!::hTimer)
			return false;
	}
	return true;
}
DWORD GetError(DWORD _Error)
{
	if (::hTimer)
		KillTimer(::hWnd, ID_TIMER); // ����������� ������� ���� �� ����� ������� MessageBox
	LPVOID lperr = NULL;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, _Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lperr, NULL, NULL); // �������������� ���� ������ � ������
	MessageBox(::hWnd, (LPCWSTR)lperr, NULL, MB_ICONERROR); // ����� ��������� �� ������
	LocalFree(lperr);
	return _Error;
}