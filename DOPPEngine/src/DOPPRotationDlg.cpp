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
#include "afxdialogex.h"

#include "DOPPEngine.h"
#include "DOPPRotationDlg.h"


IMPLEMENT_DYNAMIC(DOPPRotationDlg, CDialogEx)


DOPPRotationDlg::DOPPRotationDlg(CWnd* pParent)
    : CDialogEx(DOPPRotationDlg::IDD, pParent)
    , m_pSlider(nullptr)
    , m_pSliderValue(nullptr)
    , m_uiAngle(0)
{}


DOPPRotationDlg::~DOPPRotationDlg()
{}


void DOPPRotationDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DOPPRotationDlg, CDialogEx)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_ANGLE, &DOPPRotationDlg::OnNMCustomdrawSliderAngle)
    ON_BN_CLICKED(IDOK, &DOPPRotationDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCLE, &DOPPRotationDlg::OnBnClickedCancle)
END_MESSAGE_MAP()


void DOPPRotationDlg::OnNMCustomdrawSliderAngle(NMHDR* pNMHDR, LRESULT* pResult)
{
    m_uiAngle = m_pSlider->GetPos();

    char buf[8];
    sprintf_s(buf, " %d", m_uiAngle);

    m_pSliderValue->SetWindowTextW(CA2CT(buf));
}


BOOL DOPPRotationDlg::OnInitDialog()
{
    m_pSlider = static_cast<CSliderCtrl*>(GetDlgItem(IDC_SLIDER_ANGLE));

    m_pSliderValue = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_VALUE));

    if (!m_pSlider || !m_pSliderValue)
    {
        return false;
    }

    m_pSlider->SetRange(0, 360, false);

    return true;
}


void DOPPRotationDlg::OnBnClickedOk()
{
    CDialogEx::OnOK();
}


void DOPPRotationDlg::OnBnClickedCancle()
{
    CDialogEx::OnCancel();
}