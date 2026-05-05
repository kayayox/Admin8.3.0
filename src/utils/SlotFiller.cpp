/**==============================================================================
    Admin8.3.0 - SlotFiller.cpp
    Proposito: Implementación de la asignacion de texto en los slots de los template.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "SlotFiller.hpp"
#include "../nlp/TagStats.hpp"
#include "../db/WordRepository.hpp"
#include <algorithm>

SlotFiller::SlotFiller(ContextualCorrelator& correlator) : ctxCorr(correlator) {}

TipoPalabra SlotFiller::inferTypeFromSlotName(const std::string& slotName) {
    if (slotName == "sujeto" || slotName == "objeto" || slotName == "complemento" || slotName == "algo")
        return TipoPalabra::SUST;
    if (slotName == "verbo")
        return TipoPalabra::VERB;
    if (slotName == "articulo")
        return TipoPalabra::ART;
    if (slotName == "adjetivo")
        return TipoPalabra::ADJT;
    if (slotName == "adverbio")
        return TipoPalabra::ADV;
    return TipoPalabra::INDEFINIDO;
}

std::string SlotFiller::predictForSlot(const std::string& slotName,
                                       TipoPalabra expectedType,
                                       const std::vector<TipoPalabra>& /*prevTagContext*/,
                                       const std::vector<std::string>& prevWordContext) {
    // Obtener la palabra actual (última del contexto)
    std::string currentWord = prevWordContext.empty() ? "" : prevWordContext.back();
    std::vector<std::string> previous = prevWordContext;
    if (!currentWord.empty() && !previous.empty() && previous.back() == currentWord)
        previous.pop_back();

    std::vector<std::pair<WordPattern, double>> outcomes;
    if (!ctxCorr.queryNext(currentWord, previous, outcomes))
        return "";

    // Filtrar por tipo esperado usando WordRepository estático
    for (const auto& outcome : outcomes) {
        for (const auto& wc : outcome.first) {
            Word w;
            if (WordRepository::load(wc.first, w) && w.getTipo() == expectedType) {
                return wc.first;
            }
        }
    }
    // Si no se encuentra ninguna del tipo esperado, devolver la más probable
    return outcomes.empty() ? "" : outcomes[0].first.begin()->first;
}

std::string SlotFiller::predictForSlot(const std::string& slotName,
                                       const std::vector<TipoPalabra>& prevTagContext,
                                       const std::vector<std::string>& prevWordContext) {
    TipoPalabra expected = inferTypeFromSlotName(slotName);
    return predictForSlot(slotName, expected, prevTagContext, prevWordContext);
}

std::unordered_map<std::string, std::string> SlotFiller::fillSlots(
    const std::vector<std::string>& slots,
    const std::unordered_map<std::string, TipoPalabra>& slotTypes,
    const std::vector<TipoPalabra>& initialTagContext,
    const std::vector<std::string>& initialWordContext) {

    std::unordered_map<std::string, std::string> result;
    std::vector<TipoPalabra> currentTagContext = initialTagContext;
    std::vector<std::string> currentWordContext = initialWordContext;

    for (const auto& slot : slots) {
        TipoPalabra expected = TipoPalabra::INDEFINIDO;
        auto it = slotTypes.find(slot);
        if (it != slotTypes.end()) expected = it->second;
        else expected = inferTypeFromSlotName(slot);

        std::string value = predictForSlot(slot, expected, currentTagContext, currentWordContext);
        result[slot] = value;

        if (!value.empty()) {
            currentWordContext.push_back(value);
            currentTagContext.push_back(expected);
        }
    }
    return result;
}
