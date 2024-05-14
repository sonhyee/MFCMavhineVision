#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "CAlgorithmData.h"
#include <string>
#include <cstdlib>
#include <windows.h>

using namespace std;
using namespace cv;

CAlgorithmData::CAlgorithmData()
	: m_bAlgResult(false), m_nPtnCount(0)
{
	// 배열 멤버변수 초기화
	memset(m_szAlgName, 0, sizeof(char) * ALG_MAX_NAME_SIZE);

	for (int i_ptnName = 0; i_ptnName < ALG_MAX_PTN_CNT; i_ptnName++)
	{
		memset(m_szPtnName[i_ptnName], 0, sizeof(char) * ALG_MAX_PTN_NAME_SIZE);
	}
}

CAlgorithmData::~CAlgorithmData()
{
}

void CAlgorithmData::SetAlgName(const char* algName)
{
	strcpy_s(m_szAlgName, algName);
}

const char* CAlgorithmData::GetAlgName() const
{
	return m_szAlgName;
}

bool CAlgorithmData::GetAlgResult() const
{
	return m_bAlgResult;
}

void CAlgorithmData::SetAlgPtnInfo(const char* ptnName)
{
	string str = ptnName;
	string delim = ",";

	size_t pos = 0;
	while ((pos = str.find(delim)) != string::npos)
	{
		strcpy_s(m_szPtnName[m_nPtnCount], str.substr(0, pos).c_str());
		m_nPtnCount++;
		str.erase(0, pos + delim.length());
	}
	// 마지막 패턴까지 저장
	strcpy_s(m_szPtnName[m_nPtnCount], str.c_str());
	m_nPtnCount++;
}

bool CAlgorithmData::CheckAlgPtn(const char* ptnName) const
{
	for (int i_Ptn = 0; i_Ptn < m_nPtnCount; i_Ptn++)
	{
		if (0 == strcmp(m_szPtnName[i_Ptn], ptnName))
			return true;
	}

	return false;
}

void CAlgorithmData::RunAlgorithm(HWND hWnd, const cv::Mat& img)
{
	m_matImage = img.clone();

	Sleep(1000);

	Point camerahole(398, 277);

	Point dots[] = {
		Point(250, 268),
		Point(235, 283),
		Point(235, 921),
		Point(247, 932),
		Point(551, 932),
		Point(565, 915),
		Point(565, 285),
		Point(550, 268)
	};

	if (!strcmp(m_szAlgName, "scratch"))
	{
		m_bAlgResult = ImageScratch();
	}

	else if (!strcmp(m_szAlgName, "stain"))
	{
		m_bAlgResult = ImageBlob(camerahole);
	}

	else if (!strcmp(m_szAlgName, "red defect"))
	{
		m_bAlgResult = ImageRed(camerahole, dots);
	}

	else if (!strcmp(m_szAlgName, "green defect"))
	{
		m_bAlgResult = ImageGreen(camerahole, dots);
	}

	else if (!strcmp(m_szAlgName, "blue defect"))
	{
		m_bAlgResult = ImageBlue(camerahole, dots);
	}
}


bool CAlgorithmData::ImageScratch()
{
	cv::Mat imgCanny;
	cv::Canny(m_matImage, imgCanny, 150, 255);
	vector<cv::Vec4i>lines;
	cv::HoughLinesP(imgCanny, lines, 1, CV_PI / 180, 40, 50, 5);

	for (size_t i = 0; i < lines.size(); i++)
	{
		for (size_t j = i + 1; j < lines.size(); j++)
		{
			Vec4i l1 = lines[i];
			Vec4i l2 = lines[j];

			Point p1((l1[0] + l1[2]) / 2, (l1[1] + l1[3]) / 2);
			Point p2((l2[0] + l2[2]) / 2, (l2[1] + l2[3]) / 2);

			double distance = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
			if (distance < 30)
			{
				return false;
			}
		}
	}

	return true;
}
bool CAlgorithmData::ImageBlob(cv::Point pt)
{
	int defect_cnt = 0;

	cv::Mat imgCanny;
	cv::Canny(m_matImage, imgCanny, 150, 255);
	cv::Ptr<cv::SimpleBlobDetector> detector = SimpleBlobDetector::create();
	vector<KeyPoint>keypoints;
	detector->detect(imgCanny, keypoints);

	//카메라홀 제외
	for (size_t i = 0; i < keypoints.size(); i++)
	{
		double distance = sqrt(pow(keypoints[i].pt.x - pt.x, 2) + pow(keypoints[i].pt.y - pt.y, 2));
		if (distance < 5)
		{
			keypoints.erase(keypoints.begin() + i);
		}
	}

	//표시
	for (size_t i = 0; i < keypoints.size(); i++)
	{
		return false;
	}
	
	return true;
}

