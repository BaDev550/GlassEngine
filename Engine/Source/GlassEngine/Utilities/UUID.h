#pragma once
#include <iostream>
#include <random>
#include <sstream>

namespace ge {
	class UUID {
	public:
		UUID() {
			std::random_device rd;
			std::mt19937_64 gen(rd());
			std::uniform_int_distribution<uint64_t> dis;
			_uuid = dis(gen);
		}
		UUID(uint64_t uuid) : _uuid(uuid) {}
		UUID(const std::string& uuidStr) { _uuid = std::stoull(uuidStr, nullptr, 16); }
		std::string ToString() const {
			std::stringstream ss;
			ss << std::hex << _uuid;
			return ss.str();
		}

		operator uint64_t() const { return _uuid; }
	private:
		uint64_t _uuid;
	};
}

namespace std {
	template<>
	struct hash<ge::UUID> {
		size_t operator()(const ge::UUID& uuid) const {
			return hash<uint64_t>()(static_cast<uint64_t>(uuid));
		}
	};
}