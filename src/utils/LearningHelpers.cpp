/**==============================================================================
    Admin8.2.1 - LearningHelpers.cpp
    Proposito: Implementación de ayudas de aprendizaje.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "LearningHelpers.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <limits>

void learnContextual(ContextualCorrelator& ctx, const std::string& text) {
    if (text.empty()) return;
    ctx.learnWithPreviousTwo(text);
}

void learnDirect(ContextualCorrelator& ctx, const std::string& text) {
    if (text.empty()) return;
    ctx.learnNextWordDirect(text);
}

void learnTextWithContext(ContextualCorrelator& ctx, PatternCorrelator& corr, const std::string& text) {
    if (text.empty()) return;
    corr.learnFromText(text, 1);
    ctx.learnWithPreviousTwo(text);
    ctx.learnNextWordDirect(text);
}

void clearInputBuffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void trimString(std::string& line) {
    line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), line.end());
}
