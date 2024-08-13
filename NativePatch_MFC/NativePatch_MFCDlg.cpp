// NativePatch_MFCDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "NativePatch_MFC.h"
#include "NativePatch_MFCDlg.h"
#include "afxdialogex.h"
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "Patch.h"
#include "ico.h"
#include "RemoveAuthenticodeSignature.h"
#include "UAC.h"
#include "ConsoltoGUI.h"
using namespace std;

// 静态成员初始化
CEdit* CNativePatchMFCDlg::s_pLogEdit = nullptr;
CFont m_font;
// CNativePatchMFCDlg 对话框

CNativePatchMFCDlg::CNativePatchMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NATIVEPATCH_MFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNativePatchMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, myfile2);
	DDX_Control(pDX, IDC_EDIT_LOG, log_str);
}

BEGIN_MESSAGE_MAP(CNativePatchMFCDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CNativePatchMFCDlg::OnBnClickedOk)
	ON_WM_DROPFILES()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CNativePatchMFCDlg 消息处理程序
// 在实现文件中完成 OnInitDialog 方法
BOOL CNativePatchMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 设置 DWM 窗口属性，将标题栏设置为黑色
	DWORD color = RGB(60,60,60); // 黑色
	HRESULT hr = DwmSetWindowAttribute(m_hWnd, DWMWA_CAPTION_COLOR, &color, sizeof(color));

	if (FAILED(hr))
	{
		// 处理错误
		//AfxMessageBox(_T("无法设置标题栏颜色"));
	}
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_LOG);
	if (pEdit)
	{
		pEdit->ShowScrollBar(SB_BOTH, FALSE); // 隐藏水平和垂直滚动条
	}

	// 创建Cascadia Code字体，设置为10pt大小
	m_font.CreatePointFont(90, _T("微软雅黑"));

	// 获取对话框中的第一个控件
	CWnd* pWnd = GetWindow(GW_CHILD);

	// 遍历所有控件并设置字体
	while (pWnd)
	{
		pWnd->SetFont(&m_font); // 设置控件字体
		pWnd = pWnd->GetNextWindow(); // 获取下一个控件
	}

	// 设置对话框背景颜色
	m_brush.CreateSolidBrush(RGB(60, 60, 60)); // 暗灰色背景
	this->SetBackgroundColor(RGB(60, 60, 60));

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	// 执行此操作
	SetIcon(m_hIcon, TRUE);    // 设置大图标
	SetIcon(m_hIcon, FALSE);   // 设置小图标

	// 初始化静态指针，指向日志编辑框控件
	s_pLogEdit = &log_str;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

HBRUSH CNativePatchMFCDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// 根据 nCtlColor 确定当前处理的控件类型

	// 设置对话框背景颜色
	if (nCtlColor == CTLCOLOR_DLG)
	{
		return m_brush;  // 返回自定义的背景画刷
	}

	// 设置静态文本颜色
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetTextColor(RGB(180, 180, 188)); // 设置字体为白色
		pDC->SetBkColor(RGB(60, 60, 60));      // 设置背景为暗灰色
		return m_brush;
	}

	// 设置编辑框背景颜色和字体颜色
	if (nCtlColor == CTLCOLOR_EDIT)
	{
		pDC->SetTextColor(RGB(255, 255, 255)); // 设置字体为白色
		pDC->SetBkColor(RGB(60, 60, 60));     // 设置背景为暗灰色
		
		return m_brush;
	}

	// 设置按钮背景颜色
	if (nCtlColor == CTLCOLOR_BTN)
	{
		pDC->SetTextColor(RGB(255, 255, 255)); // 设置字体为白色
		pDC->SetBkColor(RGB(60, 60, 60));      // 设置按钮背景为深灰色
		return m_brush;
	}
	else
	{
		pDC->SetTextColor(RGB(255, 255, 255)); // 设置字体为白色
		pDC->SetBkColor(RGB(60, 60, 60));      // 设置按钮背景为深灰色
		return m_brush;
	}
	// 对于未处理的情况，使用默认的处理方式
	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}


void CNativePatchMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CNativePatchMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

string CStringTostring(CString cstr)
{
	int codePage = CP_ACP;
	int requiredSize = WideCharToMultiByte(codePage, 0, (LPCWSTR)cstr, -1, nullptr, 0, nullptr, nullptr);
	std::vector<char> buffer(requiredSize);
	WideCharToMultiByte(codePage, 0, (LPCWSTR)cstr, -1, &buffer[0], requiredSize, nullptr, nullptr);
	return std::string(&buffer[0], &buffer[0] + requiredSize - 1);
}
void RemoveQuotes(CString& str)
{
	// 移除字符串中所有的引号
	str.Remove(_T('\"'));
}

