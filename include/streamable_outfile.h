#pragma once

#include <fstream>
#include <cstdint>
#include <cstring>
#include <vector>

namespace reven {
namespace vmghost {

//! A basic file that hides the details of boost filters used by the various vbox output file formats.
class streamable_outfile {
public:
	streamable_outfile();

	//! Loads the specified file.
	void open(const std::string& file_name);

	//! Writes some value from the input stream. Defined for basic types only.
	template <typename T> void write_raw(const T& value);

	//! Writes an array of characters from the input stream.
	void write_raw(const char* destination, unsigned int length);

	//! Close the file and mark it eof.
	void close();

private:
	//! Raw data
	std::ofstream out_file_;
}; // class streamable_outfile

inline void streamable_outfile::write_raw(const char* destination, unsigned int length)
{
	out_file_.write(destination, length);
}

template <typename T> void streamable_outfile::write_raw(const T& value)
{
	write_raw(reinterpret_cast<const char*>(&value), sizeof(value));
	;
}

inline streamable_outfile& operator<<(streamable_outfile& out, const bool& data)
{
	out.write_raw(data);
	return out;
}

inline streamable_outfile& operator<<(streamable_outfile& out, const std::uint8_t& data)
{
	out.write_raw(data);
	return out;
}

inline streamable_outfile& operator<<(streamable_outfile& out, const std::uint16_t& data)
{
	out.write_raw(data);
	return out;
}

inline streamable_outfile& operator<<(streamable_outfile& out, const std::uint32_t& data)
{
	out.write_raw(data);
	return out;
}

inline streamable_outfile& operator<<(streamable_outfile& out, const std::uint64_t& data)
{
	out.write_raw(data);
	return out;
}

inline streamable_outfile& operator<<(streamable_outfile& out, const std::string& data)
{
	std::uint64_t length = data.size();
	out << length;

	out.write_raw(data.data(), length);

	return out;
}

template <typename T> streamable_outfile& operator<<(streamable_outfile& out, typename std::vector<T> const& data)
{
	std::uint64_t size = data.size();
	out << size;

	for (auto const& item : data) {
		out << item;
	}
	return out;
}
}
} // namespace reven::vmghost
