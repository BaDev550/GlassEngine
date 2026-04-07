#pragma once

namespace ge {
    class Blob {
    public:
        virtual ~Blob() = default;
        virtual uint32_t GetDataSize() = 0;
        virtual void* GetDataPointer() = 0;
    };
}
