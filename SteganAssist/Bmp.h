#pragma once
#include"BitStream.h"
#include"ImageFileBuffer.h"
#include"Image.h"

// [ICC.1:2010]
class Bmp : public Image
{
	InputBitStream _image_content;
	enum class types
	{
		dateTimeNumber,
		float32Number,
		positionNumber,
		response16Number,
		s15Fixed16Number,
		u16Fixed16Number,
		u1Fixed15Number,
		u8Fixed8Number,
		uInt8Number,
		uInt16Number,
		uInt32Number,
		uInt64Number,
		XYZNumber,

	};



public:
	Bmp(const std::string& file_path_)
		: _image_content(ImageFileBuffer(file_path_).Get())
	{
		
	}
};