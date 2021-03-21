#pragma once

#include "opencv2/opencv.hpp"

#include "CException.h"
#include "CSampleModel.h"

class CWebcamCvSample : public CSampleModel
{
public:
	CWebcamCvSample() throw(CException);
	CWebcamCvSample(uint8_t index) throw(CException);
	~CWebcamCvSample();

	void readFromWebcam(bool display);

	uint16_t getImageWidth() const override;
	uint16_t getImageHeight() const override;
	
	void setImageWidth(uint16_t height);
	void setImageHeight(uint16_t width);

	std::vector<uint8_t>& getImageVector() override;
	const uint8_t* getImagePtr() const override;
	uint16_t getImageSize() const override;
private:
	cv::VideoCapture m_CameraCapture;
	std::vector<uint8_t> m_CompressedImage;
	const std::vector<int> m_CompressParam;
};
