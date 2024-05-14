#pragma once
#include <opencv2\opencv.hpp>
#define MAX_PTN_NAME_SIZE 20

class CPatternData
{
private:
	char m_szPtnName[MAX_PTN_NAME_SIZE];	// ���� �̸�
	int m_nShutterSpeed;					// ���� �Կ� �� �ʿ��� ���� ���ǵ�
	cv::Mat m_matImage;
public:
	CPatternData();
	~CPatternData();
	void SetPtnName(const char* ptnName);	// ���� �̸��� �����ϴ� �Լ�
	const char* GetPtnName() const;			// ���� �̸��� ��ȯ�ϴ� �Լ�
	void SetShutterSpeed(int shutterSpeed);	// ���� �Կ� �� �ʿ��� ���� ���ǵ带 �����ϴ� �Լ�
	int GetShutterSpeed() const;			// ���� �Կ� �� �ʿ��� ���� ���ǵ带 ��ȯ�ϴ� �Լ�
	bool SetPtnImg(const char* ptnName);
	cv::Mat GetPtnImg();
};

