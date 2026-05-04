/**==============================================================================
    Admin8.2.1 - PatternCorrelator.hpp
    Proposito: Correlador de patrones que almacena relaciones trigrama
               (palabra actual, patrón anterior, patrón siguiente) en su propia
               base de datos SQLite (patrones.db).
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Los patrones son mapas palabra->peso (actualmente solo una palabra).
           Esta clase maneja su propia conexión a BD.
==============================================================================*/

#ifndef PATTERN_CORRELATOR_HPP
#define PATTERN_CORRELATOR_HPP

#include "../utils/PatternUtils.hpp"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>

class PatternCorrelator {
public:
    // tableSuffix: "" para tablas normales, "_chunk" para chunks, etc.
    PatternCorrelator(const std::string& dbPath, const std::string& tableSuffix = "");
    ~PatternCorrelator();

    void record(const std::string& current, const WordPattern& prevPattern, const WordPattern& nextPattern, float weight = 1.0f);
    bool query(const std::string& current, const WordPattern& prevPattern, std::vector<std::pair<WordPattern, double>>& outcomes);
    void learnFromText(const std::string& text, size_t windowSize = 1);

private:
    sqlite3* db;
    std::string tableSuffix;

    // Nombres de tablas con sufijo
    std::string getWordsTable() const { return "words" + tableSuffix; }
    std::string getPatternsTable() const { return "patterns" + tableSuffix; }
    std::string getCorrelationsTable() const { return "correlations" + tableSuffix; }

    // Caches
    std::map<std::string, int> wordToId;
    std::map<int, std::string> idToWord;
    std::map<std::string, int> patternSerializedToId;
    std::map<int, WordPattern> idToPattern;

    // Prepared statements
    sqlite3_stmt* stmtGetWordId = nullptr;
    sqlite3_stmt* stmtInsertWord = nullptr;
    sqlite3_stmt* stmtGetPatternId = nullptr;
    sqlite3_stmt* stmtInsertPattern = nullptr;
    sqlite3_stmt* stmtFindCorrelation = nullptr;
    sqlite3_stmt* stmtUpdateCorrelation = nullptr;
    sqlite3_stmt* stmtInsertCorrelation = nullptr;
    sqlite3_stmt* stmtGetTotalWeight = nullptr;
    sqlite3_stmt* stmtGetNextPatterns = nullptr;

    int getWordId(const std::string& word);
    int getPatternId(const WordPattern& pattern);
    void updateCorrelation(int currWordId, int prevPatternId, int nextPatternId, float weight);
    void prepareStatements();  // construye las sentencias con los nombres de tabla adecuados
};

#endif
