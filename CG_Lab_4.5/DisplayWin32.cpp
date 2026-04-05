#include "DisplayWin32.h"


namespace CGLib {
    DisplayWin32* DisplayWin32::s_instance_ = nullptr;


    DisplayWin32::DisplayWin32() { s_instance_ = this; }
    DisplayWin32::~DisplayWin32() { Shutdown(); }

    bool DisplayWin32::Initialize(const wchar_t* title, int width, int height, HINSTANCE hInstance, InputDevice* input)
    {
        hInstance_ = hInstance;
        width_ = width;
        height_ = height;
        inputDevice_ = input;
        className_ = title;
        running_ = true;

        WNDCLASSEX wc = {};
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = StaticWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
        wc.hIconSm = wc.hIcon;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = title;
        wc.cbSize = sizeof(WNDCLASSEX);

        if (!RegisterClassEx(&wc)) return false;

        RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        // Убрал WS_THICKFRAME чтобы окно не растягиивалось
        auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
        auto posX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
        auto posY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

        hWnd_ = CreateWindowEx(
            WS_EX_APPWINDOW,
            title,
            title,
            dwStyle,
            posX,
            posY,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,
            nullptr,
            hInstance,
            this
        );

		// Ограничиваем курсор рамками окна и скрываем его
		//RECT rect;
		//GetClientRect(hWnd_, &rect);                  // координаты клиентской области окна
		//MapWindowPoints(hWnd_, nullptr, (POINT*)&rect, 2);  // переводим в экранные координаты
		//ClipCursor(&rect);                            // ограничиваем курсор
		//ShowCursor(FALSE);                            // скрываем курсор
		//ShowCursor(TRUE);                            // скрываем курсор

        if (!hWnd_) {
			std::cout << "Failed to create window\n";
            return false;
        }

		ShowWindow(hWnd_, SW_SHOW);
		SetForegroundWindow(hWnd_);
		SetFocus(hWnd_);

		RAWINPUTDEVICE Rid;
		Rid.usUsagePage = 0x01;
		Rid.usUsage = 0x02;
		Rid.dwFlags = RIDEV_INPUTSINK;
		Rid.hwndTarget = hWnd_;

		RegisterRawInputDevices(&Rid, 1, sizeof(Rid));
		std::cout << "Window created successfully\n";
        return true;
    }

    void DisplayWin32::Shutdown()
    {
		ClipCursor(nullptr);
		ShowCursor(TRUE);

        if (hWnd_) {
            DestroyWindow(hWnd_);
            hWnd_ = nullptr;
        }
        UnregisterClass(className_.c_str(), hInstance_);
    }

    bool DisplayWin32::ProcessMessages(InputDevice* input)
    {

		
		if (hWnd_) {
			RECT rect;
			GetClientRect(hWnd_, &rect);
			MapWindowPoints(hWnd_, nullptr, (POINT*)&rect, 2);
			//ClipCursor(&rect);
		}

        MSG msg = {};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                running_ = false;
                return false;
            }
        }
        return true;
    }

    LRESULT DisplayWin32::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, InputDevice* input)
    {
        switch (message)
        {
        case WM_KEYDOWN:
            if (input) input->ProcessKeyDown(static_cast<unsigned int>(wParam));
            if (static_cast<unsigned int>(wParam) == VK_ESCAPE) PostQuitMessage(0);
            return 0;

        case WM_KEYUP:
            if (input) input->ProcessKeyUp(static_cast<unsigned int>(wParam));
            return 0;

			/*case WM_MOUSEMOVE:
				if (input) {
					POINT p;
					GetCursorPos(&p);
					ScreenToClient(hwnd, &p);
					input->ProcessMouseMove(p.x, p.y);
				}
				return 0;*/

		/*case WM_MOUSEWHEEL:
			wheelDelta_ += GET_WHEEL_DELTA_WPARAM(wParam);
			break;*/

		case WM_INPUT:
		{
			UINT size = 0;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));

			BYTE buffer[sizeof(RAWINPUT)];
			if (size > sizeof(buffer)) return 0;

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)buffer;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				int dx = raw->data.mouse.lLastX;
				int dy = raw->data.mouse.lLastY;

				if (input)
					input->ProcessMouseMove(dx, dy);
			}
		}
		return 0;

		case WM_MOUSEWHEEL:
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			input->ProcessMouseWheel(delta);
			return 0;
		}

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }


        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    LRESULT CALLBACK DisplayWin32::StaticWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            s_instance_ = static_cast<DisplayWin32*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(s_instance_));
        }
        else {
            s_instance_ = reinterpret_cast<DisplayWin32*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (s_instance_) {
            return s_instance_->WndProc(hwnd, message, wParam, lParam, s_instance_->inputDevice_);
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}
