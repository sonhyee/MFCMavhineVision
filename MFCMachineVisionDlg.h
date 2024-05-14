
// MFCMachineVisionDlg.h: 헤더 파일
//

#pragma once
#include "CInspectionHandler.h"
#include "resource.h"
#include <opencv2\opencv.hpp>

#define DEFECTMSG 1

// CMFCMachineVisionDlg 대화 상자
class CMFCMachineVisionDlg : public CDialogEx
{
// 생성입니다.
public:
	CMFCMachineVisionDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCMACHINEVISION_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonClear();
	afx_msg LPARAM PrintImage(WPARAM wParam, LPARAM lParam);	// Grab 후 이미지 로드 함수
	afx_msg LPARAM PrintLog(WPARAM wParam, LPARAM lParam);		// 로그 출력 함수
	afx_msg LPARAM TimeCheck(WPARAM wParam, LPARAM lParam);	
	afx_msg LPARAM PrintRes(WPARAM wParam, LPARAM lParam);		// 검사 결과 출력 함수
	DECLARE_MESSAGE_MAP()


public:
	CListBox m_listLog;
	CListBox m_listDefect;
	void CreateBitmapInfo(int w, int h, int bpp);
	void DrawImage(CClientDC& dc, CRect& rect, const cv::Mat& img) const;

	CStatic m_static_Result;
	
private:
	CString m_strPatternName;
	CInspectionHandler m_InspectionHandler;
	CPatternData* m_PtnHandler;
	BITMAPINFO* m_pBitmapInfo;
	cv::Mat m_matImage;
	clock_t m_startTime, m_endTime;
	int m_nModelCnt;
};
