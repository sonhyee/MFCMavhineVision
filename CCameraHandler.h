#pragma once

#define UM_LOGMSG WM_USER + 3

class CCameraHandler
{
private:
	int m_nShutterSpeed;					// 카메라 셔터 스피드
	bool m_bCamOn;							// 카메라 연결 상태
	HWND m_hWnd;							// 다이얼로그 핸들러

public:
	CCameraHandler();
	void ConnectCam(HWND hWnd);						// 카메라 연결하는 함수
	void SetCamParams(int shutterSpeed);	// 카메라 설정하는 함수
	void Grab(const char* ptnName);	// 촬상하는 함수
	void DisconnectCam();					// 카메라 연결 해제하는 함수
	bool CheckCamOn() const;				// 카메라 연결 상태 확인하는 함수

	void PrintLog(CString* logText) const;		// 로그 출력하는 함수
};

