#pragma once
#include <string>
#include <format>
#include <filesystem>
#include <string_view>
#include "GlassEngine/Memory/Allocator.h"

class GEString : public std::basic_string<char, std::char_traits<char>, ge::mem::GE_Allocator<char>> { // UTF8
public:
	using BaseString = std::basic_string<char, std::char_traits<char>, ge::mem::GE_Allocator<char>>;
	using BaseString::basic_string;

	GEString(const BaseString& other) : BaseString(other) {}
	GEString(BaseString&& other) noexcept : BaseString(std::move(other)) {}
	GEString(std::string other) : BaseString(std::move(other)) {}
	GEString& operator=(const BaseString& other) { BaseString::operator=(other); return *this; }
	GEString& operator=(BaseString&& other) noexcept { BaseString::operator=(std::move(other)); return *this; }

	std::filesystem::path ToPath() const { return std::filesystem::path(this->begin(), this->end()); }

	operator std::string_view() const { return std::string_view(this->data(), this->size()); }
	operator std::string() const { return std::string(this->begin(), this->end()); }
};
namespace std {
	template <>
	struct hash<GEString> {
		size_t operator()(const GEString& k) const noexcept {
			return hash<string_view>{}(string_view(k.data(), k.size()));
		}
	};
	template <>
	struct formatter<GEString> : formatter<string_view> {
		auto format(const GEString& str, format_context& ctx) const {
			return formatter<string_view>::format(string_view(str.data(), str.size()), ctx);
		}
	};
}

// TODO (badev): make a GEWString class using wstring utf16 for localization