#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "CInspectionHandler.h"
#include <sstream>
#include <string>

using namespace std;

// 생성자
CInspectionHandler::CInspectionHandler()
	:m_Patterns(NULL), m_Algorithms(NULL), m_CameraHandler(NULL),
	m_nTotalPtnCnt(0), m_nTotalAlgCnt(0), m_nPtnChkAlgCnt(0),
	m_bRunCamThread(TRUE), m_bRunAlgThread(TRUE), m_nTotalFinAlgCnt(0), m_bFinInspection(FALSE), m_nGrabCnt(0),
	m_nTotalModelCnt(0), m_nFinModelCnt(0), m_hWnd(NULL), m_nModelcnt(0), m_strResText("")
{}

// 소멸자
CInspectionHandler::~CInspectionHandler()
{
	if (m_CameraHandler)
	{
		if (m_CameraHandler->CheckCamOn())
		{
			m_CamCv.notify_all();							//자고있는 카메라 쓰레드 모두 깨우기
			m_CamMutex.lock();
			while (!m_CamQueue.empty()) m_CamQueue.pop();	//종료를 위해 대기중인 카메라 동작 모두 비우기
			m_CamQueue.push(DISCONNECT);					//카메라가 연결되어 있으면 카메라 연결 종료
			m_CamMutex.unlock();
			Sleep(30);										//DISCONNECT를 큐에서 꺼낼 때 까지 대기
		}

		m_bRunCamThread = false;							//카메라 쓰레드 종료 요청
		m_CamThread.join();									//카메라 쓰레드 종료 대기
		delete[] m_CameraHandler;
	}

	m_AlgMutex.lock();
	while (!m_AlgQueue.empty()) m_AlgQueue.pop();			//종료를 위해 대기중인 알고리즘 모두 비우기
	m_AlgMutex.unlock();

	m_bRunAlgThread = false;								//알고리즘 쓰레드 종료 요청

	m_AlgCv.notify_all();									//자고있는 알고리즘 쓰레드 모두 깨우기

	for (int i_Alg = 0; i_Alg < m_nTotalAlgCnt; i_Alg++)
	{
		m_AlgThreadVec[i_Alg].join();							//알고리즘 쓰레드 종료 대기
	}

	if (m_Patterns)
		delete[] m_Patterns;
	if (m_Algorithms)
		delete[] m_Algorithms;

}

void CInspectionHandler::Initialize(HWND hWnd)
{
	m_hWnd = hWnd;

	if (false == ReadConfig())
	{
		stringstream ss;
		ss.str("");
		ss << "파일을 확인하세요.\n";
		CString* logText = new CString(ss.str().c_str());
		PrintLog(logText);
		return;
	}

	m_CameraHandler = new CCameraHandler;	// 카메라 핸들러 객체 생성
	m_CamThread = thread(&CInspectionHandler::ProcessingCamQueue, this);
	m_CamQueue.push(CONNECT);				//카메라 연결
}

void CInspectionHandler::StartInspection()
{
	m_nTotalFinAlgCnt = 0;			//검사를 완료한 총 알고리즘 개수 초기화
	m_nPtnChkAlgCnt = 0;			//패턴별 확인을 완료한 총 알고리즘 개수 초기화
	m_nGrabCnt = 0;					//현재까지 촬상한 패턴 개수 초기화

	m_CamMutex.lock();
	for (int i_Ptn = 0; i_Ptn < m_nTotalPtnCnt; i_Ptn++)
	{
		m_CamQueue.push(SETCAMPARAMS);	//파라미터 세팅
		m_CamQueue.push(GRAB);			//촬상
		m_CamCv.notify_one();			//자고있는 카메라 쓰레드 깨우기
	}
	m_CamMutex.unlock();
}

