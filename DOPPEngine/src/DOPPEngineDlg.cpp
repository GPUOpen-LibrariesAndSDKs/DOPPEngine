//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "stdafx.h"
#include <string>

#include <GL/glew.h>
#include <GL/wglew.h>

#include "afxdialogex.h"
#include "DisplayManager.h"
#include "DOPPEngine.h"
#include "DOPPEngineDlg.h"
#include "DOPPRotationDlg.h"

#include "GLDOPPColorInvert.h"
#include "GLDOPPDistort.h"
#include "GLDOPPEdgeFilter.h"
#include "GLDOPPEngine.h"
#include "GLDOPPEyefinityLandscape.h"
#include "GLDOPPEyefinityPortrait.h"
#include "GLDOPPRotate.h"

#define TITLE L"DOPP Engine v 0.3"

static unsigned int g_uiWindowWidth = 800;
static unsigned int g_uiWindowHeight = 600;

using namespace std;


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void MyDebugFunc(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
    MessageBoxA(NULL, message, "GL Debug Message", MB_ICONWARNING | MB_OK);
}


CDOPPEngineDlg::CDOPPEngineDlg(CWnd* pParent)
    : CDialogEx(CDOPPEngineDlg::IDD, pParent)
    , m_pThreads(nullptr)
    , m_pThreadData(nullptr)
    , m_bEngineRunning(false)
    , m_bShowCaptureWin(false)
    , m_bBlockingPrsent(false)
    , m_pEffectComboBox(nullptr)
    , m_pShowWinCheckBox(nullptr)
    , m_pDesktopListBox(nullptr)
    , m_uiEffectSelection(0)
    , m_uiNumSelectedDesktops(0)
    , m_pDisplayManager(nullptr)
    , m_pRotationDlg(nullptr)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


CDOPPEngineDlg::~CDOPPEngineDlg()
{
    if (m_pDisplayManager)
    {
        delete m_pDisplayManager;
        m_pDisplayManager = nullptr;
    }

    if (m_pRotationDlg)
    {
        delete m_pRotationDlg;
        m_pRotationDlg = nullptr;
    }

    if (m_pThreads)
    {
        delete[] m_pThreads;
        m_pThreads = nullptr;
    }

    if (m_pThreadData)
    {
        delete[] m_pThreadData;
        m_pThreadData = nullptr;
    }
}


void CDOPPEngineDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDOPPEngineDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_COMMAND(IDC_START, &CDOPPEngineDlg::OnStart)
    ON_COMMAND(IDC_STOP, &CDOPPEngineDlg::OnStop)
    ON_COMMAND(IDC_EXIT, &CDOPPEngineDlg::OnExit)
END_MESSAGE_MAP()


BOOL CDOPPEngineDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetWindowText(TITLE);

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    m_pEffectComboBox = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_EFFECT));

    m_pEffectComboBox->Clear();
    m_pEffectComboBox->InsertString(NO_EFFECT, _T("No Effect"));
    m_pEffectComboBox->InsertString(COLOR_INVERT, _T("Invert Colors"));
    m_pEffectComboBox->InsertString(EDGE_FILTER, _T("Edge Filter"));
    m_pEffectComboBox->InsertString(DISTORT_EFFECT, _T("Distort"));
    m_pEffectComboBox->InsertString(ROTATE_DESKTOP, _T("Rotate"));
    m_pEffectComboBox->InsertString(EYEFINITY_LANDSCAPE, _T("Eyefinity Landscape"));
    m_pEffectComboBox->InsertString(EYEFINITY_PORTRAIT, _T("Eyefinity Portrait"));

    m_pEffectComboBox->SetCurSel(0);
    m_uiEffectSelection = 0;

    m_pDisplayManager = new DisplayManager;

    m_pShowWinCheckBox = static_cast<CButton*>(GetDlgItem(IDC_CHECK_WINDOW));
    m_pShowWinCheckBox->SetCheck(BST_UNCHECKED);

    m_pBlockingPresentBox = static_cast<CButton*>(GetDlgItem(IDC_CHECK_BLOCK));
    m_pBlockingPresentBox->SetCheck(BST_UNCHECKED);

    m_pDesktopListBox = static_cast<CListBox*>(GetDlgItem(IDC_LIST_DESKTOP));

    unsigned int uiNumDesktops = m_pDisplayManager->enumDisplays();

    for (unsigned int i = 0; i < uiNumDesktops; ++i)
    {
        char buf[64];

        // Add 1 to the id to have the matching Desktop Id with CCC
        sprintf_s(buf, " Desktop %d on GPU %d", (i + 1), m_pDisplayManager->getGpuId(i));

        m_pDesktopListBox->InsertString(i, CA2CT(buf));
    }

    m_pDesktopListBox->SetCurSel(0);

    m_uiNumSelectedDesktops = 0;

    m_pRotationDlg = new DOPPRotationDlg;

    return TRUE;
}


void CDOPPEngineDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}


HCURSOR CDOPPEngineDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CDOPPEngineDlg::OnStart()
{
    if (!m_bEngineRunning)
    {
        m_uiEffectSelection = m_pEffectComboBox->GetCurSel();
        m_uiNumSelectedDesktops = m_pDesktopListBox->GetSelCount();

        // Clean thread data for the case that the engine ended with an error and OnStop was not called.
        if (m_pThreadData)
        {
            delete[] m_pThreadData;
            m_pThreadData = nullptr;
        }

        if (m_pThreads)
        {
            delete[] m_pThreads;
            m_pThreads = nullptr;
        }

        // check if the selected desktops are on different GPUs
        unsigned int uiNumGpuInSystem = m_pDisplayManager->getNumGPUs();

        bool* pGpuIsFree = new bool[uiNumGpuInSystem];

        bool bValidSelection = true;

        memset(pGpuIsFree, 1, uiNumGpuInSystem * sizeof(bool));

        for (int i = 0, iEnd = m_pDesktopListBox->GetCount(); i < iEnd; ++i)
        {
            if (m_pDesktopListBox->GetSel(i))
            {
                unsigned int uiGPU = m_pDisplayManager->getGpuId(i);

                if (pGpuIsFree[uiGPU])
                {
                    pGpuIsFree[uiGPU] = false;
                }
                else
                {
                    MessageBox(L"More than one desktop on the same GPU is selected!", L"DOPP Error", MB_ICONERROR | MB_OK);
                    bValidSelection = false;
                }
            }
        }

        delete[] pGpuIsFree;

        if (bValidSelection && m_uiNumSelectedDesktops > 0)
        {
            m_pThreads = new HANDLE[m_uiNumSelectedDesktops];
            m_pThreadData = new THREAD_DATA[m_uiNumSelectedDesktops];

            memset(m_pThreads, 0, sizeof(HANDLE));
            memset(m_pThreadData, 0, sizeof(THREAD_DATA));

            if (m_uiEffectSelection == ROTATE_DESKTOP && m_pRotationDlg)
            {
                if (m_pRotationDlg->DoModal() == IDOK)
                {
                    m_uiRotationAngle = m_pRotationDlg->getAngle();
                }
                else
                {
                    return;
                }
            }

            m_bShowCaptureWin = (m_pShowWinCheckBox->GetCheck() == BST_CHECKED);
            m_bBlockingPrsent = (m_pBlockingPresentBox->GetCheck() == BST_CHECKED);

            InitializeCriticalSection(&m_CriticalSection);

            // Loop through all entries of the listbox and start a thread for each selected
            m_bEngineRunning = true;

            unsigned int uiSel = 0;

            for (int i = 0, iEnd = m_pDesktopListBox->GetCount(); i < iEnd; ++i)
            {
                if (m_pDesktopListBox->GetSel(i) > 0)
                {
                    m_pThreadData[uiSel].pDoppEngine = this;
                    m_pThreadData[uiSel].uiDesktop = i;
                    m_pThreads[uiSel] = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&CDOPPEngineDlg::threadFunction), &m_pThreadData[uiSel], 0, &m_pThreadData[uiSel].dwThreadId);

                    ++uiSel;
                }
            }
        }
    }
}


void CDOPPEngineDlg::OnStop()
{
    if (m_bEngineRunning)
    {
        m_bEngineRunning = false;

        WaitForMultipleObjects(m_uiNumSelectedDesktops, m_pThreads, true, INFINITE);

        DeleteCriticalSection(&m_CriticalSection);

        if (m_pThreadData)
        {
            delete[] m_pThreadData;
            m_pThreadData = nullptr;
        }

        if (m_pThreads)
        {
            delete[] m_pThreads;
            m_pThreads = nullptr;
        }
    }
}


