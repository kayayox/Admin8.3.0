/**==============================================================================
    Admin8.2.1 - Refiner.hpp
    Proposito: Funciones de refinamiento contextual de etiquetas usando
               modelos de unigramas, bigramas y trigramas.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef ADMIN821_REFINER_HPP
#define ADMIN821_REFINER_HPP

#include "../common/types.hpp"
#include "TagStats.hpp"
#include <vector>
#include <algorithm>
#include <utility>

struct TagConfidence {
    TipoPalabra tag;
    float confidence;
};

// Obtener la mejor predicción de una lista de pares (tag, probabilidad)
inline std::pair<TipoPalabra, float> getBestPrediction(const std::vector<std::pair<TipoPalabra, float>>& preds) {
    if (preds.empty()) return {TipoPalabra::INDEFINIDO, 0.0f};
    return *std::max_element(preds.begin(), preds.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
}

// Refinar la etiqueta de una palabra dado el contexto (dos anteriores, actual, siguiente)
TagConfidence refineTag(TipoPalabra prev2, TipoPalabra prev, TipoPalabra current,
                        TipoPalabra next, float currentConfidence);

#endif // ADMIN821_REFINER_HPP
