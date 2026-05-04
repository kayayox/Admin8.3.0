#ifndef SLOT_FILLER_HPP
#define SLOT_FILLER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "../common/types.hpp"
#include "../dialogue/ContextualCorrelator.hpp"

class SlotFiller {
public:
    explicit SlotFiller(ContextualCorrelator& correlator);

    // Predecir una palabra para un slot dado el tipo gramatical esperado y el contexto previo
    std::string predictForSlot(const std::string& slotName,
                               TipoPalabra expectedType,
                               const std::vector<TipoPalabra>& prevTagContext,
                               const std::vector<std::string>& prevWordContext);

    // Versión que infiere el tipo esperado a partir del nombre del slot
    std::string predictForSlot(const std::string& slotName,
                               const std::vector<TipoPalabra>& prevTagContext,
                               const std::vector<std::string>& prevWordContext);

    // Rellenar múltiples slots de forma iterativa
    std::unordered_map<std::string, std::string> fillSlots(
        const std::vector<std::string>& slots,
        const std::unordered_map<std::string, TipoPalabra>& slotTypes,
        const std::vector<TipoPalabra>& initialTagContext,
        const std::vector<std::string>& initialWordContext);

    TipoPalabra inferTypeFromSlotName(const std::string& slotName);

private:
    ContextualCorrelator& ctxCorr;
};

#endif // SLOT_FILLER_HPP
