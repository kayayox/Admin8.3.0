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

void SlotFiller::setPremiseContext(const std::string& subj, const std::string& verb, const std::string& obj) {
    premiseSubject_ = subj;
    premiseVerb_ = verb;
    premiseObject_ = obj;
}

void SlotFiller::clearPremiseContext() {
    premiseSubject_.clear();
    premiseVerb_.clear();
    premiseObject_.clear();
}

// Modificar predictForSlot para que primero intente con los valores de la premisa
std::string SlotFiller::predictForSlot(const std::string& slotName,
                                       TipoPalabra expectedType,
                                       const std::vector<TipoPalabra>& prevTagContext,
                                       const std::vector<std::string>& prevWordContext) {
    // Prioridad 1: si el slot coincide con sujeto/verbo/objeto de la premisa
    if (slotName == "sujeto" && !premiseSubject_.empty()) {
        Word w;
        if (WordRepository::load(premiseSubject_, w) && w.getTipo() == expectedType)
            return premiseSubject_;
    }
    if (slotName == "verbo" && !premiseVerb_.empty()) {
        Word w;
        if (WordRepository::load(premiseVerb_, w) && w.getTipo() == expectedType)
            return premiseVerb_;
    }
    if (slotName == "objeto" && !premiseObject_.empty()) {
        Word w;
        if (WordRepository::load(premiseObject_, w) && w.getTipo() == expectedType)
            return premiseObject_;
    }

    // Prioridad 2: usar correlador contextual para predecir
    std::vector<std::pair<WordPattern, double>> outcomes;
    // Usamos un contexto vacío (o podríamos pasar prevWordContext)
    if (ctxCorr.queryNext(prevWordContext.back(), prevWordContext, outcomes)) {
        for (const auto& outcome : outcomes) {
            for (const auto& wc : outcome.first) {
                Word w;
                if (WordRepository::load(wc.first, w) && w.getTipo() == expectedType) {
                    return wc.first;
                }else{
                    if(wc.second > 0.5f)return wc.first;
                }
            }
        }
    }

    // Prioridad 3: si no hay predicción, devolver una palabra genérica del tipo esperado
    if (expectedType == TipoPalabra::SUST) return "cosa";
    if (expectedType == TipoPalabra::VERB) return "hacer";
    if (expectedType == TipoPalabra::ADJT) return "bueno";
    return "";
}

std::string SlotFiller::predictForSlot(const std::string& slotName,
                                       const std::vector<TipoPalabra>& prevTagContext,
                                       const std::vector<std::string>& prevWordContext) {
    TipoPalabra expected = inferTypeFromSlotName(slotName);
    return predictForSlot(slotName, expected, prevTagContext, prevWordContext);
}
