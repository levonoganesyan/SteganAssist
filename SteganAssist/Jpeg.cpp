#include "Jpeg.h"

Jpeg::HuffmanTree::Node::Node()
	: _left(nullptr)
	, _right(nullptr)
	, _value(-1)
	, _code_end(false)
{

}
bool Jpeg::HuffmanTree::add_element(Node*& current_node, int lenght, int value)
{
	if (current_node != nullptr && current_node->_code_end == true)
		return false;
	if (current_node == nullptr)
	{
		current_node = new Node();
	}
	if (lenght == 0)
	{
		current_node->_value = value;
		current_node->_code_end = true;
		return true;
	}
	if (add_element(current_node->_left, lenght - 1, value))
	{
		return true;
	}
	if (add_element(current_node->_right, lenght - 1, value))
	{
		return true;
	}
	return false;
}
Jpeg::HuffmanTree::HuffmanTree()
	: _root(new Node())
	, _state(_root)
{
}
void Jpeg::HuffmanTree::AddElement(int lenght_, int value_)
{
	add_element(_root, lenght_, value_);
}
Jpeg::HuffmanTree::Node * Jpeg::HuffmanTree::GetRoot()
{
	return _root;
}

Jpeg::HuffmanTree::HuffmanTreeIterator::HuffmanTreeIterator(HuffmanTree * tree_)
	: _state(tree_->GetRoot())
	, _root(tree_->GetRoot())
{
}
Jpeg::HuffmanTree::HuffmanTreeIterator * Jpeg::HuffmanTree::HuffmanTreeIterator::Step(int way_)
{
	_state = (way_ == 0 ? _state->_left : _state->_right);
	if (_state == nullptr)
	{
		throw std::exception("Something goes wrong");
	}
	return this;
}
bool Jpeg::HuffmanTree::HuffmanTreeIterator::IsCodeEnd()
{
	return _state->_code_end;
}
byte Jpeg::HuffmanTree::HuffmanTreeIterator::GetValue()
{
	return _state->_value;
}
void Jpeg::HuffmanTree::HuffmanTreeIterator::Reset()
{
	_state = _root;
}


bool Jpeg::check_for_image_correctness(InputBitStream& image_content_)
{
	throw std::exception("Not implemented yet");
	//if (image_content_.size() < 4) // todo: understand, how much can be minimal size of picture
	//{
	//	throw std::exception("too small size of picture");
	//}
	//if (image_content_[0] * 0x100 + image_content_[1] != 0xffd8)
	//{
	//	throw std::exception("this is not jpeg file, start bytes are note matching");
	//}
	//if (image_content_[image_content_.size() - 2] * 0x100 + image_content_[image_content_.size() - 1] != 0xffd9)
	//{
	//	throw std::exception("this is not jpeg file, end bytes are note matching");
	//}
	//// todo: add another checks
	//return true;
}

void Jpeg::process_start_of_frame_baseline_DCT(InputBitStream& image_content_)
{
	byte size_1, size_2;
	image_content_ >> size_1 >> size_2;
	int size_of_frame = size_1 * 0x100 + size_2;

	byte precision;
	image_content_ >> precision;
	byte picture_height_1, picture_height_2;
	image_content_ >> picture_height_1 >> picture_height_2;
	int picture_height = picture_height_1 * 0x100 + picture_height_2;
	byte picture_width_1, picture_width_2;
	image_content_ >> picture_width_1 >> picture_width_2;
	int picture_width = picture_width_1 * 0x100 + picture_width_2;

	byte components_count;
	image_content_ >> components_count;

	_max_horizontal_thinning = 0;
	_max_vertical_thinning = 0;

	for (int i = 0; i < components_count; i++)
	{
		Frame frame;
		image_content_ >> frame._id;
		byte thinning;
		image_content_ >> thinning;
		frame._horizontal_thinning = thinning >> 4;
		frame._vertical_thinning = thinning & 0x0F;

		image_content_ >> frame._id_of_quantization_table;
		_frames.push_back(frame);

		_max_horizontal_thinning = std::max(frame._horizontal_thinning, _max_horizontal_thinning);
		_max_vertical_thinning = std::max(frame._vertical_thinning, _max_vertical_thinning);
	}
}

void Jpeg::process_start_of_frame_extended_sequential_DCT(InputBitStream& image_content_)
{
	throw std::exception("Not implemented yet");
}

void Jpeg::process_start_of_frame_progressive_DCT(InputBitStream& image_content_)
{
	throw std::exception("Not implemented yet");
}

