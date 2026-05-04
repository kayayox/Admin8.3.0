/**==============================================================================
    Admin8.2.1 - ContextualCorrelator.hpp
    Proposito: Capa de conveniencia sobre PatternCorrelator que maneja contextos
               de hasta dos palabras anteriores y usa patrones de una sola palabra.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef CONTEXTUAL_CORRELATOR_H
#define CONTEXTUAL_CORRELATOR_H

#include "PatternCorrelator.hpp"
#include "../utils/PatternUtils.hpp"
#include <memory>
#include <string>
#include <vector>

class ContextualCorrelator {
public:
    explicit ContextualCorrelator(const std::string& dbPath);

    void learnWithPreviousTwo(const std::string& text);
    void learnNextWordDirect(const std::string& text);

    bool queryNext(const std::string& current,
                   const std::vector<std::string>& previousWords,
                   std::vector<std::pair<WordPattern, double>>& outcomes);

    bool queryNextWithTwoPrev(const std::string& current,
                              const std::string& prev1,
                              const std::string& prev2,
                              std::vector<std::pair<WordPattern, double>>& outcomes);

    bool queryNextWithOnePrev(const std::string& current,
                              const std::string& prev,
                              std::vector<std::pair<WordPattern, double>>& outcomes);

private:
    std::unique_ptr<PatternCorrelator> corr;  // usa sufijo "" (tablas estándar)
    WordPattern makePattern(const std::vector<std::string>& words) const;
};

#endif
