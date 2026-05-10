#pragma once
#include "Core.h"
#include "Logger.h"
#include <algorithm>
#include <functional>

#ifdef _WIN32
#include <direct.h>
#include <Windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace ge::filesystem {
    class Path {
    public:
        Path() = default;
        Path(const char* path) : _path(path) { Normalize(); }
        Path(const GEString& path) : _path(path) { Normalize(); }

        std::string_view GetExtension() const {
            size_t dot = _path.find_last_of('.');
            if (dot == GEString::npos) return "";
            return std::string_view(_path).substr(dot);
        }

        filesystem::Path ReplaceExtension(const GEString& newExt) {
            size_t dot = _path.find_last_of('.');
            if (dot != GEString::npos) {
                _path = _path.substr(0, dot);
            }

            if (newExt.empty()) return _path;

            if (newExt[0] != '.') _path += '.';
            _path += newExt;
            return _path;
        }

        GEString GetFileName() const {
            size_t slash = _path.find_last_of('/');
            size_t start = (slash == GEString::npos) ? 0 : slash + 1;
            size_t dot = _path.find_last_of('.');
            return GEString(_path).substr(start, dot - start);
        }

        Path GetParentPath() const {
            if (_path.empty()) return Path("");

            std::string_view sv(_path);

            if (sv.length() > 1 && sv.back() == '/') {
                sv.remove_suffix(1);
            }

            size_t lastSlash = sv.find_last_of('/');

            if (lastSlash == std::string_view::npos) {
                return Path("");
            }
            return Path(GEString(sv.substr(0, lastSlash)));
        }

        const bool empty() const { return _path.empty(); }
        const GEString& string() const { return _path; }
        const char* c_str() const { return _path.c_str(); }

        Path& operator/=(const Path& other) {
            if (other._path.empty()) return *this;

            if (!_path.empty() && _path.back() != '/') {
                _path += '/';
            }

            GEString appendPart = other._path;
            if (!appendPart.empty() && appendPart.front() == '/') {
                appendPart.erase(0, 1);
            }

            _path += appendPart;
            return *this;
        }

        friend Path operator/(Path lhs, const Path& rhs) {
            lhs /= rhs;
            return lhs;
        }

        Path& operator/=(const char* rhs) {
            return *this /= Path(rhs);
        }

        friend Path operator/(Path lhs, const char* rhs) {
            lhs /= Path(rhs);
            return lhs;
        }

        bool operator==(const Path& other) { return _path == other._path; }
    private:
        void Normalize() {
            std::replace(_path.begin(), _path.end(), '\\', '/');
        }

        GEString _path;
    };

    class FileSystem { // TODO (0x): add platform abstaction class for this kind of operations
    public:
        using IterateCallback = std::function<void(const Path&)>;

        static bool CreateDir(const Path& path) {
#ifdef _WIN32
            return CreateDirectoryA(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
#else
            return mkdir(path.c_str(), 0777) == 0;
#endif
        }

        static bool Exists(const Path& path) {
#ifdef _WIN32
            DWORD dwAttrib = GetFileAttributesA(path.c_str());
            return (dwAttrib != INVALID_FILE_ATTRIBUTES);
#else
            return access(path.c_str(), F_OK) == 0;
#endif
        }

        static Path GetAbsolutePath(const Path& path) {
#ifdef _WIN32
            char buffer[MAX_PATH];
            DWORD length = GetFullPathNameA(path.c_str(), MAX_PATH, buffer, NULL);

            if (length == 0) {
                return path;
            }

            return Path(buffer);
#else
            char buffer[PATH_MAX];
            char* res = realpath(path.c_str(), buffer);
            if (res) return Path(buffer);
            return path;
#endif
        }

        static void IterateDirectory(const Path& path, const IterateCallback& callback) {
#ifdef _WIN32
            GEString searchPath = path.string() + "/*";
            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    const char* name = findData.cFileName;
                    if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                        callback(path / name);
                    }
                } while (FindNextFileA(hFind, &findData));
                FindClose(hFind);
            }
#else
            DIR* dir = opendir(path.c_str());
            if (dir) {
                struct dirent* entry;
                while ((entry = readdir(dir)) != nullptr) {
                    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                        callback(path / entry->d_name);
                    }
                }
                closedir(dir);
            }
#endif
        }
    };
}