void CDOPPEngineDlg::OnExit()
{
    OnStop();
    OnOK();
}


DWORD CDOPPEngineDlg::threadFunction(void* pData)
{
    if (pData)
    {
        THREAD_DATA* pThreadData = reinterpret_cast<THREAD_DATA*>(pData);

        pThreadData->pDoppEngine->threadLoop(pThreadData->uiDesktop);
    }

    return 0;
}


DWORD CDOPPEngineDlg::threadLoop(unsigned int uiDesktop)
{
    THREAD_DATA* pMyData = &m_pThreadData[uiDesktop];

    GLDOPPEngine* pEngine = nullptr;

    switch (m_uiEffectSelection)
    {
        case COLOR_INVERT:
            pEngine = new GLDOPPColorInvert;
            break;

        case EDGE_FILTER:
            pEngine = new GLDOPPEdgeFilter;
            break;

        case DISTORT_EFFECT:
            pEngine = new GLDOPPDistort;
            break;

        case ROTATE_DESKTOP:
            pEngine = new GLDOPPRotate;
            break;

        case EYEFINITY_LANDSCAPE:
            pEngine = new GLDOPPEyefinityLandscape;
            break;

        case EYEFINITY_PORTRAIT:
            pEngine = new GLDOPPEyefinityPortrait;
            break;

        default:
            pEngine = new GLDOPPEngine;
            break;
    }

    // Enter critical section to make sure if init fails the subsequent
    // threads won't attempt to init again.
    EnterCriticalSection(&m_CriticalSection);

    wchar_t wcClassname[16];
    swprintf_s(wcClassname, L"OGL%d", pMyData->uiDesktop);

    // Create window with ogl context to load extensions
    if (m_bEngineRunning && !createGLWindow(uiDesktop, wcClassname))
    {
        m_bEngineRunning = false;
        MessageBox(L"Failed to create GL window!", L"GL Error", MB_ICONERROR | MB_OK);
    }

    // m_uiDesktopSelection is the id of the selected element in the Combo Box
    // Need to add 1 to get the dektop id as shown in CCC
    if (m_bEngineRunning && !pEngine->initDOPP(uiDesktop + 1))
    {
        m_bEngineRunning = false;
        MessageBox(L"Failed to initialize DOPP!", L"DOPP Error", MB_ICONERROR | MB_OK);
    }

    if (m_bEngineRunning && !pEngine->initEffect())
    {
        m_bEngineRunning = false;
        MessageBox(L"Failed to initialize DOPP Shaders!", L"DOPP Error", MB_ICONERROR | MB_OK);
    }

    LeaveCriticalSection(&m_CriticalSection);

    if (m_bEngineRunning && m_bShowCaptureWin)
    {
        // Create off screen context
        showWindow(uiDesktop);
    }

    if (m_bEngineRunning && m_uiEffectSelection == ROTATE_DESKTOP)
    {
        GLDOPPRotate* pRot = dynamic_cast<GLDOPPRotate*>(pEngine);

        pRot->setRotation(static_cast<float>(m_uiRotationAngle));
    }

    MSG mMsg;

    while (m_bEngineRunning)
    {
        pEngine->processDesktop(m_bBlockingPrsent);

        if (m_bShowCaptureWin)
        {
            // Blit FBO of DOPPEngine into the window
            glViewport(0, 0, g_uiWindowWidth, g_uiWindowHeight);

            unsigned int uiFBO = pEngine->getPresentBuffer();

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, uiFBO);

            glBlitFramebuffer(0, pEngine->getDesktopHeight(), pEngine->getDesktopWidth(), 0, 0, 0, g_uiWindowWidth, g_uiWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            SwapBuffers(pMyData->hDC);

            while (PeekMessage(&mMsg, NULL, 0, 0, PM_REMOVE) && mMsg.message != WM_QUIT)
            {
                TranslateMessage(&mMsg);
                DispatchMessage(&mMsg);
            }

            if (mMsg.message == WM_QUIT)
            {
                m_bEngineRunning = false;
            }
        }
    }

    delete pEngine;

    EnterCriticalSection(&m_CriticalSection);

    deleteWindow(uiDesktop, wcClassname);

    LeaveCriticalSection(&m_CriticalSection);

    return 0;
}


