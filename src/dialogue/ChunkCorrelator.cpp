/**==============================================================================
    Admin8.3.0 - ChunkCorrelator.cpp
    Propósito: Implementación del correlador de chunks, incluyendo aprendizaje
               con contexto previo y consultas para predicción.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "ChunkCorrelator.hpp"
#include "../common/types.hpp"

ChunkCorrelator::ChunkCorrelator(const std::string& dbPath)
    : corr(std::make_unique<PatternCorrelator>(dbPath, "_chunk")) {}

WordPattern ChunkCorrelator::makePattern(const std::vector<std::string>& chunks) const {
    WordPattern pat;
    for (const auto& ch : chunks) {
        pat[ch] = 1.0f;
    }
    return pat;
}

void ChunkCorrelator::learnWithPreviousTwo(const std::vector<std::string>& chunks) {
    for (size_t i = 0; i < chunks.size(); ++i) {
        const std::string& current = chunks[i];
        std::vector<std::string> prevChunks;
        if (i >= 2) {
            prevChunks.push_back(chunks[i-2]);
            prevChunks.push_back(chunks[i-1]);
        } else if (i == 1) {
            prevChunks.push_back(chunks[i-1]);
        } else {
            // Sin contexto previo: no se aprende
            continue;
        }

        WordPattern prevPat = makePattern(prevChunks);
        if (i + 1 < chunks.size()) {
            WordPattern nextPat = makePattern({chunks[i+1]});
            corr->record(current, prevPat, nextPat, 1.0f);
        }
    }
}

void ChunkCorrelator::learnNextChunkDirect(const std::vector<std::string>& chunks) {
    for (size_t i = 0; i + 1 < chunks.size(); ++i) {
        const std::string& current = chunks[i];
        WordPattern prevPat = makePattern({"__NO_CONTEXT__"});
        WordPattern nextPat = makePattern({chunks[i+1]});
        corr->record(current, prevPat, nextPat, 1.0f);
    }
}

void ChunkCorrelator::learnFromClassifiedSentence(const std::vector<Word>& sentence) {
    std::vector<std::string> chunks = Chunker::chunk(sentence);
    learnWithPreviousTwo(chunks);
}

bool ChunkCorrelator::queryNext(const std::string& current,
                                const std::vector<std::string>& previousChunks,
                                std::vector<std::pair<WordPattern, double>>& outcomes) {
    WordPattern prevPat = makePattern(previousChunks);
    return corr->query(current, prevPat, outcomes);
}

bool ChunkCorrelator::queryNextWithOnePrev(const std::string& current,
                                           const std::string& prev,
                                           std::vector<std::pair<WordPattern, double>>& outcomes) {
    return queryNext(current, {prev}, outcomes);
}

bool ChunkCorrelator::queryNextWithTwoPrev(const std::string& current,
                                           const std::string& prev1,
                                           const std::string& prev2,
                                           std::vector<std::pair<WordPattern, double>>& outcomes) {
    return queryNext(current, {prev2, prev1}, outcomes);
}
