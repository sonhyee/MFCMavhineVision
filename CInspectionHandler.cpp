#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "CInspectionHandler.h"
#include <sstream>
#include <string>

using namespace std;

// ������
CInspectionHandler::CInspectionHandler()
	:m_Patterns(NULL), m_Algorithms(NULL), m_CameraHandler(NULL),
	m_nTotalPtnCnt(0), m_nTotalAlgCnt(0), m_nPtnChkAlgCnt(0),
	m_bRunCamThread(TRUE), m_bRunAlgThread(TRUE), m_nTotalFinAlgCnt(0), m_bFinInspection(FALSE), m_nGrabCnt(0),
	m_nTotalModelCnt(0), m_nFinModelCnt(0), m_hWnd(NULL), m_nModelcnt(0), m_strResText("")
{}

// �Ҹ���
CInspectionHandler::~CInspectionHandler()
{
	if (m_CameraHandler)
	{
		if (m_CameraHandler->CheckCamOn())
		{
			m_CamCv.notify_all();							//�ڰ��ִ� ī�޶� ������ ��� �����
			m_CamMutex.lock();
			while (!m_CamQueue.empty()) m_CamQueue.pop();	//���Ḧ ���� ������� ī�޶� ���� ��� ����
			m_CamQueue.push(DISCONNECT);					//ī�޶� ����Ǿ� ������ ī�޶� ���� ����
			m_CamMutex.unlock();
			Sleep(30);										//DISCONNECT�� ť���� ���� �� ���� ���
		}

		m_bRunCamThread = false;							//ī�޶� ������ ���� ��û
		m_CamThread.join();									//ī�޶� ������ ���� ���
		delete[] m_CameraHandler;
	}

	m_AlgMutex.lock();
	while (!m_AlgQueue.empty()) m_AlgQueue.pop();			//���Ḧ ���� ������� �˰��� ��� ����
	m_AlgMutex.unlock();

	m_bRunAlgThread = false;								//�˰��� ������ ���� ��û

	m_AlgCv.notify_all();									//�ڰ��ִ� �˰��� ������ ��� �����

	for (int i_Alg = 0; i_Alg < m_nTotalAlgCnt; i_Alg++)
	{
		m_AlgThreadVec[i_Alg].join();							//�˰��� ������ ���� ���
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
		ss << "������ Ȯ���ϼ���.\n";
		CString* logText = new CString(ss.str().c_str());
		PrintLog(logText);
		return;
	}

	m_CameraHandler = new CCameraHandler;	// ī�޶� �ڵ鷯 ��ü ����
	m_CamThread = thread(&CInspectionHandler::ProcessingCamQueue, this);
	m_CamQueue.push(CONNECT);				//ī�޶� ����
}

