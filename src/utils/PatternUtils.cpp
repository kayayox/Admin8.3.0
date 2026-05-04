/**==============================================================================
    Admin8.2.1 - PatternUtils.cpp
    Proposito: Implementación de serialización de patrones.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "PatternUtils.hpp"
#include <sstream>

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
    std::stringstream ss(serialized);
    std::string item;
    while (std::getline(ss, item, ';')) {
        size_t colon = item.find(':');
        if (colon != std::string::npos) {
            std::string word = item.substr(0, colon);
            float weight = std::stof(item.substr(colon + 1));
            pat[word] = weight;
        }
    }
    return pat;
}
