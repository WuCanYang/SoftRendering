#include <windows.h> 
#include <stdlib.h> 
#include <string.h> 
#include <tchar.h> 
#include <iostream>
#include "SoftRender.h"
#include "Model/Constant.h"


// ------------------------Global variables---------------------------------
// 主窗体类名
static TCHAR szWindowClass[] = _T("SoftRender");
// 应用程序标题栏处出现的字符串
static TCHAR szTitle[] = _T("SoftRender");

//HINSTANCE 是Windows里的一中数据类型，是用于标示（记录）一个程序的实例。
//它与HMODULE是一样的（通用的，这两种类型最终就是32位无符号长整形）。
//HINSTANCE，分开看就是 HANDLE(句柄) + INSTANCE（实例）
//实例就是一个程序。用qq来举例：你可以开同时开2个qq号，但是你电脑里的qq软件只有一份。
//这2个qq号就是qq的2个实例
HINSTANCE hInst;

// -------------------------需要预定义的函数放置在此代码块种:---------------------------- 


//每个 Windows 桌面应用程序必须具有一个窗口过程函数
//此函数处理应用程序从操作系统中接收的大量消息。 
//例如，如果应用程序的对话框中有“确定”按钮，那么用户单击该按钮时，
//操作系统会向应用程序发送一条消息，通知按钮已被单击。WndProc 负责对该事件作出响应。
//在本例中，相应的响应可能是关闭对话框。
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//-------------------------------------正式内容-----------------------------------------

//主窗体函数(入口过程)
//每个基于 Win32 的应用程序的函数必须具有 WinMain 函数
//正如每个 C 应用程序和 C++ 控制台应用程序在起始点必须具有 main 函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//创建 WNDCLASSEX 类型的窗口类结构。 此结构包含关于窗口的信息
	//例如应用程序图标、窗口背景色、标题栏中显示的名称、窗口过程函数的名称等。
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

	//对已创建的窗口类进行注册。 使用 RegisterClassEx 函数，并将窗口类结构作为参数传递。
	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Win32 Guided Tour"), NULL);
		return 1;
	}

	// Store instance handle in our global variable 
	// 将句柄实例存储于全局变量中
	hInst = hInstance;

	// CreateWindow 函数的参数解释: 
	// szWindowClass: the name of the application 
	// szTitle: the text that appears in the title bar 
	// WS_OVERLAPPEDWINDOW: the type of window to create 
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y) 
	// 500, 100: initial size (width, length) 
	// NULL: the parent of this window 
	// NULL: this application does not have a menu bar 
	// hInstance: the first parameter from WinMain 
	// NULL: not used in this application 
	// 返回的HWND是一个窗口的句柄
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
	// UpdateWindow函数用于更新窗口指定的区域
	// 如果窗口更新的区域不为空，UpdateWindow函数就发送一个WM_PAINT消息来更新指定窗口的客户区。
	// 函数绕过应用程序的消息队列，直接发送WM_PAINT消息给指定窗口的窗口过程
	// 如果更新区域为空，则不发送消息。
	UpdateWindow(hWnd);

	// Main message loop: 
	// 添加用于侦听操作系统所发送消息的消息循环。 
	// 当应用程序收到一条消息时，此循环将该消息调度到 WndProc 函数以进行处理。 
	MSG msg;
	while (!exit) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) exit = true;
			else
			{
				TranslateMessage(&msg); //翻译消息
				DispatchMessage(&msg);  //派遣消息
			}
		}
		else
		{
			engine.frame();
		}
	}
	return (int)msg.wParam;
}

// //  函数: WndProc(HWND, UINT, WPARAM, LPARAM) 
// //  目标:  处理主窗体产生的信息
// //  WM_PAINT消息代表绘制主窗体 
// //  WM_DESTROY消息代表投递一个退出消息并返回 
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR greeting[] = _T("Hello, World!");
	switch (message) {
	/*case WM_PAINT:
		//要处理 WM_PAINT 消息，首先应调用 BeginPaint
		//然后处理所有的逻辑以及在窗口中布局文本、按钮和其他控件等
		//然后调用 EndPaint。 
		hdc = BeginPaint(hWnd, &ps);

		// -----------------在这段代码对应用程序进行布局------------------------ 
		// 对于此应用程序，开始调用和结束调用之间的逻辑是在窗口中显示字符串 “Hello，World!”。
		// 请注意 TextOut 函数用于显示字符串。
		TextOut(hdc, 50, 5, greeting, _tcslen(greeting));
		// -----------------------布局模块结束----------------------------------

		EndPaint(hWnd, &ps);
		break;*/
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		//DefWindowProc缺省窗口处理函数
		//这个函数是默认的窗口处理函数，我们可以把不关心的消息都丢给它来处理
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}