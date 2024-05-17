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
	CPatternData* m_Patterns;			// 패턴 정보 객체 포인터
	CAlgorithmData* m_Algorithms;		// 검사 알고리즘 정보 객체 포인터
	CCameraHandler* m_CameraHandler;	// 카메라 제어 객체 포인터

	int m_nTotalPtnCnt;					// 총 검사 패턴 개수
	int m_nTotalAlgCnt;					// 총 검사 알고리즘 개수

	int m_nPtnChkAlgCnt;				//(알고리즘 쓰레드) 패턴별 확인을 완료한 알고리즘 개수
	int m_nTotalFinAlgCnt;				//(알고리즘 쓰레드) 검사를 완료한 총 알고리즘 개수
	int m_nGrabCnt;						//현재까지 촬상한 패턴 개수

	bool m_bRunCamThread;				//카메라 쓰레드 종료 여부
	bool m_bRunAlgThread;				//알고리즘 쓰레드 종료 여부
	bool m_bFinInspection;				//검사 종료 여부

	mutex m_CamMutex;					//카메라 Queue Mutex
	mutex m_AlgMutex;					//알고리즘 Queue Mutex
	mutex m_PtnChkAlgCntMutex;			//패턴별 확인을 완료한 알고리즘 개수 Mutex
	mutex m_TotalFinAlgCntMutex;		//검사를 완료한 총 알고리즘 개수 Mutex

	queue<int> m_CamQueue;				//카메라 Queue
	queue<pair<string, int>> m_AlgQueue;			//알고리즘 Queue

	vector<thread> m_AlgThreadVec;		//알고리즘 쓰레드 벡터
	thread m_CamThread;					//카메라 쓰레드				

	condition_variable m_CamCv;			//카메라 쓰레드 조건변수
	condition_variable m_AlgCv;			//알고리즘 쓰레드 조건변수

	CString m_strResText;
	
public:
	CInspectionHandler();
	~CInspectionHandler();

	void Initialize(HWND hWnd);			// 클래스 초기화 및 시작하는 함수
	void StartInspection();				// 검사 시작하는 함수
	bool ReadConfig();					// Config 파일 읽는 함수

	void ProcessingAlgQueue();			//알고리즘 쓰레드에서 동작하는 함수
	void ProcessingCamQueue();			//카메라 쓰레드에서 동작하는 함수
	void PrintLog(CString* logText, int defect = 0) const;
	int GetModelCount() const;
	void DisconnectCam();
private:
	HWND m_hWnd;
	int m_nFinModelCnt;
};

