#pragma once

#include <cstdint>
#include <vector>

class CSampleModel
{
public:
	virtual ~CSampleModel() = default;

	virtual uint16_t getImageWidth() const = 0;
	virtual uint16_t getImageHeight() const = 0;
	virtual std::vector<uint8_t>& getImageVector() = 0;
	virtual const uint8_t* getImagePtr() const = 0;
	virtual uint16_t getImageSize() const = 0;
};
