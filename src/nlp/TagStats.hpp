/**==============================================================================
    Admin8.2.1 - TagStats.hpp
    Proposito: Estadísticas de transición de etiquetas (unigramas, bigramas,
               trigramas) para el refinamiento contextual. Los datos se guardan
               en la base de datos de patrones.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef ADMIN821_TAG_STATS_HPP
#define ADMIN821_TAG_STATS_HPP

#include "../common/types.hpp"
#include <iostream>
#include <vector>
#include <utility>

namespace TagStats {

    // Configurar la ruta de la base de datos de patrones (debe llamarse antes de usar)
    void setDatabasePath(const std::string& path);

    // Crear las tablas si no existen
    void initializeTables();

    // Actualización de conteos
    void updateUnigram(TipoPalabra prev, TipoPalabra curr, int inc = 1);
    void updateBigram(TipoPalabra prev2, TipoPalabra prev, TipoPalabra curr, int inc = 1);
    void updateTrigram(TipoPalabra prev, TipoPalabra curr, TipoPalabra next, int inc = 1);

    // Consultas: predicción de la etiqueta actual 'curr'
    std::vector<std::pair<TipoPalabra, float>> getUnigramProbs(TipoPalabra prev);
    std::vector<std::pair<TipoPalabra, float>> getBigramProbs(TipoPalabra prev2, TipoPalabra prev);
    std::vector<std::pair<TipoPalabra, float>> getTrigramProbs(TipoPalabra prev, TipoPalabra next);

    // Cargar datos iniciales desde estáticos (solo si las tablas están vacías)
    void loadDefaultFromStatic();
}

#endif // ADMIN821_TAG_STATS_HPP
