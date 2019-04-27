#pragma once
#include<vector>
#include<string>

class ImageFileBuffer
{
	std::vector<unsigned char> _file_content;
public:
	ImageFileBuffer(const std::string& file_path_);
	std::vector<unsigned char> Get();
};