void CInspectionHandler::StartInspection()
{
	m_nTotalFinAlgCnt = 0;			//�˻縦 �Ϸ��� �� �˰��� ���� �ʱ�ȭ
	m_nPtnChkAlgCnt = 0;			//���Ϻ� Ȯ���� �Ϸ��� �� �˰��� ���� �ʱ�ȭ
	m_nGrabCnt = 0;					//������� �Ի��� ���� ���� �ʱ�ȭ

	m_CamMutex.lock();
	for (int i_Ptn = 0; i_Ptn < m_nTotalPtnCnt; i_Ptn++)
	{
		m_CamQueue.push(SETCAMPARAMS);	//�Ķ���� ����
		m_CamQueue.push(GRAB);			//�Ի�
		m_CamCv.notify_one();			//�ڰ��ִ� ī�޶� ������ �����
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

		GetPrivateProfileStringA(secName, "pattern name", "", buf, sizeof(buf), ".\\config.ini");	// ���� �̸� �б�

		if (0 == strcmp(buf, ""))
			return false;

		m_Patterns[i_Ptn].SetPtnName(buf);	// ���� �̸� ����

		m_Patterns[i_Ptn].SetShutterSpeed(GetPrivateProfileIntA(secName, "shutter speed", 0, ".\\config.ini")); // ���� ���ǵ� �б� �� ����
	}


	// Algorithm Load
	m_nTotalAlgCnt = GetPrivateProfileIntA("general", "total algorithm count", 0, ".\\config.ini");

	if (0 == m_nTotalAlgCnt)
		return false;

	for (int i_Alg = 0; i_Alg < m_nTotalAlgCnt; i_Alg++)
	{
		m_AlgThreadVec.push_back(thread(&CInspectionHandler::ProcessingAlgQueue, this)); //�˰��� ������ŭ ������ ����
	}

	m_Algorithms = new CAlgorithmData[m_nTotalAlgCnt];

	for (int i_Alg = 0; i_Alg < m_nTotalAlgCnt; i_Alg++)
	{
		sprintf_s(secName, "Algorithm%02d", i_Alg);

		GetPrivateProfileStringA(secName, "algorithm name", "", buf, sizeof(buf), ".\\config.ini");	// �˰��� �̸� �б�

		if (0 == strcmp(buf, ""))
			return false;

		m_Algorithms[i_Alg].SetAlgName(buf);	// �˰��� �̸� ����

		GetPrivateProfileStringA(secName, "pattern", "", buf, sizeof(buf), ".\\config.ini");	// �˰����� ���Ǵ� ���� �б�

		if (0 == strcmp(buf, ""))
			return false;

		m_Algorithms[i_Alg].SetAlgPtnInfo(buf);	// �˰����� ���Ǵ� ���� ����
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

		if (!m_bRunCamThread)	//ī�޶� ������ ���� ��û�� ����
		{
			Cam_lock.unlock();
			return;
		}

		int option = m_CamQueue.front();
		m_CamQueue.pop();		//�� ���� ������ ������ ������ �� Queue�� ���� ����
		Cam_lock.unlock();

		switch (option)
		{
		case CONNECT:
			m_CameraHandler->ConnectCam(m_hWnd);												// ī�޶� ����	
			break;
		case SETCAMPARAMS:
			m_CameraHandler->SetCamParams(m_Patterns[m_nGrabCnt].GetShutterSpeed());	//�Ķ���� ����
			break;
		case GRAB:
		{
			m_CameraHandler->Grab(m_Patterns[m_nGrabCnt].GetPtnName());													//�Ի�

			string tmp = m_Patterns[m_nGrabCnt].GetPtnName();
			m_nModelcnt = GetModelCount();
			tmp.append("00");
			tmp += to_string(m_nModelcnt);
			tmp += ".jpg";
			const char* imgName = tmp.c_str();
			m_Patterns[m_nGrabCnt].SetPtnImg(imgName);

			m_AlgMutex.lock();
			m_AlgQueue.push({ m_Patterns[m_nGrabCnt].GetPtnName(), m_nGrabCnt });		//�Ի��� ���� �̸� Queue�� ����
			m_AlgCv.notify_all();														//�ڰ��ִ� �˰��� ������ ��� ����� -> �׷��� ���� ó���� �ǹ̰� ����
			m_AlgMutex.unlock();

			uintptr_t patternPtr = (uintptr_t)&m_Patterns[m_nGrabCnt];
			SendMessage(m_hWnd, UM_PRINTIMG, (WPARAM)patternPtr, 0);

			m_nGrabCnt++;																//������� �Ի��� ���� ����
		}
			break;
		case DISCONNECT:
			if (m_CameraHandler->CheckCamOn())
				m_CameraHandler->DisconnectCam();										//ī�޶� ����Ȯ�� �� ��������
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

		if (!m_bRunAlgThread)		//�˰��� ������ ���� ��û�� ����
		{
			Alg_lock.unlock();
			return;
		}

		string ptnName = m_AlgQueue.front().first;
		int ptnNum = m_AlgQueue.front().second;
		int algIdx = m_nPtnChkAlgCnt;

		m_PtnChkAlgCntMutex.lock();
		m_nPtnChkAlgCnt++;			// (�˰��� ������) ���Ϻ� Ȯ���� �Ϸ��� �˰��� ���� ����

		if (m_nPtnChkAlgCnt == m_nTotalAlgCnt)
		{
			m_AlgQueue.pop();		//��� �˰����� ���� Ȯ���� �Ϸ������� Queue���� ������
			m_nPtnChkAlgCnt = 0;
		}
		m_PtnChkAlgCntMutex.unlock();
		Alg_lock.unlock();

	
		if (m_Algorithms[algIdx].CheckAlgPtn(ptnName.c_str()))   // �˻��ϴ� �˰����̶��
		{
			m_Algorithms[algIdx].RunAlgorithm(m_hWnd, m_Patterns[ptnNum].GetPtnImg());	// �˰��� ����

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

		if (m_nTotalFinAlgCnt == m_nTotalPtnCnt * m_nTotalAlgCnt)		//��� ���Ͽ� ���� ��� �˰��� �˻簡 ��������
		{
			stringstream ss;
			ss.str("");
			ss << "Inspection completed.\n";
			CString* logText = new CString(ss.str().c_str());
			PrintLog(logText);

			SendMessage(m_hWnd, UM_RESMSG, NULL, (LPARAM)&m_strResText);

			m_strResText = " ";
			m_nTotalFinAlgCnt = 0;										//(�˰��� ������) �˻縦 �Ϸ��� �� �˰��� ���� ����
			
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