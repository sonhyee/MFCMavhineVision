#pragma once
#include <opencv2\opencv.hpp>
#define MAX_PTN_NAME_SIZE 20

class CPatternData
{
private:
	char m_szPtnName[MAX_PTN_NAME_SIZE];	// 패턴 이름
	int m_nShutterSpeed;					// 패턴 촬영 시 필요한 셔터 스피드
	cv::Mat m_matImage;
public:
	CPatternData();
	~CPatternData();
	void SetPtnName(const char* ptnName);	// 패턴 이름을 설정하는 함수
	const char* GetPtnName() const;			// 패턴 이름을 반환하는 함수
	void SetShutterSpeed(int shutterSpeed);	// 패턴 촬영 시 필요한 셔터 스피드를 설정하는 함수
	int GetShutterSpeed() const;			// 패턴 촬영 시 필요한 셔터 스피드를 반환하는 함수
	bool SetPtnImg(const char* ptnName);
	cv::Mat GetPtnImg();
};

