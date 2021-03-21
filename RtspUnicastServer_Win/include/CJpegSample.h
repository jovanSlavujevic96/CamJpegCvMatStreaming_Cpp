#pragma once

#include "CSampleModel.h"

class CJpegSample : public CSampleModel
{
public:
	CJpegSample(uint8_t channelId);
	~CJpegSample();

	uint16_t getImageWidth() const override;
	uint16_t getImageHeight() const override;
	std::vector<uint8_t>& getImageVector() override;
	const uint8_t* getImagePtr() const override;
	uint16_t getImageSize() const override;

private:
	static std::vector<uint8_t> JpegScanDataCh1A, JpegScanDataCh1B;
	static std::vector<uint8_t> JpegScanDataCh2A, JpegScanDataCh2B;
	std::vector<uint8_t> sampleVector;
	bool bPeriodically;
	uint8_t uChannelId;
};
