#pragma once

#include <fstream>
#include <cstdint>
#include <cstring>
#include <vector>

#include <iostream>

namespace reven {
namespace vmghost {

//! A basic file that hides the details of boost filters used by the various vbox output file formats.
class streamable_file {
public:
	streamable_file();

	//! Loads the specified file. If there is an error, eof() will return true.
	void load(const std::string& file_name);

	//! Reads some value from the input stream. Defined for basic types only.
	template <typename T> void read_raw(T& value);

	//! Reads an array of characters from the input stream.
	void read_raw(char* destination, unsigned int length);

	void seek(std::uint64_t pos)
	{
		eof_ = false;
		in_file_.clear();
		in_file_.seekg(pos);
		check_eof();
	}

	void seek_from_end(std::int64_t pos)
	{
		eof_ = false;
		in_file_.clear();
		in_file_.seekg(-pos, std::ios_base::end);
		check_eof();
	}

	std::uint64_t pos() const { return in_file_.tellg(); }

	//! Returns true if end of file is reached or an error occurred while reading.
	bool eof() const { return eof_; }

	//! Returns true if a scenario file was opened.
	bool is_open() const { return opened_; }

	//! Close the file and mark it eof.
	void close();

private:
	void check_eof() { eof_ = in_file_.bad() || in_file_.eof() || !in_file_.is_open(); }

	//! Raw data
	mutable std::ifstream in_file_;

	//! Set if the last read operation failed or end of file is reached
	bool eof_;
	bool opened_;
}; // class streamable_file

inline void streamable_file::read_raw(char* destination, unsigned int length)
{
	std::memset(destination, 0, length);
	if (not eof()) {
		in_file_.read(destination, length);
		check_eof();
	}
}

template <typename T> void streamable_file::read_raw(T& value)
{
	read_raw(reinterpret_cast<char*>(&value), sizeof(value));
	;
}

inline streamable_file& operator>>(streamable_file& in, bool& data)
{
	in.read_raw(data);
	return in;
}

inline streamable_file& operator>>(streamable_file& in, std::uint8_t& data)
{
	in.read_raw(data);
	return in;
}

inline streamable_file& operator>>(streamable_file& in, std::uint16_t& data)
{
	in.read_raw(data);
	return in;
}

inline streamable_file& operator>>(streamable_file& in, std::uint32_t& data)
{
	in.read_raw(data);
	return in;
}

inline streamable_file& operator>>(streamable_file& in, std::uint64_t& data)
{
	in.read_raw(data);
	return in;
}

inline streamable_file& operator>>(streamable_file& in, std::string& data)
{
	std::uint64_t length;
	in >> length;

	data.reserve(length);

	char received[length + 1];
	in.read_raw(received, length);
	received[length] = 0;

	data = received;
	return in;
}

template <typename T> streamable_file& operator>>(streamable_file& in, typename std::vector<T>& data)
{
	std::uint64_t size;
	in >> size;
	data.clear();
	for (std::uint64_t i = 0; i < size && !in.eof(); ++i) {
		T item;
		in >> item;
		data.push_back(std::move(item));
	}
	return in;
}
}
} // namespace reven::vmghost
