#include "ImageFileBuffer.h"
#include <iterator>
#include <fstream>

ImageFileBuffer::ImageFileBuffer(const std::string & file_path_)
{
	std::ifstream file(file_path_.c_str(), std::ios::binary);
	unsigned char one_byte;
	while ( file >> std::noskipws >> one_byte )
		_file_content.push_back(one_byte);
}

std::vector<unsigned char> ImageFileBuffer::Get()
{
	return _file_content;
}