void CNativePatchMFCDlg::OnBnClickedOk()
{
	SetDlgItemText(IDC_EDIT_LOG, NULL);
	CNativePatchMFCDlg::LogMessage(_T("开始运行：						By:Laster"));
	CNativePatchMFCDlg::LogMessage(_T("-------------------------------------------------------------------------------------------------"));
	CString modifyPeFilePath(_T(""));
	GetDlgItemText(IDC_EDIT1, modifyPeFilePath); // 获取文件路径
	CString textOrPePath(_T(""));
	GetDlgItemText(IDC_EDIT2, textOrPePath); // 获取另一个路径
	CString output = modifyPeFilePath + "_Patch.exe";
	RemoveQuotes(modifyPeFilePath);
	RemoveQuotes(textOrPePath);
	RemoveQuotes(output);
	CNativePatchMFCDlg::LogMessage(_T("[+]白文件路径："+ modifyPeFilePath));
	CNativePatchMFCDlg::LogMessage(_T("[+]黑文件路径：" + textOrPePath));
	CNativePatchMFCDlg::LogMessage(_T("[+]输出路径：" + output));
	std::vector<char> bytes = execute(CStringTostring(modifyPeFilePath), CStringTostring(textOrPePath));

	if (((CButton*)GetDlgItem(IDC_CHECK_ICO))->GetCheck() == 1) // 需要修改ico
	{
		 changeIcon(&bytes,"C:\\Users\\Laster\\Pictures\\1.ico");
		 //writeToFile("1.exe", bytes);
	}
	if (!bytes.empty())
	{
		writeToFile(CStringTostring(output), bytes);//写出文件
		if (((CButton*)GetDlgItem(IDC_CHECK_Signature))->GetCheck() == 1) // 需要修改签名
		{
			RemoveAuthenticodeSignature(output);
		}
		if (((CButton*)GetDlgItem(IDC_CHECK_UAC))->GetCheck() == 1) // 需要修改UAC
		{
			// 将 CString 转换为 std::wstring
			std::wstring wstr(output.GetString());
			std::wcout << L"[+] : " << wstr << std::endl;
			ModifyManifestResource(wstr);
		}
	}
	if (((CButton*)GetDlgItem(IDC_CHECK_GUI))->GetCheck() == 1) // 需要修改为GUI程序
	{
		if (ConvertToGUI(CStringTostring(output).c_str())) 
		{
			CNativePatchMFCDlg::LogMessage(_T("[+]已转换为GUI程序"));
		}
		else
		{
			CNativePatchMFCDlg::LogMessage(_T("[!]转换为GUI失败"));
		}
	}
	CNativePatchMFCDlg::LogMessage(_T("-------------------------------------------------------------------------------------------------"));
	CNativePatchMFCDlg::LogMessage(_T("程序结束"));
}

void CNativePatchMFCDlg::OnDropFiles(HDROP hDropInfo)
{
	TCHAR szPath[MAX_PATH] = { 0 };
	DragQueryFile(hDropInfo, 0, szPath, MAX_PATH);
	CString path1(_T(""));
	CString path2(_T(""));

	GetDlgItemText(IDC_EDIT1, path1); // 获取文件路径
	GetDlgItemText(IDC_EDIT2, path2); // 获取文件路径
	if (!path1.IsEmpty())
	{
		SetDlgItemText(IDC_EDIT2, szPath);
	}
	if (!path2.IsEmpty())
	{
		SetDlgItemText(IDC_EDIT_ICO, szPath);
	}
	if (path1.IsEmpty()&& path2.IsEmpty())
	{
		SetDlgItemText(IDC_EDIT1, szPath);

	}
	UpdateData(TRUE);
	CDialogEx::OnDropFiles(hDropInfo);

}

// 静态日志函数
void CNativePatchMFCDlg::LogMessage(CString str)
{
	if (s_pLogEdit)
	{
		CString currentText;
		s_pLogEdit->GetWindowText(currentText); // 获取当前日志内容

		currentText += str + _T("\r\n"); // 追加新日志内容并换行

		s_pLogEdit->SetWindowText(currentText); // 更新日志内容
	}
}