bool CAlgorithmData::ImageRed(cv::Point pt_cam, cv::Point pt_dots[])
{
	// 팔각형 ROI 정의
	Mat mask = Mat::zeros(m_matImage.size(), CV_8UC1);
	fillConvexPoly(mask, pt_dots, 8, Scalar(255));

	// 카메라 홀 영역 제외 (반지름 5)
	circle(mask, pt_cam, 10, Scalar(0), -1);

	// ROI 내 R 평균 계산
	Scalar mean, stddev;
	Mat roi;
	m_matImage.copyTo(roi, mask);

	meanStdDev(roi, mean, stddev);
	double R_mean = mean[0];

	// ROI 내 R 평균보다 작은 픽셀 검사
	for (int i = 0; i < m_matImage.rows; i++)
	{
		for (int j = 0; j < m_matImage.cols; j++)
		{
			if (mask.at<uchar>(i, j) == 255 && m_matImage.at<Vec3b>(i, j)[2] < R_mean)
			{
				return false;
			}
		}
	}

	return true;
}

bool CAlgorithmData::ImageGreen(cv::Point pt_cam, cv::Point pt_dots[])
{
	// 팔각형 ROI 정의
	Mat mask = Mat::zeros(m_matImage.size(), CV_8UC1);
	fillConvexPoly(mask, pt_dots, 8, Scalar(255));

	// 카메라 홀 영역 제외 (반지름 5)
	circle(mask, pt_cam, 10, Scalar(0), -1);

	// ROI 내 R 평균 계산
	Scalar mean, stddev;
	Mat roi;
	m_matImage.copyTo(roi, mask);
	meanStdDev(roi, mean, stddev);
	double G_mean = mean[1];

	// ROI 내 G 평균보다 작은 픽셀 검사
	for (int i = 0; i < m_matImage.rows; i++) {
		for (int j = 0; j < m_matImage.cols; j++) {
			if (mask.at<uchar>(i, j) == 255 && m_matImage.at<Vec3b>(i, j)[1] < G_mean)
			{
				return false;
			}
		}
	}

	return true;
}

bool CAlgorithmData::ImageBlue(cv::Point pt_cam, cv::Point pt_dots[])
{
	// 팔각형 ROI 정의
	Mat mask = Mat::zeros(m_matImage.size(), CV_8UC1);
	fillConvexPoly(mask, pt_dots, 8, Scalar(255));

	// 카메라 홀 영역 제외 (반지름 5)
	circle(mask, pt_cam, 10, Scalar(0), -1);

	// ROI 내 R 평균 계산
	Scalar mean, stddev;
	Mat roi;
	m_matImage.copyTo(roi, mask);
	meanStdDev(roi, mean, stddev);
	double B_mean = mean[2];

	// ROI 내 B 평균보다 작은 픽셀 검사
	for (int i = 0; i < m_matImage.rows; i++) {
		for (int j = 0; j < m_matImage.cols; j++) {
			if (mask.at<uchar>(i, j) == 255 && m_matImage.at<Vec3b>(i, j)[0] < B_mean)
			{
				return false;
			}
		}
	}

	return true;
}
