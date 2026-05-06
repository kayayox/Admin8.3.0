/**==============================================================================
    Admin8.2.1 - PatternUtils.cpp
    Proposito: Implementación de serialización de patrones.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "PatternUtils.hpp"
#include <sstream>
#include <iostream>

std::string serializePattern(const WordPattern& pat) {
    std::stringstream ss;
    for (auto it = pat.begin(); it != pat.end(); ++it) {
        if (it != pat.begin()) ss << ";";
        ss << it->first << ":" << it->second;
    }
    return ss.str();
}

WordPattern deserializePattern(const std::string& serialized) {
    WordPattern pat;
    if (serialized.empty()) return pat;   // Evitar procesar cadena vacía

    std::stringstream ss(serialized);
    std::string item;
    while (std::getline(ss, item, ';')) {
        size_t colon = item.find(':');
        if (colon != std::string::npos) {
            std::string word = item.substr(0, colon);
            std::string weightStr = item.substr(colon + 1);
            float weight = 0.0f;
            try {
                weight = std::stof(weightStr);
            } catch (const std::invalid_argument& e) {
                std::cerr << "[ERROR] PatternUtils: Valor no numérico al deserializar: '"
                          << weightStr << "' para la palabra '" << word << "'" << std::endl;
                continue;   // Saltar este elemento corrupto
            } catch (const std::out_of_range& e) {
                std::cerr << "[ERROR] PatternUtils: Número fuera de rango: '"
                          << weightStr << "' para la palabra '" << word << "'" << std::endl;
                continue;
            }
            pat[word] = weight;
        } else {
            // Si el formato es incorrecto (falta ':'), también lo reportamos y saltamos
            std::cerr << "[ERROR] PatternUtils: Formato inválido en item: '" << item << "'" << std::endl;
        }
    }
    return pat;
}
