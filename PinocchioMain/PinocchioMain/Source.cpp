#include <Windows.h>
#include <tchar.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#define ID_TIMER ( WM_USER + 1 ) // ID �������
#define ID_EDIT ( WM_USER + 2 ) // ID Edit
#define ID_BUTTON ( WM_USER + 3 ) // ID Button
#define CLOSE_WND ( WM_USER + 14 )
#define SIZE_PASSWORD 20

static HINSTANCE hinstance = NULL;
static LPCWSTR CLASS_NAME = L"Pinocchio_Question"; // ��� ������ ����
static LPCWSTR WND_NAME = L"Pinocchio"; // ��������� ����
HWND hWnd = NULL; // ���������� ����
HWND hEdit = NULL; // ���������� Edit
HWND hButton = NULL; // ���������� Button
static UINT_PTR hTimer = NULL; // ���������� �������
WCHAR path_exe[MAX_PATH] = { 0 }; // ���� � ������������ �����

DWORD GetError(DWORD);
bool Auth(void);

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

	::hWnd = CreateWindow(CLASS_NAME, WND_NAME, WS_OVERLAPPED | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, 200, 150, NULL, NULL, ::hinstance, NULL); // �������� ����
	if (!::hWnd)
		return GetError(GetLastError());

	::hEdit = CreateWindowEx(NULL, L"EDIT", NULL, ES_PASSWORD | WS_CHILD | WS_VISIBLE | WS_BORDER, 20, 20, 150, 20, ::hWnd, (HMENU)ID_EDIT, ::hinstance, NULL); // �������� ���� ����� ��� ������
	if (!::hEdit)
		return GetError(GetLastError());

	::hButton = CreateWindowEx(NULL, L"BUTTON", L"���������", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, 20, 60, 150, 20, ::hWnd, (HMENU)ID_BUTTON, ::hinstance, NULL); // �������� ������
	if (!::hButton)
		return GetError(GetLastError());

	SendMessage(::hEdit, EM_SETPASSWORDCHAR, (WPARAM)L'\u2716', NULL);

	if (!AnimateWindow(::hWnd, 300, AW_ACTIVATE | AW_VER_POSITIVE | AW_SLIDE)) // �������� ��� ��������� ����
		return GetLastError();

	ShowWindow(::hWnd, SW_MINIMIZE); // ����������� ����

	UpdateWindow(::hWnd); // ���������� ����

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		// ��������� ������� Enter � Edit
		if ((WM_KEYDOWN == msg.message) && (VK_RETURN == msg.wParam) && (msg.hwnd == hEdit))
		{
			if (!Auth()) // ����� ������� �������� ������
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

	_SHELLEXECUTEINFOW sei = { NULL };
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_DEFAULT;
	sei.lpVerb = L"open";
	sei.lpFile = L"\"C:\\Program Files\\Pinocchio\\Question.exe\"";
	sei.nShow = SW_SHOW;
	switch (_Msg)
	{
	case WM_CREATE:
		::hTimer = SetTimer(_hWnd, ID_TIMER, 300000, NULL); // ������
		if (!::hTimer)
		{
			exit = true;
			PostQuitMessage(GetError(GetLastError()));
			return GetLastError();
		}
		MessageBox(::hWnd, L"Pinocchio �������!", L"Pinocchio", MB_ICONINFORMATION);
		break;
	case WM_NCPAINT:
		SendMessage(::hWnd, WM_PAINT, _wParam, _lParam); // ����� WM_PAINT
		break;
	case WM_TIMER:
		ShellExecuteEx(&sei); // ������ ��������� Question.exe
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
		if (_wParam == ID_BUTTON)
			if (!Auth()) // ����� ������� �������� ������
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
		if (!KillTimer(::hWnd, ID_TIMER)) // ����������� ������� ���� �� ����� ������ MessageBox
			return false;

	int len = GetWindowTextLength(::hEdit); // ����� ������ � ���� �����
	if (len)
	{
		wchar_t passw[SIZE_PASSWORD] = { 0 }; // �����, �������� � ����������
		SendMessage(::hEdit, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)&passw[0]); // ��������� ������ �� ���� �����
		wchar_t *loc = _wsetlocale(LC_ALL, L"");
		int r = _wcsicmp(L"gfhjkm", passw); // ��������� ������, ��������� � ���������� � ��������� �������
		_wsetlocale(LC_ALL, loc);
		if (!r)
		{
			MessageBox(::hWnd, L"��!", L"\u263a\u263a\u263a", NULL);
			SendMessage(::hWnd, WM_DESTROY, (WPARAM)CLOSE_WND, NULL); // �������� ����
		}
		else
		{
			MessageBox(::hWnd, L"�������� ������!\r\n������ ���������!", L"\u2639\u2639\u2639", NULL);
			::hTimer = SetTimer(::hWnd, ID_TIMER, 300000, NULL); // ������
			if (!::hTimer)
				return false;
		}
	}
	else
	{
		if (!MessageBox(::hWnd, L"������ �� ������!", L"", MB_ICONWARNING)) // ��������� �� ������
			return false;

		::hTimer = SetTimer(::hWnd, ID_TIMER, 300000, NULL); // ������
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