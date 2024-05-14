#include "pch.h"
#include "CCameraHandler.h"
#include <cstdio>
#include <windows.h>
#include <sstream>
#include "MFCMachineVisionDlg.h"

CCameraHandler::CCameraHandler() : m_nShutterSpeed(0), m_bCamOn(false), m_hWnd(NULL) {}

void CCameraHandler::PrintLog(CString* logText) const
{
	SendMessage(m_hWnd, UM_LOGMSG, NULL, (LPARAM)logText);
}

void CCameraHandler::ConnectCam(HWND hWnd)
{
	m_hWnd = hWnd;
	stringstream ss;
	ss.str("");
	ss << "CCameraHandler::ConnectCam called.\n";
	CString* logText = new CString(ss.str().c_str());
	PrintLog(logText);

	Sleep(1000);
	m_bCamOn = true;

	ss.str("");
	ss << "CCameraHandler::ConnectCam finished.\n";
	logText = new CString(ss.str().c_str());
	PrintLog(logText);
}

void CCameraHandler::SetCamParams(int shutterSpeed)
{
	stringstream ss;
	ss.str("");
	ss << "CCameraHandler::SetCamParams called(" << shutterSpeed << "s).\n";
	CString* logText = new CString(ss.str().c_str());
	PrintLog(logText);

	Sleep(100);
	m_nShutterSpeed = shutterSpeed;
	
	ss.str("");
	ss << "CCameraHandler::SetCamParams finished(" << shutterSpeed << "s).\n";
	logText = new CString(ss.str().c_str());
	PrintLog(logText);
}

void CCameraHandler::Grab( const char* ptnName)
{
	stringstream ss;
	ss.str("");
	ss << "CCameraHandler::Grab called(" << ptnName << ").\n";
	CString* logText = new CString(ss.str().c_str());
	PrintLog(logText);
	Sleep(m_nShutterSpeed * 1000);

	ss.str("");
	ss << "CCameraHandler::Grab finished(" << ptnName << ").\n";
	logText = new CString(ss.str().c_str());
	PrintLog(logText);
}

void CCameraHandler::DisconnectCam()
{
	stringstream ss;
	ss.str("");
	ss << "CCameraHandler::DisconnectCam called.\n";
	CString* logText = new CString(ss.str().c_str());
	PrintLog(logText);

	Sleep(1000);
	m_bCamOn = false;

	ss.str("");
	ss << "CCameraHandler::DisconnectCam finished.\n";
	logText = new CString(ss.str().c_str());
	PrintLog(logText);
}

bool CCameraHandler::CheckCamOn() const
{
	stringstream ss;
	ss.str("");
	ss << "CCameraHandler::CheckCamOn called.\n";
	CString* logText = new CString(ss.str().c_str());
	PrintLog(logText);
	return m_bCamOn;
}
