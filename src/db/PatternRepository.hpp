/**==============================================================================
    Admin8.2.1 - PatternRepository.hpp
    Proposito: Repositorio para persistir patrones gramaticales (Patron) en la
               base de datos de patrones.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Los patrones se serializan como string de enteros separados por coma.
==============================================================================*/

#ifndef ADMIN821_PATTERN_REPOSITORY_HPP
#define ADMIN821_PATTERN_REPOSITORY_HPP

#include "../core/Pattern.hpp"
#include <iostream>
#include <optional>
#include <vector>

class PatternRepository {
public:
    static void setDatabasePath(const std::string& path);
    static std::string getDatabasePath();
    static void initializeTables();

    // Guardar un patrón (incrementa frecuencia si ya existe)
    static void save(const Pattern& pattern);

    // Buscar un patrón que coincida exactamente con la secuencia dada.
    // Retorna el patrón encontrado y llena outSimilitud (1.0 si coincide exacto).
    static std::optional<Pattern> findExactMatch(const std::vector<TipoPalabra>& sequence,
                                                  float& outSimilitud);

    // Cargar todos los patrones
    static std::vector<Pattern> loadAll();

    // Eliminar un patrón (opcional)
    static bool remove(const Pattern& pattern);
};

#endif // ADMIN821_PATTERN_REPOSITORY_HPP
