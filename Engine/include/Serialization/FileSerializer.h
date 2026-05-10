#pragma once

#include <fstream>
#include <Filesystem.h>

namespace ge::file {
	class Writer {
	public:
		Writer(const filesystem::Path& path) : _path(path) { _stream = std::ofstream(path.c_str(), std::ofstream::out | std::ofstream::binary); }
		~Writer() { _stream.close(); }
		bool IsStreamGood() const { return _stream.good(); }
		uint64_t GetStreamPosition() { return _stream.tellp(); }
		void SetStreamPosition(uint64_t pos) { _stream.seekp(pos); }
		bool WriteData(const char* data, size_t size) { _stream.write(data, size); return true; }
		bool WriteString(const char* str, size_t) { _stream << str; return true; }
	private:
		filesystem::Path _path;
		std::ofstream _stream;
	};

	class Reader {
	public:
		Reader(const filesystem::Path& path) : _path(path) { _stream = std::ifstream(path.c_str(), std::ifstream::in | std::ifstream::binary); }
		Reader(const filesystem::Path& path, std::ios_base::openmode mode) : _path(path) { _stream = std::ifstream(path.c_str(), mode); }
		Reader() : _path("") {}
		~Reader() { _stream.close(); }
		void OpenStream(const filesystem::Path& path) { _path = path; _stream.open(path.c_str(), std::ifstream::in | std::ifstream::binary); }
		bool IsStreamGood() const { return _stream.good(); }
		uint64_t GetStreamPosition() { return _stream.tellg(); }
		void SetStreamPosition(uint64_t pos) { _stream.seekg(pos); }
		bool ReadData(char* destination, size_t size) { _stream.read(destination, size); return true; }
	private:
		filesystem::Path _path;
		std::ifstream _stream;
	};
}