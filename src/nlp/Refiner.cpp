/**==============================================================================
    Admin8.2.1 - Refiner.cpp
    Proposito: Implementación del refinamiento contextual.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "Refiner.hpp"

TagConfidence refineTag(TipoPalabra prev2, TipoPalabra prev, TipoPalabra current,
                        TipoPalabra next, float currentConfidence) {
    // Si la etiqueta actual tiene confianza media-alta, mantenerla
    if (current != TipoPalabra::INDEFINIDO && currentConfidence >= 0.5f) {
        return {current, currentConfidence};
    }

    auto trigram = TagStats::getTrigramProbs(prev, next);
    auto bigram  = TagStats::getBigramProbs(prev2, prev);
    auto unigram = TagStats::getUnigramProbs(prev);

    TagConfidence result={TipoPalabra::INDEFINIDO, 0.0f};

    if (current == TipoPalabra::INDEFINIDO) {
        // Sin etiqueta actual: elegir la mejor entre todos los modelos
        if (!trigram.empty()) result = {getBestPrediction(trigram).first, getBestPrediction(trigram).second};
        if (!bigram.empty()) {
            auto best = getBestPrediction(bigram);
            if (best.second > result.confidence) result = {best.first, best.second};
        }
        if (!unigram.empty()) {
            auto best = getBestPrediction(unigram);
            if (best.second > result.confidence) result = {best.first, best.second};
        }
        return result;
    }

    // Si hay etiqueta actual, favorecer trigrama si coincide
    if (!trigram.empty()) {
        auto best = getBestPrediction(trigram);
        if (best.first == current) {
            float newConf = std::min(1.0f, currentConfidence + 0.2f);
            return {current, newConf};
        } else {
            result = {best.first, best.second};
        }
    }
    if (!bigram.empty()) {
        auto best = getBestPrediction(bigram);
        if (best.first == current) {
            float newConf = std::min(1.0f, currentConfidence + 0.1f);
            return {current, newConf};
        } else if (best.second > result.confidence) {
            result = {best.first, best.second};
        }
    }
    if (!unigram.empty()) {
        auto best = getBestPrediction(unigram);
        if (best.first == current) {
            return {current, currentConfidence + 0.05f};
        } else if (best.second > result.confidence) {
            result = {best.first, best.second};
        }
    }
    return result;
}
