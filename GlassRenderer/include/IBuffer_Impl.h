#pragma once

#include <Graphics/Interfaces/IBuffer.h>

#include "rhi/RHIObject.h"

namespace ge::graphics {
	class GE_API IBuffer_Impl : public IBuffer, public RHIObject {
	public:
		virtual ~IBuffer_Impl() = default;
		static mem::Ref<IBuffer_Impl> Create(BufferSpec specs);
	};
}