bool CInspectionHandler::ReadConfig()
{
	char secName[1024];
	char buf[1024];

	// Pattern Load
	m_nTotalPtnCnt = GetPrivateProfileIntA("general", "total pattern count", 0, ".\\config.ini");

	if (0 == m_nTotalPtnCnt)
		return false;

	m_Patterns = new CPatternData[m_nTotalPtnCnt];

	for (int i_Ptn = 0; i_Ptn < m_nTotalPtnCnt; i_Ptn++)
	{
		sprintf_s(secName, "Pattern%02d", i_Ptn);

		GetPrivateProfileStringA(secName, "pattern name", "", buf, sizeof(buf), ".\\config.ini");	// 패턴 이름 읽기

		if (0 == strcmp(buf, ""))
			return false;

		m_Patterns[i_Ptn].SetPtnName(buf);	// 패턴 이름 설정

		m_Patterns[i_Ptn].SetShutterSpeed(GetPrivateProfileIntA(secName, "shutter speed", 0, ".\\config.ini")); // 셔터 스피드 읽기 및 설정
	}


	// Algorithm Load
	m_nTotalAlgCnt = GetPrivateProfileIntA("general", "total algorithm count", 0, ".\\config.ini");

	if (0 == m_nTotalAlgCnt)
		return false;

	for (int i_Alg = 0; i_Alg < m_nTotalAlgCnt; i_Alg++)
	{
		m_AlgThreadVec.push_back(thread(&CInspectionHandler::ProcessingAlgQueue, this)); //알고리즘 개수만큼 쓰레드 생성
	}

	m_Algorithms = new CAlgorithmData[m_nTotalAlgCnt];

	for (int i_Alg = 0; i_Alg < m_nTotalAlgCnt; i_Alg++)
	{
		sprintf_s(secName, "Algorithm%02d", i_Alg);

		GetPrivateProfileStringA(secName, "algorithm name", "", buf, sizeof(buf), ".\\config.ini");	// 알고리즘 이름 읽기

		if (0 == strcmp(buf, ""))
			return false;

		m_Algorithms[i_Alg].SetAlgName(buf);	// 알고리즘 이름 설정

		GetPrivateProfileStringA(secName, "pattern", "", buf, sizeof(buf), ".\\config.ini");	// 알고리즘이 사용되는 패턴 읽기

		if (0 == strcmp(buf, ""))
			return false;

		m_Algorithms[i_Alg].SetAlgPtnInfo(buf);	// 알고리즘이 사용되는 패턴 설정
	}

	//Model Load
	m_nTotalModelCnt = GetPrivateProfileIntA("general", "total model count", 0, ".\\config.ini");

	return true;
}

void CInspectionHandler::ProcessingCamQueue()
{
	while (m_bRunCamThread)
	{
		unique_lock<mutex>Cam_lock(m_CamMutex);
		m_CamCv.wait(Cam_lock, [this, &Cam_lock] { return !m_CamQueue.empty() || !m_bRunCamThread; });

		if (!m_bRunCamThread)	//카메라 쓰레드 종료 요청시 종료
		{
			Cam_lock.unlock();
			return;
		}

		int option = m_CamQueue.front();
		m_CamQueue.pop();		//할 일이 있으면 꺼내서 저장한 뒤 Queue로 부터 삭제
		Cam_lock.unlock();

		switch (option)
		{
		case CONNECT:
			m_CameraHandler->ConnectCam(m_hWnd);												// 카메라 연결	
			break;
		case SETCAMPARAMS:
			m_CameraHandler->SetCamParams(m_Patterns[m_nGrabCnt].GetShutterSpeed());	//파라미터 설정
			break;
		case GRAB:
		{
			m_CameraHandler->Grab(m_Patterns[m_nGrabCnt].GetPtnName());													//촬상

			string tmp = m_Patterns[m_nGrabCnt].GetPtnName();
			m_nModelcnt = GetModelCount();
			tmp.append("00");
			tmp += to_string(m_nModelcnt);
			tmp += ".jpg";
			const char* imgName = tmp.c_str();
			m_Patterns[m_nGrabCnt].SetPtnImg(imgName);

			m_AlgMutex.lock();
			m_AlgQueue.push({ m_Patterns[m_nGrabCnt].GetPtnName(), m_nGrabCnt });		//촬상한 패턴 이름 Queue에 저장
			m_AlgCv.notify_all();														//자고있는 알고리즘 쓰레드 모두 깨우기 -> 그래야 병렬 처리의 의미가 있음
			m_AlgMutex.unlock();

			uintptr_t patternPtr = (uintptr_t)&m_Patterns[m_nGrabCnt];
			SendMessage(m_hWnd, UM_PRINTIMG, (WPARAM)patternPtr, 0);

			m_nGrabCnt++;																//현재까지 촬상한 개수 증가
		}
			break;
		case DISCONNECT:
			if (m_CameraHandler->CheckCamOn())
				m_CameraHandler->DisconnectCam();										//카메라 연결확인 후 연결해제
			break;
		}
	}
}

