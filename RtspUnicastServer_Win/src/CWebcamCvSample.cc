#include "CWebcamCvSample.h"

CWebcamCvSample::CWebcamCvSample() throw(CException) :
	m_CompressParam{ cv::IMWRITE_JPEG_QUALITY, 50 }
{
	if (!m_CameraCapture.open(0, cv::CAP_DSHOW))
	{
		throw CException("Camera Capture open failed");
	}
}

CWebcamCvSample::CWebcamCvSample(uint8_t index) throw(CException) :
	m_CompressParam{ cv::IMWRITE_JPEG_QUALITY, 50 }
{
	if (!m_CameraCapture.open(index, cv::CAP_DSHOW))
	{
		throw CException("Camera Capture open failed");
	}
}

CWebcamCvSample::~CWebcamCvSample()
{
	m_CameraCapture.release();
	cv::destroyAllWindows();
}

void CWebcamCvSample::readFromWebcam(bool display)
{
	cv::Mat frame;
	m_CameraCapture >> frame;
	while (!frame.empty())
	{
		if (!cv::imencode(".jpg", frame, m_CompressedImage, m_CompressParam)) /* encode/compress frame */
		{
			break;
		}
		else if (cv::waitKey(1) == 27)
		{
			break;
		}
		if (display)
		{
			cv::imshow("camera stream", frame);
		}
		/* read frame from camera */
		m_CameraCapture >> frame;
	}
}

uint16_t CWebcamCvSample::getImageWidth() const
{
	return (uint16_t)m_CameraCapture.get(cv::CAP_PROP_FRAME_WIDTH);
}

uint16_t CWebcamCvSample::getImageHeight() const
{
	return (uint16_t)m_CameraCapture.get(cv::CAP_PROP_FRAME_HEIGHT);
}

void CWebcamCvSample::setImageWidth(uint16_t width)
{
	m_CameraCapture.set(cv::CAP_PROP_FRAME_WIDTH, width);
}

void CWebcamCvSample::setImageHeight(uint16_t height)
{
	m_CameraCapture.set(cv::CAP_PROP_FRAME_HEIGHT, height);
}

std::vector<uint8_t>& CWebcamCvSample::getImageVector()
{
	return m_CompressedImage;
}

const uint8_t* CWebcamCvSample::getImagePtr() const
{
	return m_CompressedImage.data();
}

uint16_t CWebcamCvSample::getImageSize() const
{
	return (uint16_t)m_CompressedImage.size();
}
