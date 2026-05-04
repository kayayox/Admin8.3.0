/**==============================================================================
    Admin8.2.1 - ContextualCorrelator.cpp
    Proposito: Implementación del correlador contextual.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "ContextualCorrelator.hpp"
#include <sstream>

ContextualCorrelator::ContextualCorrelator(const std::string& dbPath)
    : corr(std::make_unique<PatternCorrelator>(dbPath, "")) {}

WordPattern ContextualCorrelator::makePattern(const std::vector<std::string>& words) const {
    WordPattern pat;
    for (const auto& w : words) {
        pat[w] = 1.0f;
    }
    return pat;
}

void ContextualCorrelator::learnWithPreviousTwo(const std::string& text) {
    std::vector<std::string> words;
    std::stringstream ss(text);
    std::string w;
    while (ss >> w) words.push_back(w);

    for (size_t i = 0; i < words.size(); ++i) {
        const std::string& current = words[i];
        std::vector<std::string> prevWords;
        if (i >= 2) {
            prevWords.push_back(words[i-2]);
            prevWords.push_back(words[i-1]);
        } else if (i == 1) {
            prevWords.push_back(words[i-1]);
        }
        if (prevWords.empty()) continue;

        WordPattern prevPat = makePattern(prevWords);
        if (i + 1 < words.size()) {
            WordPattern nextPat = makePattern({words[i+1]});
            corr->record(current, prevPat, nextPat, 1.0f);
        }
    }
}

void ContextualCorrelator::learnNextWordDirect(const std::string& text) {
    std::vector<std::string> words;
    std::stringstream ss(text);
    std::string w;
    while (ss >> w) words.push_back(w);

    for (size_t i = 0; i + 1 < words.size(); ++i) {
        const std::string& current = words[i];
        WordPattern prevPat = {{"__NO_CONTEXT__", 1.0f}};
        WordPattern nextPat = makePattern({words[i+1]});
        corr->record(current, prevPat, nextPat, 1.0f);
    }
}

bool ContextualCorrelator::queryNext(const std::string& current,
                                     const std::vector<std::string>& previousWords,
                                     std::vector<std::pair<WordPattern, double>>& outcomes) {
    WordPattern prevPat = makePattern(previousWords);
    return corr->query(current, prevPat, outcomes);
}

bool ContextualCorrelator::queryNextWithTwoPrev(const std::string& current,
                                                const std::string& prev1,
                                                const std::string& prev2,
                                                std::vector<std::pair<WordPattern, double>>& outcomes) {
    return queryNext(current, {prev2, prev1}, outcomes);
}

bool ContextualCorrelator::queryNextWithOnePrev(const std::string& current,
                                                const std::string& prev,
                                                std::vector<std::pair<WordPattern, double>>& outcomes) {
    return queryNext(current, {prev}, outcomes);
}