void CInspectionHandler::ProcessingAlgQueue()
{
	while (m_bRunAlgThread)
	{
		unique_lock<mutex> Alg_lock(m_AlgMutex);

		m_AlgCv.wait(Alg_lock, [this, &Alg_lock] {return !m_AlgQueue.empty() || !m_bRunCamThread; });

		if (!m_bRunAlgThread)		//알고리즘 쓰레드 종료 요청시 종료
		{
			Alg_lock.unlock();
			return;
		}

		string ptnName = m_AlgQueue.front().first;
		int ptnNum = m_AlgQueue.front().second;
		int algIdx = m_nPtnChkAlgCnt;

		m_PtnChkAlgCntMutex.lock();
		m_nPtnChkAlgCnt++;			// (알고리즘 쓰레드) 패턴별 확인을 완료한 알고리즘 개수 증가

		if (m_nPtnChkAlgCnt == m_nTotalAlgCnt)
		{
			m_AlgQueue.pop();		//모든 알고리즘이 패턴 확인을 완료했으면 Queue에서 꺼내기
			m_nPtnChkAlgCnt = 0;
		}
		m_PtnChkAlgCntMutex.unlock();
		Alg_lock.unlock();

	
		if (m_Algorithms[algIdx].CheckAlgPtn(ptnName.c_str()))   // 검사하는 알고리즘이라면
		{
			m_Algorithms[algIdx].RunAlgorithm(m_hWnd, m_Patterns[ptnNum].GetPtnImg());	// 알고리즘 수행

			if (m_Algorithms[algIdx].GetAlgResult())
			{
				stringstream ss;
				ss.str("");
				ss << "Ptn[" << ptnName.c_str() << "00" << m_nModelcnt << "] : Alg[" << m_Algorithms[algIdx].GetAlgName() << "] OK.\n";
				CString* logText = new CString(ss.str().c_str());
				PrintLog(logText, DEFECTMSG);
			}
			else
			{
				stringstream ss;
				ss.str("");
				ss << "Ptn[" << ptnName.c_str() << "00" << m_nModelcnt << "] : Alg[" << m_Algorithms[algIdx].GetAlgName() << "] NG.\n";
				CString* logText = new CString(ss.str().c_str());

				m_strResText += m_Algorithms[algIdx].GetAlgName();
				m_strResText += " ";
				
				PrintLog(logText, DEFECTMSG);
			}
		}

		m_TotalFinAlgCntMutex.lock();
		m_nTotalFinAlgCnt++;

		if (m_nTotalFinAlgCnt == m_nTotalPtnCnt * m_nTotalAlgCnt)		//모든 패턴에 대한 모든 알고리즘 검사가 끝났으면
		{
			stringstream ss;
			ss.str("");
			ss << "Inspection completed.\n";
			CString* logText = new CString(ss.str().c_str());
			PrintLog(logText);

			SendMessage(m_hWnd, UM_RESMSG, NULL, (LPARAM)&m_strResText);

			m_strResText = " ";
			m_nTotalFinAlgCnt = 0;										//(알고리즘 쓰레드) 검사를 완료한 총 알고리즘 개수 증가
			
			if (GetModelCount() != m_nTotalModelCnt)
			{
				m_nFinModelCnt++;
				StartInspection();
			}
			else
			{
				SendMessage(m_hWnd, UM_INSPEND, 0, 0);
				m_nFinModelCnt = 0;
			}
		}
		m_TotalFinAlgCntMutex.unlock();
	}
}

int CInspectionHandler::GetModelCount() const
{
	return m_nFinModelCnt + 1;
}

void CInspectionHandler::PrintLog(CString* logText, int defect) const
{
	if (defect)
	{
		SendMessage(m_hWnd, UM_LOGMSG, (WPARAM)DEFECTMSG, (LPARAM)logText);
		return;
	}

	SendMessage(m_hWnd, UM_LOGMSG, NULL, (LPARAM)logText);
}

void CInspectionHandler::DisconnectCam()
{
	m_CamMutex.lock();
	m_CamQueue.push(DISCONNECT);			
	m_CamMutex.unlock();
}