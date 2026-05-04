/**==============================================================================
    Admin8.2.1 - PatternCorrelator.cpp
    Proposito: Implementación del correlador de patrones.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "PatternCorrelator.hpp"
#include <iostream>
#include <sstream>

PatternCorrelator::PatternCorrelator(const std::string& dbPath, const std::string& suffix)
    : tableSuffix(suffix) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::string msg = "No se pudo abrir la BD del correlator: ";
        msg += sqlite3_errmsg(db);
        throw std::runtime_error(msg);
    }

    // Crear tablas con sufijo
    std::string createSQL =
        "CREATE TABLE IF NOT EXISTS " + getWordsTable() + " ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "word TEXT UNIQUE NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS " + getPatternsTable() + " ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "serialized TEXT UNIQUE NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS " + getCorrelationsTable() + " ("
        "current_word_id INTEGER NOT NULL,"
        "prev_pattern_id INTEGER NOT NULL,"
        "next_pattern_id INTEGER NOT NULL,"
        "total_weight REAL NOT NULL,"
        "count INTEGER NOT NULL,"
        "PRIMARY KEY (current_word_id, prev_pattern_id, next_pattern_id),"
        "FOREIGN KEY(current_word_id) REFERENCES " + getWordsTable() + "(id),"
        "FOREIGN KEY(prev_pattern_id) REFERENCES " + getPatternsTable() + "(id),"
        "FOREIGN KEY(next_pattern_id) REFERENCES " + getPatternsTable() + "(id)"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, createSQL.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creando tablas: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    prepareStatements();
}

PatternCorrelator::~PatternCorrelator() {
    if (stmtGetWordId) sqlite3_finalize(stmtGetWordId);
    if (stmtInsertWord) sqlite3_finalize(stmtInsertWord);
    if (stmtGetPatternId) sqlite3_finalize(stmtGetPatternId);
    if (stmtInsertPattern) sqlite3_finalize(stmtInsertPattern);
    if (stmtFindCorrelation) sqlite3_finalize(stmtFindCorrelation);
    if (stmtUpdateCorrelation) sqlite3_finalize(stmtUpdateCorrelation);
    if (stmtInsertCorrelation) sqlite3_finalize(stmtInsertCorrelation);
    if (stmtGetTotalWeight) sqlite3_finalize(stmtGetTotalWeight);
    if (stmtGetNextPatterns) sqlite3_finalize(stmtGetNextPatterns);
    if (db) sqlite3_close(db);
}

void PatternCorrelator::prepareStatements() {
    std::string sql;

    sql = "SELECT id FROM " + getWordsTable() + " WHERE word = ?";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmtGetWordId, nullptr);

    sql = "INSERT INTO " + getWordsTable() + " (word) VALUES (?)";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmtInsertWord, nullptr);

    sql = "SELECT id FROM " + getPatternsTable() + " WHERE serialized = ?";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmtGetPatternId, nullptr);

    sql = "INSERT INTO " + getPatternsTable() + " (serialized) VALUES (?)";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmtInsertPattern, nullptr);

    sql = "SELECT total_weight, count FROM " + getCorrelationsTable() +
          " WHERE current_word_id=? AND prev_pattern_id=? AND next_pattern_id=?";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmtFindCorrelation, nullptr);

    sql = "UPDATE " + getCorrelationsTable() +
          " SET total_weight=?, count=? WHERE current_word_id=? AND prev_pattern_id=? AND next_pattern_id=?";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmtUpdateCorrelation, nullptr);

    sql = "INSERT INTO " + getCorrelationsTable() +
          " (current_word_id, prev_pattern_id, next_pattern_id, total_weight, count) VALUES (?,?,?,?,?)";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmtInsertCorrelation, nullptr);

    sql = "SELECT SUM(total_weight) FROM " + getCorrelationsTable() +
          " WHERE current_word_id=? AND prev_pattern_id=?";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmtGetTotalWeight, nullptr);

    sql = "SELECT next_pattern_id, total_weight FROM " + getCorrelationsTable() +
          " WHERE current_word_id=? AND prev_pattern_id=?";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmtGetNextPatterns, nullptr);
}

int PatternCorrelator::getWordId(const std::string& word) {
    auto it = wordToId.find(word);
    if (it != wordToId.end()) return it->second;

    sqlite3_bind_text(stmtGetWordId, 1, word.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmtGetWordId) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmtGetWordId, 0);
        sqlite3_reset(stmtGetWordId);
        wordToId[word] = id;
        idToWord[id] = word;
        return id;
    }
    sqlite3_reset(stmtGetWordId);

    sqlite3_bind_text(stmtInsertWord, 1, word.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmtInsertWord) != SQLITE_DONE) {
        std::cerr << "Error insertando palabra: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_reset(stmtInsertWord);
        return -1;
    }
    int newId = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_reset(stmtInsertWord);
    wordToId[word] = newId;
    idToWord[newId] = word;
    return newId;
}

int PatternCorrelator::getPatternId(const WordPattern& pattern) {
    std::string serialized = serializePattern(pattern);
    auto it = patternSerializedToId.find(serialized);
    if (it != patternSerializedToId.end()) return it->second;

    sqlite3_bind_text(stmtGetPatternId, 1, serialized.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmtGetPatternId) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmtGetPatternId, 0);
        sqlite3_reset(stmtGetPatternId);
        patternSerializedToId[serialized] = id;
        idToPattern[id] = pattern;
        return id;
    }
    sqlite3_reset(stmtGetPatternId);

    sqlite3_bind_text(stmtInsertPattern, 1, serialized.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmtInsertPattern) != SQLITE_DONE) {
        std::cerr << "Error insertando patrón: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_reset(stmtInsertPattern);
        return -1;
    }
    int newId = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_reset(stmtInsertPattern);
    patternSerializedToId[serialized] = newId;
    idToPattern[newId] = pattern;
    return newId;
}

void PatternCorrelator::updateCorrelation(int currWordId, int prevPatternId, int nextPatternId, float weight) {
    sqlite3_bind_int(stmtFindCorrelation, 1, currWordId);
    sqlite3_bind_int(stmtFindCorrelation, 2, prevPatternId);
    sqlite3_bind_int(stmtFindCorrelation, 3, nextPatternId);
    int stepResult = sqlite3_step(stmtFindCorrelation);

    if (stepResult == SQLITE_ROW) {
        double oldWeight = sqlite3_column_double(stmtFindCorrelation, 0);
        int oldCount = sqlite3_column_int(stmtFindCorrelation, 1);
        sqlite3_reset(stmtFindCorrelation);

        sqlite3_bind_double(stmtUpdateCorrelation, 1, oldWeight + weight);
        sqlite3_bind_int(stmtUpdateCorrelation, 2, oldCount + 1);
        sqlite3_bind_int(stmtUpdateCorrelation, 3, currWordId);
        sqlite3_bind_int(stmtUpdateCorrelation, 4, prevPatternId);
        sqlite3_bind_int(stmtUpdateCorrelation, 5, nextPatternId);
        if (sqlite3_step(stmtUpdateCorrelation) != SQLITE_DONE) {
            std::cerr << "Error actualizando correlación: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmtUpdateCorrelation);
    } else {
        sqlite3_reset(stmtFindCorrelation);
        sqlite3_bind_int(stmtInsertCorrelation, 1, currWordId);
        sqlite3_bind_int(stmtInsertCorrelation, 2, prevPatternId);
        sqlite3_bind_int(stmtInsertCorrelation, 3, nextPatternId);
        sqlite3_bind_double(stmtInsertCorrelation, 4, weight);
        sqlite3_bind_int(stmtInsertCorrelation, 5, 1);
        if (sqlite3_step(stmtInsertCorrelation) != SQLITE_DONE) {
            std::cerr << "Error insertando correlación: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmtInsertCorrelation);
    }
}

void PatternCorrelator::record(const std::string& current, const WordPattern& prevPattern, const WordPattern& nextPattern, float weight) {
    int currId = getWordId(current);
    int prevId = getPatternId(prevPattern);
    int nextId = getPatternId(nextPattern);
    if (currId == -1 || prevId == -1 || nextId == -1) return;
    updateCorrelation(currId, prevId, nextId, weight);
}

bool PatternCorrelator::query(const std::string& current, const WordPattern& prevPattern,
                              std::vector<std::pair<WordPattern, double>>& outcomes) {
    int currId = getWordId(current);
    int prevId = getPatternId(prevPattern);
    if (currId == -1 || prevId == -1) return false;

    sqlite3_bind_int(stmtGetTotalWeight, 1, currId);
    sqlite3_bind_int(stmtGetTotalWeight, 2, prevId);
    double totalWeight = 0.0;
    if (sqlite3_step(stmtGetTotalWeight) == SQLITE_ROW) {
        totalWeight = sqlite3_column_double(stmtGetTotalWeight, 0);
    }
    sqlite3_reset(stmtGetTotalWeight);
    if (totalWeight == 0.0) return false;

    sqlite3_bind_int(stmtGetNextPatterns, 1, currId);
    sqlite3_bind_int(stmtGetNextPatterns, 2, prevId);
    while (sqlite3_step(stmtGetNextPatterns) == SQLITE_ROW) {
        int nextId = sqlite3_column_int(stmtGetNextPatterns, 0);
        double weight = sqlite3_column_double(stmtGetNextPatterns, 1);
        double prob = weight / totalWeight;
        auto it = idToPattern.find(nextId);
        if (it != idToPattern.end()) {
            outcomes.emplace_back(it->second, prob);
        } else {
            // Cargar patrón desde la BD, usando una cadena no temporal para evitar puntero colgante
            std::string sqlStr = "SELECT serialized FROM " + getPatternsTable() + " WHERE id = ?";
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db, sqlStr.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                std::cerr << "Error preparando consulta de patrón: " << sqlite3_errmsg(db) << std::endl;
                continue;
            }
            sqlite3_bind_int(stmt, 1, nextId);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                const char* serialized = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                WordPattern pat = deserializePattern(serialized);
                outcomes.emplace_back(pat, prob);
                idToPattern[nextId] = pat;
            }
            sqlite3_finalize(stmt);
        }
    }
    sqlite3_reset(stmtGetNextPatterns);
    return !outcomes.empty();
}

void PatternCorrelator::learnFromText(const std::string& text, size_t windowSize) {
    if (windowSize != 1) {
        std::cerr << "Advertencia: learnFromText solo soporta windowSize=1. Usando valor 1." << std::endl;
    }
    std::vector<std::string> words;
    std::stringstream ss(text);
    std::string w;
    while (ss >> w) words.push_back(w);

    for (size_t i = 0; i + 2 < words.size(); ++i) {
        WordPattern prevPat = {{words[i], 1.0f}};
        WordPattern nextPat = {{words[i+2], 1.0f}};
        record(words[i+1], prevPat, nextPat);
    }
}
