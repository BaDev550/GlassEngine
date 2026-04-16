#pragma once
#include <vector>
#include <cstdint>

namespace ge::file {
	class BufferReader {
	public:
		BufferReader(const GEVector<uint8_t>& buffer) : _buffer(buffer), _offset(0) {}
		bool ReadData(void* dest, size_t size) {
			if (_offset + size > _buffer.size()) {
				return false;
			}
			std::memcpy(dest, _buffer.data() + _offset, size);
			_offset += size;
			return true;
		}

		template<typename T>
		bool Read(T& dest) { return ReadData(&dest, sizeof(T)); }
		const uint8_t* ReadPtr(size_t size) {
			if (_offset + size > _buffer.size()) {
				return nullptr;
			}
			const uint8_t* ptr = _buffer.data() + _offset;
			_offset += size;
			return ptr;
		}

		bool IsStreamGood() const { return _offset < _buffer.size(); }
		size_t GetOffset() const { return _offset; }
		size_t GetRemaining() const { return _buffer.size() - _offset; }
	private:
		const GEVector<uint8_t>& _buffer;
		size_t _offset;
	};
}