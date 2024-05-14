
// MFCMachineVisionDlg.cpp: 구현 파일
//
//패턴이랑 알고리즘에 mat 멤버 추가
//알고리즘에서 이미지처리 다하기
//하나하면 메시지
//다 하면 종료 메시지
//다이얼로그에서 이미지 가져오기
//get, set함수로
#include "pch.h"
#include "resource.h"
#include "afxdialogex.h"
#include "framework.h"
#include "MFCMachineVision.h"
#include "MFCMachineVisionDlg.h"
#include <time.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace cv;
using namespace std;
// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCMachineVisionDlg 대화 상자

CMFCMachineVisionDlg::CMFCMachineVisionDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCMACHINEVISION_DIALOG, pParent),
	m_pBitmapInfo(nullptr), m_strPatternName(_T("")), m_PtnHandler(nullptr),
	m_startTime(0), m_endTime(0), m_nModelCnt(1)
	
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCMachineVisionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Control(pDX, IDC_LIST_DEFECT, m_listDefect);
	DDX_Control(pDX, IDC_STATIC_RESULT, m_static_Result);
}

BEGIN_MESSAGE_MAP(CMFCMachineVisionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE(UM_LOGMSG, PrintLog)		// 카메라에서 로그 출력시
	ON_MESSAGE(UM_PRINTIMG, PrintImage) // InspectionHandler에서 그랩 후 이미지 로드
	ON_MESSAGE(UM_INSPEND, TimeCheck)	// 검사 종료 후 시간 측정
	ON_MESSAGE(UM_RESMSG, PrintRes)		// 모델 당 검사 결과 출력

	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMFCMachineVisionDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_START, &CMFCMachineVisionDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CMFCMachineVisionDlg::OnBnClickedButtonClear)
	//ON_STN_CLICKED(IDC_STATIC1, &CMFCMachineVisionDlg::OnStnClickedStatic1)
END_MESSAGE_MAP()


// CMFCMachineVisionDlg 메시지 처리기

BOOL CMFCMachineVisionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	m_InspectionHandler.Initialize(GetSafeHwnd());

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMFCMachineVisionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_CLOSE)
	{
		m_InspectionHandler.DisconnectCam();
		this->EndDialog(IDCANCEL);
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMFCMachineVisionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMFCMachineVisionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCMachineVisionDlg::CreateBitmapInfo(int w, int h, int bpp)
{
	if (m_pBitmapInfo != NULL)
	{
		delete m_pBitmapInfo;
		m_pBitmapInfo = NULL;
	}
	if (bpp == 8)
		m_pBitmapInfo = (BITMAPINFO*)new BYTE[sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD)];
	else
		m_pBitmapInfo = (BITMAPINFO*) new BYTE[sizeof(BITMAPINFO)];

	m_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBitmapInfo->bmiHeader.biPlanes = 1;
	m_pBitmapInfo->bmiHeader.biBitCount = bpp;
	m_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	m_pBitmapInfo->bmiHeader.biSizeImage = 0;
	m_pBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biClrUsed = 0;
	m_pBitmapInfo->bmiHeader.biClrImportant = 0;

	if (bpp == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			m_pBitmapInfo->bmiColors[i].rgbBlue = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbGreen = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbRed = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbReserved = 0;
		}
	}

	m_pBitmapInfo->bmiHeader.biWidth = w;
	m_pBitmapInfo->bmiHeader.biHeight = -h;
}

void CMFCMachineVisionDlg::DrawImage(CClientDC& dc,CRect& rect, const Mat& img) const
{
	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR);
	StretchDIBits(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, img.cols, img.rows, img.data, m_pBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
	Sleep(300);
}

LPARAM CMFCMachineVisionDlg::PrintImage(WPARAM wParam, LPARAM lParam)
{
	m_PtnHandler = reinterpret_cast<CPatternData*>(wParam);
	m_matImage = m_PtnHandler->GetPtnImg().clone();

	//이미지 출력
	CClientDC dc(GetDlgItem(IDC_PC_PREVIEW));
	CRect rect;
	GetDlgItem(IDC_PC_PREVIEW)->GetClientRect(&rect);

	CreateBitmapInfo(m_matImage.cols, m_matImage.rows, m_matImage.channels() * 8);
	DrawImage(dc, rect, m_matImage);

	return TRUE;
}

LPARAM CMFCMachineVisionDlg::PrintLog(WPARAM wParam, LPARAM lParam)
{
	if (wParam == DEFECTMSG)
	{
		CString* logText = (CString*)lParam;

		m_listDefect.InsertString(m_listDefect.GetCount(), *logText);
		m_listDefect.SetCurSel(m_listDefect.GetCount() - 1);
		UpdateData(FALSE);
		delete logText;
		
		return TRUE;
	}

	CString* logText = (CString*)lParam;

	m_listLog.InsertString(m_listLog.GetCount(), *logText);
	m_listLog.SetCurSel(m_listLog.GetCount() - 1);
	UpdateData(FALSE);
	delete logText;
	return TRUE;
}

LPARAM CMFCMachineVisionDlg::PrintRes(WPARAM wParam, LPARAM lParam)
{
	CString* resText = (CString*)lParam;

	if (*resText != "")
	{
		CString strCnt;
		strCnt.Format(_T("%d"), m_nModelCnt);

		CString strModelCnt;
		strModelCnt = "<Model 00";
		strModelCnt += strCnt;
		strModelCnt += "> ";
		strModelCnt.Append(*resText);
		strModelCnt += ": NG";
		m_static_Result.SetWindowText(strModelCnt);
	}

	m_nModelCnt++;

	return TRUE;
}

void CMFCMachineVisionDlg::OnBnClickedButtonStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_InspectionHandler.DisconnectCam();
	AfxGetMainWnd()->SendMessage(WM_CLOSE);
}


void CMFCMachineVisionDlg::OnBnClickedButtonStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str = _T("Inspection started.\n");
	m_listLog.InsertString(m_listLog.GetCount(), str);
	m_listLog.SetCurSel(m_listLog.GetCount() - 1);
	UpdateData(FALSE);

	m_startTime = clock();	
	m_nModelCnt = 1;
	m_InspectionHandler.StartInspection();
}

void CMFCMachineVisionDlg::OnBnClickedButtonClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_listLog.ResetContent();
	m_listDefect.ResetContent();
}

LPARAM CMFCMachineVisionDlg::TimeCheck(WPARAM wParam, LPARAM lParam)
{
	double runTime;
	m_endTime = clock();
	runTime = (double)(m_endTime - m_startTime);
	
	CString str;
	CString tmp;
	str = "Inspection Time : ";
	tmp.Format(_T("%f"), runTime / CLOCKS_PER_SEC);
	str += tmp;

	m_listLog.InsertString(m_listLog.GetCount(), str);
	m_listLog.SetCurSel(m_listLog.GetCount() - 1);

	return TRUE;
}

