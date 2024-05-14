#pragma once
#include <opencv2\opencv.hpp>

#define ALG_MAX_NAME_SIZE 20
#define ALG_MAX_PTN_CNT	5
#define ALG_MAX_PTN_NAME_SIZE 20

class CAlgorithmData
{
private:
	char m_szAlgName[ALG_MAX_NAME_SIZE];	// �˰��� �̸�
	char m_szPtnName[ALG_MAX_PTN_CNT][ALG_MAX_PTN_NAME_SIZE];	// �˰����� ���Ǵ� ���� �̸�
	bool m_bAlgResult;						// �˰��� �˻� ���
	int m_nPtnCount;						// �˰����� ���Ǵ� ���� ����
	cv::Mat m_matImage;

public:
	CAlgorithmData();
	~CAlgorithmData();
	void SetAlgName(const char* algName);		// �˰��� �̸��� �����ϴ� �Լ�
	void SetAlgPtnInfo(const char* ptnName);	// �˰����� ���Ǵ� ���� ������ �����ϴ� �Լ�
	bool CheckAlgPtn(const char* ptnName) const;// �˰����� ���Ǵ� �������� �˻��ϴ� �Լ�
	const char* GetAlgName() const;				// �˰��� �̸��� ��ȯ�ϴ� �Լ�
	bool GetAlgResult() const;					// �˰��� �˻� ����� ��ȯ�ϴ� �Լ�
	void RunAlgorithm(HWND hWnd, const cv::Mat& img);			// �˰��� ����

	bool ImageScratch();
	bool ImageBlob(cv::Point pt);
	bool ImageRed(cv::Point pt1, cv::Point pt2[]);
	bool ImageGreen(cv::Point pt1, cv::Point pt2[]);
	bool ImageBlue(cv::Point pt1, cv::Point pt2[]);
};