void Jpeg::process_huffman_table(InputBitStream& image_content_)
{
	byte size_1, size_2;
	image_content_ >> size_1 >> size_2;
	int size_of_table = size_1 * 0x100 + size_2;

	byte temp;
	image_content_ >> temp;
	byte coef_type = temp >> 4;
	byte table_id = temp & 0x0F;

	std::vector<byte> huffman_codes_lenght(0x10);
	int number_of_lengths = 0;
	for (int i = 0; i < huffman_codes_lenght.size(); i++)
	{
		image_content_ >> huffman_codes_lenght[i];
		number_of_lengths += huffman_codes_lenght[i];
	}

	std::vector<byte> huffman_codes_values(number_of_lengths);
	for (int i = 0; i < huffman_codes_values.size(); i++)
	{
		image_content_ >> huffman_codes_values[i];
	}
	_huffman_trees[table_id].resize(2);
	_huffman_trees[table_id][coef_type] = new HuffmanTree();

	for (int i = 0, count = 0; i < huffman_codes_lenght.size(); i++)
	{
		while (huffman_codes_lenght[i])
		{
			_huffman_trees[table_id][coef_type]->AddElement(i + 1, huffman_codes_values[count++]);
			huffman_codes_lenght[i]--;
		}
	}
}

void Jpeg::process_quantization_table(InputBitStream& image_content_)
{
	byte size_1, size_2;
	image_content_ >> size_1 >> size_2;
	int size_of_table = size_1 * 0x100 + size_2;

	byte temp;
	image_content_ >> temp;
	byte value_in_2_bytes = temp >> 4;
	byte table_id = temp & 0x0F;

	int size_of_matrix = (int)sqrt(size_of_table - 3);

	_quantization_tables[table_id].resize(size_of_matrix, std::vector<int>(size_of_matrix));

	this->calculating_zigzag_order_traversal(size_of_table, size_of_matrix);

	for (int i = 3, t = 0; i < size_of_table; i += 1 + value_in_2_bytes, t++)
	{
		std::pair<int, int> next_index = _zigzag_order_traversal_indices[t];
		byte first_byte_of_value;
		image_content_ >> first_byte_of_value;
		_quantization_tables[table_id][next_index.first][next_index.second] = first_byte_of_value;
		if (value_in_2_bytes)
		{
			_quantization_tables[table_id][next_index.first][next_index.second] *= 0x100;
			byte second_byte_of_value;
			image_content_ >> second_byte_of_value;
			_quantization_tables[table_id][next_index.first][next_index.second] += second_byte_of_value;
		}
	}
}

void Jpeg::process_arithmetic_table(InputBitStream & image_content_)
{
	throw std::exception("Not implemented yet");
}

