#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "CInspectionHandler.h"
#include <sstream>
#include <string>

using namespace std;

// 생성자
CInspectionHandler::CInspectionHandler()
	:m_Patterns(NULL), m_Algorithms(NULL), m_CameraHandler(NULL),
	m_nTotalPtnCnt(0), m_nTotalAlgCnt(0),
	m_nTotalModelCnt(0), m_nFinModelCnt(0), 
	m_hWnd(NULL), m_nModelcnt(0)
{}

// 소멸자
CInspectionHandler::~CInspectionHandler()
{
	if (m_Patterns)
		delete[] m_Patterns;

	if (m_Algorithms)
		delete[] m_Algorithms;

	if (m_CameraHandler)
	{
		if (m_CameraHandler->CheckCamOn())
			m_CameraHandler->DisconnectCam();	// 카메라가 연결되어 있으면 카메라 연결 종료
		delete[] m_CameraHandler;
	}
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
	m_CameraHandler = new CCameraHandler;
	m_CameraHandler->ConnectCam(m_hWnd);
}

void CInspectionHandler::StartInspection()
{	
	//검사 완료 모델 개수 초기화
	m_nFinModelCnt = 0;

	//검사 해야하는 모델 개수만큼 반복
	while (m_nTotalModelCnt != m_nFinModelCnt)
	{
		//패턴 개수만큼 전체 과정 반복
		for (int i_Ptn = 0; i_Ptn < m_nTotalPtnCnt; i_Ptn++)
		{
			//파라미터 설정 후 이미지 그랩
			m_CameraHandler->SetCamParams(m_Patterns[i_Ptn].GetShutterSpeed());
			m_CameraHandler->Grab(m_Patterns[i_Ptn].GetPtnName());

			string tmp = m_Patterns[i_Ptn].GetPtnName();
			m_nModelcnt = GetModelCount();
			tmp.append("00");
			tmp += to_string(m_nModelcnt);
			tmp += ".jpg";
			const char* imgName = tmp.c_str();
			m_Patterns[i_Ptn].SetPtnImg(imgName);

			//Dialog에 이미지 프린팅
			uintptr_t patternPtr = (uintptr_t)&m_Patterns[i_Ptn];
			SendMessage(m_hWnd, UM_PRINTIMG, (WPARAM)patternPtr, 0);

			SendMessage(m_hWnd, UM_INSPEND, 0, 0);

			//전체 알고리즘 개수만큼 반복
			for (int i_Alg = 0; i_Alg < m_nTotalAlgCnt; i_Alg++)
			{
				//알고리즘이 검사하는 패턴이면
				if (m_Algorithms[i_Alg].CheckAlgPtn(m_Patterns[i_Ptn].GetPtnName()))
				{
					//검사 알고리즘 수행
					m_Algorithms[i_Alg].RunAlgorithm(m_hWnd, m_Patterns[i_Ptn].GetPtnImg());

					//OK
					if (m_Algorithms[i_Alg].GetAlgResult())
					{
						stringstream ss;
						ss.str("");
						ss << "Ptn[" << m_Patterns[i_Ptn].GetPtnName() << "00" << m_nModelcnt << "] : Alg[" 
							<< m_Algorithms[i_Alg].GetAlgName() << "] OK.\n";
						CString* logText = new CString(ss.str().c_str());
						PrintLog(logText, 1);
					}
					//NG
					else
					{
						stringstream ss;
						ss.str("");
						ss << "Ptn[" << m_Patterns[i_Ptn].GetPtnName() << "00" << m_nModelcnt << "] : Alg[" 
							<< m_Algorithms[i_Alg].GetAlgName() << "] NG.\n";
						CString* logText = new CString(ss.str().c_str());
						PrintLog(logText, 1);
					}
				}
			}
			SendMessage(m_hWnd, UM_INSPEND, 0, 0);
		}
		//검사 완료 모델 개수 +1
		m_nFinModelCnt++;
		SendMessage(m_hWnd, UM_INSPEND, 0, 0);
	}
	//모든 모델 전체 검사 과정 완료
	SendMessage(m_hWnd, UM_INSPEND, 0, 0);
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
