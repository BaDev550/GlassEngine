#pragma once
#include <filesystem>
#include "Asset.h"

namespace ge {
	enum class AssetLoadingState : uint8_t {
		NotLoaded = 0,
		Loading,
		Loaded,
		Failed
	};

	struct AssetMetadata {
		AssetHandle handle = GE_INVALID_ASSET_HANDLE;
		AssetType type = GE_INVALID_ASSET_TYPE;
		AssetLoadingState state = AssetLoadingState::NotLoaded;
		std::filesystem::path path;

		bool IsLoaded() const { return state == AssetLoadingState::Loaded; }
		bool IsValid() const { return type != GE_INVALID_ASSET_TYPE; }
	};
}