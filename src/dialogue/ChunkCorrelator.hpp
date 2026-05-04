#ifndef CHUNK_CORRELATOR_HPP
#define CHUNK_CORRELATOR_HPP

#include "PatternCorrelator.hpp"
#include "../utils/Chunker.hpp"
#include <memory>
#include <vector>
#include <string>
#include <utility>

class ChunkCorrelator {
public:
    // Recibe la ruta de la base de datos
    explicit ChunkCorrelator(const std::string& dbPath);

    void learnWithPreviousTwo(const std::vector<std::string>& chunks);
    void learnNextChunkDirect(const std::vector<std::string>& chunks);
    void learnFromClassifiedSentence(const std::vector<Word>& sentence);

    bool queryNext(const std::string& current,
                   const std::vector<std::string>& previousChunks,
                   std::vector<std::pair<WordPattern, double>>& outcomes);

    bool queryNextWithOnePrev(const std::string& current,
                              const std::string& prev,
                              std::vector<std::pair<WordPattern, double>>& outcomes);

    bool queryNextWithTwoPrev(const std::string& current,
                              const std::string& prev1,
                              const std::string& prev2,
                              std::vector<std::pair<WordPattern, double>>& outcomes);

private:
    std::unique_ptr<PatternCorrelator> corr;  // usa sufijo "_chunk"
    WordPattern makePattern(const std::vector<std::string>& chunks) const;
};

#endif
