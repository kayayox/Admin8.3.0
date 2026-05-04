/**==============================================================================
    Admin8.2.1 - PatternUtils.hpp
    Proposito: Serialización y deserialización de objetos Pattern (map<string, float>)
               para almacenamiento en BD.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef PATTERN_UTILS_HPP
#define PATTERN_UTILS_HPP

#include <map>
#include <string>

using WordPattern = std::map<std::string, float>;

std::string serializePattern(const WordPattern& pat);
WordPattern deserializePattern(const std::string& serialized);

#endif
