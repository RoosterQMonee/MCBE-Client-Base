#include "GuiData.h"

#include <Util/Memory.h>
#include <Dependencies/spdlog/spdlog.h>
#include <Dependencies/LibHat/libhat/Scanner.hpp>

void GuiData::DisplayClientMessage(const std::string& str) {
    if (str.empty())
        return;

    static uintptr_t sig;

    if (sig == NULL) {
        hat::scan_result result = hat::find_pattern(Signatures::DisplayClientMessage, ".text");
        const std::byte* address = result.get();

        if (address == nullptr) {
            spdlog::warn("Cannot find GuiData::DisplayClientMessage!");
            return;
        }
    }

    char padding[0x28]{};

    using func_t = void(*)(GuiData*, const std::string&, char*, bool);
    static auto func = reinterpret_cast<func_t>(sig);
    func(this, str, padding, true);
}