void Jpeg::process_start_of_scan(InputBitStream& image_content_)
{
	byte size_1, size_2;
	image_content_ >> size_1 >> size_2;
	int header_size = size_1 * 0x100 + size_2;
	byte number_components_to_read;
	image_content_ >> number_components_to_read;

	struct component_t
	{
		byte id;
		byte id_for_DC_and_AC_coefs;
	};

	std::vector<component_t> components(number_components_to_read);

	for (int i = 0; i < components.size(); i++)
	{
		image_content_ >> components[i].id;
		image_content_ >> components[i].id_for_DC_and_AC_coefs;
	}

	byte Ss, Se, A;
	image_content_ >> Ss >> Se >> A;
	byte Ah = A >> 4, Al = A & 0xFF;

	std::vector<std::vector<std::vector<int>>> resulting_matrices;
	int number_of_matrices = 6;
	bool skip_zero = false;
	while (number_of_matrices--)
	{
		if (skip_zero)
		{
			byte zeros[2];
			image_content_ >> zeros[0] >> zeros[1];
			if (zeros[0] == 0 && zeros[1] == 0)
			{
				skip_zero = false;
				continue;
			}
			else
			{
				image_content_.BytesBack(2);
			}
		}
		byte end_byte[2];
		image_content_ >> end_byte[0] >> end_byte[1];
		if (end_byte[0] == 0xFF)
		{
			if (end_byte[1] == markers::EOI)
			{
				break;
			}
			else if (end_byte[1] == 0)
			{
				skip_zero = true;
			}
		}
		image_content_.BytesBack(2);
		for (int matrix_number = 0; matrix_number < 6; matrix_number++)
		{
			int component_index = std::max(matrix_number - 3, 0);
			HuffmanTree::HuffmanTreeIterator* huffman_tree_iterator =
				new HuffmanTree::HuffmanTreeIterator(_huffman_trees[components[component_index].id_for_DC_and_AC_coefs >> 4][coef_type::DC]);
			std::vector<std::vector<int>> matrix(_quantization_tables[0].size(), std::vector<int>(_quantization_tables[0].size()));

			int zigzag_order_counter = 0;

			bit next;
			// DC coef
			while (image_content_ >> next)
			{
				huffman_tree_iterator->Step(next);
				if (huffman_tree_iterator->IsCodeEnd())
				{
					byte bits_to_read = huffman_tree_iterator->GetValue();
					// int number_of_0_to_add = huffman_tree_value >> 4;

					if (bits_to_read)
					{
						bit first_bit = 0;
						image_content_ >> first_bit;

						int real_value = first_bit;

						for (int i = 1; i < bits_to_read; i++)
						{
							real_value *= 2;
							bit next_bit;
							image_content_ >> next_bit;
							real_value += next_bit;
						}

						if (first_bit == 0)
						{
							real_value -= (1 << bits_to_read) - 1;
						}
						matrix[_zigzag_order_traversal_indices[zigzag_order_counter].first]
							[_zigzag_order_traversal_indices[zigzag_order_counter].second] = real_value;
					}
					zigzag_order_counter++;
					break;
				}
			}
			huffman_tree_iterator = 
				new HuffmanTree::HuffmanTreeIterator(_huffman_trees[components[component_index].id_for_DC_and_AC_coefs >> 4][coef_type::AC]);
			// AC coefs
			int number = 0;
			while (image_content_ >> next)
			{
				number++;
				huffman_tree_iterator->Step(next);
				if (huffman_tree_iterator->IsCodeEnd())
				{
					number+=10000;
					byte huffman_tree_value = huffman_tree_iterator->GetValue();
					if (huffman_tree_value == 0)
					{
						break;
					}

					byte number_of_0_to_add = huffman_tree_value >> 4;
					zigzag_order_counter += number_of_0_to_add;

					if (zigzag_order_counter >= _zigzag_order_traversal_indices.size())
					{
						break;
					}

					byte bits_to_read = huffman_tree_value & 0x0F;

					if (bits_to_read)
					{
						bit first_bit = 0;
						image_content_ >> first_bit;

						int real_value = first_bit;

						for (int i = 1; i < bits_to_read; i++)
						{
							real_value *= 2;
							bit next_bit;
							image_content_ >> next_bit;
							real_value += next_bit;
						}

						if (first_bit == 0)
						{
							real_value -= (1 << bits_to_read) - 1;
						}
						matrix[_zigzag_order_traversal_indices[zigzag_order_counter].first]
							[_zigzag_order_traversal_indices[zigzag_order_counter].second] = real_value;
						zigzag_order_counter++;
						if (zigzag_order_counter >= _zigzag_order_traversal_indices.size())
						{
							break;
						}
					}
					huffman_tree_iterator->Reset();
				}
			}

			resulting_matrices.push_back(matrix);
			// TODO
		}
	}
	for (int i = 2; i < 5; i++)
	{
		resulting_matrices[i + 1][0][0] += resulting_matrices[i][0][0];
	}
	// we have all matrices

}

void Jpeg::process_restart_interval(InputBitStream& image_content_)
{
	throw std::exception("Not implemented yet");
}

void Jpeg::process_application_specific(InputBitStream& image_content_)
{
	throw std::exception("Not implemented yet");
}

void Jpeg::process_comment(InputBitStream& image_content_)
{
	byte size_1, size_2;
	image_content_ >> size_1 >> size_2;
	int size_of_comment = size_1 * 0x100 + size_2;

	for (int i = 2; i < size_of_comment; i++)
	{
		byte temp;
		image_content_ >> temp;
		_comment += temp;
	}
}

void Jpeg::process_number_of_lines(InputBitStream & image_content_)
{
	throw std::exception("Not implemented yet");
}

void Jpeg::calculating_zigzag_order_traversal(int size_of_table_, int size_of_matrix_)
{
	// _zigzag_order_traversal_indices.resize(size_of_table_ - 3);
	// Calculating indices for zigzag order traversal
	for (int i = 0; i < 2 * size_of_matrix_ - 1; i++)
	{
		if (i % 2 == 1)
		{
			int x = i < size_of_matrix_ ? 0 : i - size_of_matrix_ + 1;
			int y = i < size_of_matrix_ ? i : size_of_matrix_ - 1;
			while (x < size_of_matrix_ && y >= 0) {
				_zigzag_order_traversal_indices.push_back({ x++, y-- });
			}
		}
		else
		{
			int x = i < size_of_matrix_ ? i : size_of_matrix_ - 1;
			int y = i < size_of_matrix_ ? 0 : i - size_of_matrix_ + 1;
			while (x >= 0 && y < size_of_matrix_) {
				_zigzag_order_traversal_indices.push_back({ x--, y++ });
			}
		}
	}
}


