#pragma once
#include <opencv2\opencv.hpp>
#include "CPatternData.h"
#include "CAlgorithmData.h"
#include "CCameraHandler.h"
#include <thread>
#include <queue>
#include <string>
#include <mutex>
#include <vector>
#include <condition_variable>

#define DEFECTMSG 1
#define UM_PRINTIMG	WM_USER + 1
#define UM_INSPEND	WM_USER + 2
#define UM_LOGMSG	WM_USER + 3
#define UM_RESMSG	WM_USER + 4

using namespace std;

enum AlgResult { OK = 1, NG = 2};
enum UserMenu { INSP_START = 1, INSP_STOP };
enum options { CONNECT = 1, SETCAMPARAMS, GRAB, DISCONNECT };

class CInspectionHandler
{
private:
	CPatternData* m_Patterns;			// ���� ���� ��ü ������
	CAlgorithmData* m_Algorithms;		// �˻� �˰��� ���� ��ü ������
	CCameraHandler* m_CameraHandler;	// ī�޶� ���� ��ü ������

	int m_nTotalPtnCnt;					// �� �˻� ���� ����
	int m_nTotalAlgCnt;					// �� �˻� �˰��� ����

	int m_nPtnChkAlgCnt;				//(�˰��� ������) ���Ϻ� Ȯ���� �Ϸ��� �˰��� ����
	int m_nTotalFinAlgCnt;				//(�˰��� ������) �˻縦 �Ϸ��� �� �˰��� ����
	int m_nGrabCnt;						//������� �Ի��� ���� ����

	bool m_bRunCamThread;				//ī�޶� ������ ���� ����
	bool m_bRunAlgThread;				//�˰��� ������ ���� ����
	bool m_bFinInspection;				//�˻� ���� ����

	mutex m_CamMutex;					//ī�޶� Queue Mutex
	mutex m_AlgMutex;					//�˰��� Queue Mutex
	mutex m_PtnChkAlgCntMutex;			//���Ϻ� Ȯ���� �Ϸ��� �˰��� ���� Mutex
	mutex m_TotalFinAlgCntMutex;		//�˻縦 �Ϸ��� �� �˰��� ���� Mutex

	queue<int> m_CamQueue;				//ī�޶� Queue
	queue<pair<string, int>> m_AlgQueue;			//�˰��� Queue

	vector<thread> m_AlgThreadVec;		//�˰��� ������ ����
	thread m_CamThread;					//ī�޶� ������				

	condition_variable m_CamCv;			//ī�޶� ������ ���Ǻ���
	condition_variable m_AlgCv;			//�˰��� ������ ���Ǻ���

	CString m_strResText;
	
public:
	CInspectionHandler();
	~CInspectionHandler();

	void Initialize(HWND hWnd);			// Ŭ���� �ʱ�ȭ �� �����ϴ� �Լ�
	void StartInspection();				// �˻� �����ϴ� �Լ�
	bool ReadConfig();					// Config ���� �д� �Լ�

	void ProcessingAlgQueue();			//�˰��� �����忡�� �����ϴ� �Լ�
	void ProcessingCamQueue();			//ī�޶� �����忡�� �����ϴ� �Լ�
	void PrintLog(CString* logText, int defect = 0) const;
	int GetModelCount() const;
	void DisconnectCam();
private:
	HWND m_hWnd;
	int m_nFinModelCnt;
};

