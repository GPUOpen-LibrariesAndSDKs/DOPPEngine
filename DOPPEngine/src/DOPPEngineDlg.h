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

#pragma once

class DisplayManager;
class DOPPRotationDlg;

class CDOPPEngineDlg : public CDialogEx
{
public:
    CDOPPEngineDlg(CWnd* pParent = nullptr);
    ~CDOPPEngineDlg();

    enum { IDD = IDD_DOPPENGINE_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

protected:
    HICON m_hIcon;

    virtual BOOL        OnInitDialog();
    afx_msg void        OnPaint();
    afx_msg HCURSOR     OnQueryDragIcon();
    afx_msg void        OnStart();
    afx_msg void        OnStop();
    afx_msg void        OnExit();

    DECLARE_MESSAGE_MAP()

private:

    enum EffectId { NO_EFFECT, COLOR_INVERT, EDGE_FILTER, DISTORT_EFFECT, ROTATE_DESKTOP, EYEFINITY_LANDSCAPE, EYEFINITY_PORTRAIT };

    struct THREAD_DATA
    {
        CDOPPEngineDlg*     pDoppEngine;
        DWORD               dwThreadId;
        unsigned int        uiDesktop;
        HWND                hWnd;
        HDC                 hDC;
        HGLRC               hCtx;
    };

    bool                createGLWindow(unsigned int id, wchar_t* pClassName);
    void                showWindow(unsigned int id);
    void                deleteWindow(unsigned int id, wchar_t* pClassName);

    static DWORD        threadFunction(void* pData);
    DWORD               threadLoop(unsigned int uiDesktop);

    bool                    m_bEngineRunning;
    bool                    m_bShowCaptureWin;
    bool                    m_bBlockingPrsent;

    unsigned int            m_uiNumSelectedDesktops;
    unsigned int            m_uiEffectSelection;
    unsigned int            m_uiRotationAngle;

    CComboBox*              m_pEffectComboBox;
    CListBox*               m_pDesktopListBox;
    CButton*                m_pShowWinCheckBox;
    CButton*                m_pBlockingPresentBox;

    DisplayManager*         m_pDisplayManager;

    DOPPRotationDlg*        m_pRotationDlg;

    HANDLE*                 m_pThreads;
    struct THREAD_DATA*     m_pThreadData;

    CRITICAL_SECTION        m_CriticalSection;
};