#include"BitStream.h"


InputBitStream::InputBitStream(const std::vector<unsigned char>& buffer_)
	: _buffer(buffer_)
	, _index(0)
	, _bit_number(7)
	, _stream_end(false)
{
}

InputBitStream::InputBitStream(const std::string & buffer_)
	: _buffer(buffer_.begin(), buffer_.end())
	, _index(0)
	, _bit_number(7)
	, _stream_end(false)
{
}

InputBitStream & InputBitStream::operator>>(bit& value_)
{
	value_ = 0;
	if (_index < _buffer.size())
	{
		value_ = bool(_buffer[_index] & (1 << _bit_number));
		_bit_number--;
		if (_bit_number == -1)
		{
			_bit_number = 7;
			_index++;
		}
	}
	else
	{
		_stream_end = true;
	}
	return *this;
}
InputBitStream & InputBitStream::operator>>(byte& value_)
{
	value_ = 0;
	if (_index < _buffer.size())
	{
		value_ = _buffer[_index];
		_index++;
	}
	else
	{
		_stream_end = true;
	}
	return *this;
}


InputBitStream::operator bool() const
{
	return !_stream_end;
}

void InputBitStream::BitsBack(int number_of_bits_to_revert_)
{
	int chars_number = number_of_bits_to_revert_ / 8;
	number_of_bits_to_revert_ -= chars_number * 8;
	_bit_number -= number_of_bits_to_revert_;
	if (_bit_number < 0)
	{
		_bit_number += 8;
		_index--;
	}
	this->BytesBack(chars_number);
}

void InputBitStream::BytesBack(int number_of_chars_to_revert_)
{
	_index -= number_of_chars_to_revert_;
	_index = std::max(_index, 0);
}

unsigned int InputBitStream::Size() const
{
	return _buffer.size() - _index;
}
