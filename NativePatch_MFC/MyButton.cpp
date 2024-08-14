#include "pch.h"
#include "MyButton.h"


MyButton::MyButton() {}
MyButton::~MyButton() {}

BEGIN_MESSAGE_MAP(MyButton, CButton)
    ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void MyButton::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    // ��ȡ��ť�ľ�������
    CRect rect;
    GetClientRect(&rect);

    // ���ñ�����ɫΪRGB(60, 60, 60)
    CBrush brush(RGB(111, 111, 111));
    dc.FillRect(&rect, &brush);

    // ����1������ɫ�߿�
    CPen pen(PS_SOLID, 1, RGB(128, 0, 128)); // ��ɫ�߿�
    CPen* pOldPen = dc.SelectObject(&pen);
    dc.Rectangle(rect);

    // ��ԭ�ɵĻ���
    dc.SelectObject(pOldPen);
}
HBRUSH MyButton::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = MyButton::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{

	case IDOK:
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetBkColor(RGB(255, 0, 255));
		pDC->SetTextColor(RGB(255, 0, 0));
		static CBrush brush(RGB(255, 0, 255));
		return brush;
	}
	break;
	return hbr;
	}
}

