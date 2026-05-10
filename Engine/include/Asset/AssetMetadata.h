#pragma once
#include <Filesystem.h>
#include "Asset.h"

namespace ge {
	enum class AssetLoadingState : uint8_t {
		NotLoaded = 0,
		Loading,
		Loaded,
		Failed
	};

	constexpr std::string AssetLoadingStateToString(AssetLoadingState type) noexcept {
		switch (type)
		{
		case AssetLoadingState::NotLoaded: return "NotLoaded";
		case AssetLoadingState::Loading: return "Loading";
		case AssetLoadingState::Loaded: return "Loaded";
		case AssetLoadingState::Failed: return "Failed";
		default:
			return "Unknown";
		};
	}

	struct AssetMetadata {
		AssetHandle handle = GE_INVALID_ASSET_HANDLE;
		AssetType type = GE_INVALID_ASSET_TYPE;
		AssetLoadingState state = AssetLoadingState::NotLoaded;
		filesystem::Path path;

		bool IsLoaded() const { return state == AssetLoadingState::Loaded; }
		bool IsValid() const { return type != GE_INVALID_ASSET_TYPE; }
	};
}