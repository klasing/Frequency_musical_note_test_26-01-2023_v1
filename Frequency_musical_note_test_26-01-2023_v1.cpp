// Frequency_musical_note_test_26-01-2023_v1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Frequency_musical_note_test_26-01-2023_v1.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

//Note oNote;
HWND g_hDlg = { 0 };

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
LoadStringW(hInstance, IDC_FREQUENCYMUSICALNOTETEST26012023V1, szWindowClass, MAX_LOADSTRING);
MyRegisterClass(hInstance);

// Perform application initialization:
if (!InitInstance(hInstance, nCmdShow))
{
    return FALSE;
}

HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FREQUENCYMUSICALNOTETEST26012023V1));

MSG msg;

// Main message loop:
while (GetMessage(&msg, nullptr, 0, 0))
{
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FREQUENCYMUSICALNOTETEST26012023V1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_FREQUENCYMUSICALNOTETEST26012023V1);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW
        , 10//CW_USEDEFAULT
        , 10//0
        , 500//CW_USEDEFAULT
        , 500//0
        , nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
/*    CHAR szOutputDebugStringBuffer[64];*/
    switch (message)
    {
    case WM_NCCREATE:
    {
        // create dialog
        g_hDlg = CreateDialog(g_hInst, L"DLGPROCWINDOW", hWnd, DlgProc);

        return DefWindowProc(hWnd, message, wParam, lParam);
    } // eof WM_NCCREATE

    case WM_CREATE:
    {

        return (INT_PTR)TRUE;
    } // eof WM_CREATE
    case WM_SIZE:
    {
        RECT rect;
        GetClientRect(hWnd, &rect);
        // set size dialog and show dialog
        SetWindowPos(g_hDlg
            , HWND_TOP
            , rect.left
            , rect.top
            , rect.right
            , rect.bottom
            , SWP_SHOWWINDOW
        );

        return (INT_PTR)TRUE;
    } // eof WM_SIZE
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_NCDESTROY:
    {
        return (INT_PTR)FALSE;
    } // eof WM_NCDESTROY
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//****************************************************************************
//*                     DlgProc
//****************************************************************************
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HRESULT hr = S_OK;
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        onWmInitDialog_DlgProc(g_hInst
            , hDlg
        );

        return (INT_PTR)FALSE;
    } // eof WM_INITDIALOG
    case WM_SIZE:
    {
        onWmSize_DlgProc(hDlg
        );

        return (INT_PTR)TRUE;
    } // eof WM_SIZE
    case WM_COMMAND:
    {

        onWmCommand_DlgProc(hDlg
            , wParam
        );

        // this break is vital, otherwise a WM_COMMAND falls
        // through into the underlying message handler!
        break;
    } // eof WM_COMMAND
    } // eof switch

    return (INT_PTR)FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

/*
    HRESULT hr = S_OK;
    IMMDeviceEnumerator* pEnumerator = NULL;
    // get the unique device ID
    IMMDevice* dev = NULL;
    wchar_t* device_id = NULL;
    hr = dev->GetId(&device_id);
    // get the system default device
    IMMDevice* def_dev = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender
        , eMultimedia
        , &def_dev
    );
        // initialize the COM-interface subsystem
        hr = CoInitializeEx(NULL, 0);
        // enumerate the devices
        IMMDeviceEnumerator* pEnumerator = NULL;
        const CLSID CLSID_MMDeviceEnumerator = { 0xbcde0395
        //const GUID _CLSID_MMDeviceEnumerator = { 0xbcde0395
            , 0xe52f
            , 0x467c
            , {0x8e, 0x3d, 0xc4,0x57,0x92,0x91,0x69,0x2e}
        };
        const IID IID_IMMDeviceEnumerator = { 0xa95664d2
        //const GUID _IID_IMMDeviceEnumerator = { 0xa95664d2
            , 0x9614
            , 0x4f35
            , {0xa7,0x46, 0xde,0x8d,0xb6,0x36,0x17,0xe6}
        };
        CoCreateInstance(CLSID_MMDeviceEnumerator
            , NULL
            , CLSCTX_ALL
            , IID_IMMDeviceEnumerator
            , (void**)&pEnumerator
        );
        //CoCreateInstance(&_CLSID_MMDeviceEnumerator
        //    , NULL
        //    , CLSCTX_ALL
        //    , &_IID_IMMDeviceEnumerator
        //    , (void**)&enu
        //);

        HRESULT hr = S_OK;

        //hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        IMMDeviceEnumerator* pEnumerator = NULL;
        IMMDeviceCollection* pCollection = NULL;
        IMMDevice* pEndPoint = NULL;
        IPropertyStore* pProps = NULL;

        const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
        const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
        hr = CoCreateInstance(CLSID_MMDeviceEnumerator
            , NULL
            , CLSCTX_ALL
            , IID_IMMDeviceEnumerator
            , (void**)&pEnumerator
        );

        hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATEMASK_ALL, &pCollection); //DEVICE_STATE_ACTIVE, &pCollection);

        hr = pCollection->Item(0, &pEndPoint);
    case WM_NCCREATE:
    {
        oNote.init();
        return (INT_PTR)TRUE;
    } // eof WM_NCCREATE

        CoUninitialize();
        
        //////////////////////////////////////////////////////////////////////
        IMMEndpoint* pEnumerator;
        const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
        const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
        HRESULT hr = CoCreateInstance(
            CLSID_MMDeviceEnumerator, NULL,
            CLSCTX_ALL, IID_IMMDeviceEnumerator,
            (void**)&pEnumerator);
        /////////////////////////////////////////////////////////////////////
        for (BYTE x = 4; x < NOF_ALL_OCTAVE - 2; x++)
        {
            for (BYTE y = 0; y < NOF_NOTE_PER_OCTAVE; y++)
            {
                Beep((1. / (oNote.aTone[x][y]) * 1e6), 500);
                sprintf_s(szOutputDebugStringBuffer
                    , (size_t)64
                    , "x: %2d y: %2d\n"
                    , x
                    , y
                );
                OutputDebugStringA(szOutputDebugStringBuffer);
            }
        }
    } // eof WM_CREATE
*/


