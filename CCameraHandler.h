#pragma once

#define UM_LOGMSG WM_USER + 3

class CCameraHandler
{
private:
	int m_nShutterSpeed;					// ī�޶� ���� ���ǵ�
	bool m_bCamOn;							// ī�޶� ���� ����
	HWND m_hWnd;							// ���̾�α� �ڵ鷯

public:
	CCameraHandler();
	void ConnectCam(HWND hWnd);						// ī�޶� �����ϴ� �Լ�
	void SetCamParams(int shutterSpeed);	// ī�޶� �����ϴ� �Լ�
	void Grab(const char* ptnName);	// �Ի��ϴ� �Լ�
	void DisconnectCam();					// ī�޶� ���� �����ϴ� �Լ�
	bool CheckCamOn() const;				// ī�޶� ���� ���� Ȯ���ϴ� �Լ�

	void PrintLog(CString* logText) const;		// �α� ����ϴ� �Լ�
};

