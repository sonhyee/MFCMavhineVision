#pragma once
#include "CPatternData.h"
#include "CAlgorithmData.h"
#include "CCameraHandler.h"
#include <string>

#define DEFECTMSG 1
#define UM_PRINTIMG	WM_USER + 1
#define UM_INSPEND	WM_USER + 2
#define UM_LOGMSG	WM_USER + 3

using namespace std;

class CInspectionHandler
{
private:
	CPatternData* m_Patterns;			// 패턴 정보 객체 포인터
	CAlgorithmData* m_Algorithms;		// 검사 알고리즘 정보 객체 포인터
	CCameraHandler* m_CameraHandler;	// 카메라 제어 객체 포인터

	int m_nTotalPtnCnt;					// 총 검사 패턴 개수
	int m_nTotalAlgCnt;					// 총 검사 알고리즘 개수
public:
	CInspectionHandler();
	~CInspectionHandler();

	void Initialize(HWND hWnd);			// 클래스 초기화 및 시작하는 함수
	void StartInspection();				// 검사 시작하는 함수
	bool ReadConfig();					// Config 파일 읽는 함수

	void PrintLog(CString* logText, int defect = 0) const;
	int GetModelCount() const;
private:
	HWND m_hWnd;
	int m_nTotalModelCnt;
	int m_nFinModelCnt;
	int m_nModelcnt;					// 현재 촬영한 모델 번호
};

