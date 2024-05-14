#pragma once
#include <opencv2\opencv.hpp>

#define ALG_MAX_NAME_SIZE 20
#define ALG_MAX_PTN_CNT	5
#define ALG_MAX_PTN_NAME_SIZE 20

class CAlgorithmData
{
private:
	char m_szAlgName[ALG_MAX_NAME_SIZE];	// 알고리즘 이름
	char m_szPtnName[ALG_MAX_PTN_CNT][ALG_MAX_PTN_NAME_SIZE];	// 알고리즘이 사용되는 패턴 이름
	bool m_bAlgResult;						// 알고리즘 검사 결과
	int m_nPtnCount;						// 알고리즘이 사용되는 패턴 개수
	cv::Mat m_matImage;

public:
	CAlgorithmData();
	~CAlgorithmData();
	void SetAlgName(const char* algName);		// 알고리즘 이름을 설정하는 함수
	void SetAlgPtnInfo(const char* ptnName);	// 알고리즘이 사용되는 패턴 정보를 설정하는 함수
	bool CheckAlgPtn(const char* ptnName) const;// 알고리즘이 사용되는 패턴인지 검사하는 함수
	const char* GetAlgName() const;				// 알고리즘 이름을 반환하는 함수
	bool GetAlgResult() const;					// 알고리즘 검사 결과를 반환하는 함수
	void RunAlgorithm(HWND hWnd, const cv::Mat& img);			// 알고리즘 수행

	bool ImageScratch();
	bool ImageBlob(cv::Point pt);
	bool ImageRed(cv::Point pt1, cv::Point pt2[]);
	bool ImageGreen(cv::Point pt1, cv::Point pt2[]);
	bool ImageBlue(cv::Point pt1, cv::Point pt2[]);
};

