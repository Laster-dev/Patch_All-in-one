#pragma once
#include <afxwin.h>
class MyButton :
    public CButton
{
public:
    MyButton();
    virtual ~MyButton();

protected:
    afx_msg void OnPaint();
    HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    DECLARE_MESSAGE_MAP()
};

