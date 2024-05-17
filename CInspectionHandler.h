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
	CPatternData* m_Patterns;			// ���� ���� ��ü ������
	CAlgorithmData* m_Algorithms;		// �˻� �˰��� ���� ��ü ������
	CCameraHandler* m_CameraHandler;	// ī�޶� ���� ��ü ������

	int m_nTotalPtnCnt;					// �� �˻� ���� ����
	int m_nTotalAlgCnt;					// �� �˻� �˰��� ����
public:
	CInspectionHandler();
	~CInspectionHandler();

	void Initialize(HWND hWnd);			// Ŭ���� �ʱ�ȭ �� �����ϴ� �Լ�
	void StartInspection();				// �˻� �����ϴ� �Լ�
	bool ReadConfig();					// Config ���� �д� �Լ�

	void PrintLog(CString* logText, int defect = 0) const;
	int GetModelCount() const;
private:
	HWND m_hWnd;
	int m_nTotalModelCnt;
	int m_nFinModelCnt;
	int m_nModelcnt;					// ���� �Կ��� �� ��ȣ
};

