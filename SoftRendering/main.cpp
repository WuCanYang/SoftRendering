#include <windows.h> 
#include <stdlib.h> 
#include <string.h> 
#include <tchar.h> 
#include <iostream>
#include "SoftRender.h"
#include "Model/Constant.h"


// ------------------------Global variables---------------------------------
// ����������
static TCHAR szWindowClass[] = _T("SoftRender");
// Ӧ�ó�������������ֵ��ַ���
static TCHAR szTitle[] = _T("SoftRender");

//HINSTANCE ��Windows���һ���������ͣ������ڱ�ʾ����¼��һ�������ʵ����
//����HMODULE��һ���ģ�ͨ�õģ��������������վ���32λ�޷��ų����Σ���
//HINSTANCE���ֿ������� HANDLE(���) + INSTANCE��ʵ����
//ʵ������һ��������qq������������Կ�ͬʱ��2��qq�ţ�������������qq���ֻ��һ�ݡ�
//��2��qq�ž���qq��2��ʵ��
HINSTANCE hInst;

// -------------------------��ҪԤ����ĺ��������ڴ˴������:---------------------------- 


//ÿ�� Windows ����Ӧ�ó���������һ�����ڹ��̺���
//�˺�������Ӧ�ó���Ӳ���ϵͳ�н��յĴ�����Ϣ�� 
//���磬���Ӧ�ó���ĶԻ������С�ȷ������ť����ô�û������ð�ťʱ��
//����ϵͳ����Ӧ�ó�����һ����Ϣ��֪ͨ��ť�ѱ�������WndProc ����Ը��¼�������Ӧ��
//�ڱ����У���Ӧ����Ӧ�����ǹرնԻ���
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//-------------------------------------��ʽ����-----------------------------------------

//�����庯��(��ڹ���)
//ÿ������ Win32 ��Ӧ�ó���ĺ���������� WinMain ����
//����ÿ�� C Ӧ�ó���� C++ ����̨Ӧ�ó�������ʼ�������� main ����
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//���� WNDCLASSEX ���͵Ĵ�����ṹ�� �˽ṹ�������ڴ��ڵ���Ϣ
	//����Ӧ�ó���ͼ�ꡢ���ڱ���ɫ������������ʾ�����ơ����ڹ��̺��������Ƶȡ�
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	//���Ѵ����Ĵ��������ע�ᡣ ʹ�� RegisterClassEx ����������������ṹ��Ϊ�������ݡ�
	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Win32 Guided Tour"), NULL);
		return 1;
	}

	// Store instance handle in our global variable 
	// �����ʵ���洢��ȫ�ֱ�����
	hInst = hInstance;

	// CreateWindow �����Ĳ�������: 
	// szWindowClass: the name of the application 
	// szTitle: the text that appears in the title bar 
	// WS_OVERLAPPEDWINDOW: the type of window to create 
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y) 
	// 500, 100: initial size (width, length) 
	// NULL: the parent of this window 
	// NULL: this application does not have a menu bar 
	// hInstance: the first parameter from WinMain 
	// NULL: not used in this application 
	// ���ص�HWND��һ�����ڵľ��
	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, hInstance, NULL);
	if (!hWnd) {
		MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Win32 Guided Tour"), NULL);
		return 1;
	}

	AllocConsole();
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);

	SoftRender engine;
	engine.Init(hWnd);
	bool exit = false;



	ShowWindow(hWnd, nCmdShow);
	// UpdateWindow�������ڸ��´���ָ��������
	// ������ڸ��µ�����Ϊ�գ�UpdateWindow�����ͷ���һ��WM_PAINT��Ϣ������ָ�����ڵĿͻ�����
	// �����ƹ�Ӧ�ó������Ϣ���У�ֱ�ӷ���WM_PAINT��Ϣ��ָ�����ڵĴ��ڹ���
	// �����������Ϊ�գ��򲻷�����Ϣ��
	UpdateWindow(hWnd);

	// Main message loop: 
	// ���������������ϵͳ��������Ϣ����Ϣѭ���� 
	// ��Ӧ�ó����յ�һ����Ϣʱ����ѭ��������Ϣ���ȵ� WndProc �����Խ��д��� 
	MSG msg;
	while (!exit) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) exit = true;
			else
			{
				TranslateMessage(&msg); //������Ϣ
				DispatchMessage(&msg);  //��ǲ��Ϣ
			}
		}
		else
		{
			engine.frame();
		}
	}
	return (int)msg.wParam;
}

// //  ����: WndProc(HWND, UINT, WPARAM, LPARAM) 
// //  Ŀ��:  �����������������Ϣ
// //  WM_PAINT��Ϣ������������� 
// //  WM_DESTROY��Ϣ����Ͷ��һ���˳���Ϣ������ 
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR greeting[] = _T("Hello, World!");
	switch (message) {
	/*case WM_PAINT:
		//Ҫ���� WM_PAINT ��Ϣ������Ӧ���� BeginPaint
		//Ȼ�������е��߼��Լ��ڴ����в����ı�����ť�������ؼ���
		//Ȼ����� EndPaint�� 
		hdc = BeginPaint(hWnd, &ps);

		// -----------------����δ����Ӧ�ó�����в���------------------------ 
		// ���ڴ�Ӧ�ó��򣬿�ʼ���úͽ�������֮����߼����ڴ�������ʾ�ַ��� ��Hello��World!����
		// ��ע�� TextOut ����������ʾ�ַ�����
		TextOut(hdc, 50, 5, greeting, _tcslen(greeting));
		// -----------------------����ģ�����----------------------------------

		EndPaint(hWnd, &ps);
		break;*/
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		//DefWindowProcȱʡ���ڴ�����
		//���������Ĭ�ϵĴ��ڴ����������ǿ��԰Ѳ����ĵ���Ϣ��������������
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}