bool CDOPPEngineDlg::createGLWindow(unsigned int id, wchar_t* lpClassName)
{
    int mPixelFormat;

    WNDCLASSEX wndclass = {};

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_OWNDC;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = static_cast<HINSTANCE>(GetModuleHandle(NULL));
    wndclass.hIcon = static_cast<HICON>(LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, NULL));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = lpClassName;
    wndclass.hIconSm = static_cast<HICON>(LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, NULL));

    if (!RegisterClassEx(&wndclass))
    {
        return FALSE;
    }

    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;

    int nOriginX;
    int nOriginY;

    if (!m_pDisplayManager->getOrigin(id, nOriginX, nOriginY))
    {
        nOriginX = 0;
        nOriginY = 0;
    }

    wchar_t wcWindowName[32];
    swprintf_s(wcWindowName, L"Desktop %d", id);

    m_pThreadData[id].hWnd = CreateWindow(lpClassName,
                                          wcWindowName,
                                          dwStyle,
                                          nOriginX,
                                          nOriginY,
                                          g_uiWindowWidth,
                                          g_uiWindowHeight,
                                          NULL,
                                          NULL,
                                          static_cast<HINSTANCE>(AfxGetInstanceHandle()),
                                          nullptr);

    if (!m_pThreadData[id].hWnd)
    {
        return FALSE;
    }

    static PIXELFORMATDESCRIPTOR pfd = {};

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    m_pThreadData[id].hDC = ::GetDC(m_pThreadData[id].hWnd);

    if (!m_pThreadData[id].hDC)
    {
        return FALSE;
    }

    mPixelFormat = ChoosePixelFormat(m_pThreadData[id].hDC, &pfd);

    if (!mPixelFormat)
    {
        return FALSE;
    }

    SetPixelFormat(m_pThreadData[id].hDC, mPixelFormat, &pfd);

    m_pThreadData[id].hCtx = wglCreateContext(m_pThreadData[id].hDC);

    wglMakeCurrent(m_pThreadData[id].hDC, m_pThreadData[id].hCtx);

    if (glewInit() != GLEW_OK)
    {
        return false;
    }

    if (WGLEW_ARB_create_context)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(m_pThreadData[id].hCtx);

        int attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                          WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                          WGL_CONTEXT_PROFILE_MASK_ARB , WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef _DEBUG             
                          WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif                    
                          0 };

        m_pThreadData[id].hCtx = wglCreateContextAttribsARB(m_pThreadData[id].hDC, 0, attribs);

        if (m_pThreadData[id].hCtx)
        {
            wglMakeCurrent(m_pThreadData[id].hDC, m_pThreadData[id].hCtx);
        }
        else
        {
            return false;
        }
    }

    if (!wglMakeCurrent(m_pThreadData[id].hDC, m_pThreadData[id].hCtx))
    {
        return false;
    }

    if (GLEW_AMD_debug_output)
    {
        glDebugMessageCallbackAMD(reinterpret_cast<GLDEBUGPROCAMD>(&MyDebugFunc), nullptr);
    }

    return true;
}


void CDOPPEngineDlg::showWindow(unsigned int id)
{
    ::ShowWindow(m_pThreadData[id].hWnd, SW_SHOW);

    ::UpdateWindow(m_pThreadData[id].hWnd);
}


void CDOPPEngineDlg::deleteWindow(unsigned int id, wchar_t* lpClassName)
{
    if (m_pThreadData[id].hDC && m_pThreadData[id].hCtx)
    {
        wglMakeCurrent(m_pThreadData[id].hDC, NULL);
        wglDeleteContext(m_pThreadData[id].hCtx);
    }

    if (m_pThreadData[id].hDC && m_pThreadData[id].hWnd)
    {
        ::ReleaseDC(m_pThreadData[id].hWnd, m_pThreadData[id].hDC);
        ::DestroyWindow(m_pThreadData[id].hWnd);
    }

    ::UnregisterClass(lpClassName, NULL);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CHAR:
            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            return 0;

        case WM_CREATE:
            return 0;

        case WM_SIZE:
            g_uiWindowWidth = LOWORD(lParam);
            g_uiWindowHeight = HIWORD(lParam);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}