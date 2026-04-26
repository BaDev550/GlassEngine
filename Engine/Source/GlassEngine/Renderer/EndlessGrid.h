#pragma once
#include "Buffer.h"
#include "Pipeline.h"
#include "Framebuffer.h"
#include "glm/glm.hpp"

namespace ge::renderer {
	class EndlessGrid : public mem::RefCounted
	{
	public:
		EndlessGrid(mem::Ref<Framebuffer>& framebuffer);
		void Draw();
	private:
		uint32_t _vertexCount = 6;
		mem::Ref<Buffer> _gridBuffer;
		mem::Ref<Pipeline> _pipeline;
	};
}