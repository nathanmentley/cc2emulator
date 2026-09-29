#pragma once

#include <array>
#include <atomic>

#include <Compucolor/Common/IPluginManager.h>

#include <Compucolor/Memory/IMemory.h>

namespace Compucolor::Impl::Memory
{
    class VectorMemory: public Compucolor::Memory::IMemory {
        public:
            VectorMemory();

            uint8_t GetByte(uint16_t address) override;
            void SetByte(uint16_t address, uint8_t data) override;

            static std::shared_ptr<IMemory> Creator(Common::IPluginManager* manager);

        private:
            std::array<std::atomic<uint8_t>, 64 * 1024> _data{};
    };
}