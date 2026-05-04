/**==============================================================================
    Admin8.2.1 - TagStats.cpp
    Proposito: Implementación de estadísticas de etiquetas.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "TagStats.hpp"
#include "../db/DatabaseManager.hpp"
#include <sqlite3.h>
#include <algorithm>
#include <iostream>
#include <map>

namespace TagStats {

    static std::string patternDbPath_;
    constexpr float SMOOTHING_K = 0.001f;
    constexpr int NUM_TAGS = static_cast<int>(TipoPalabra::INDEFINIDO) + 1;

    void setDatabasePath(const std::string& path) {
        patternDbPath_ = path;
        DatabaseManager::instance().init(patternDbPath_);
    }

    void initializeTables() {
        sqlite3* db = DatabaseManager::instance().getHandle(patternDbPath_);
        if (!db) return;

        const char* sql = R"(
            CREATE TABLE IF NOT EXISTS tag_unigrams (
                prev INTEGER NOT NULL,
                curr INTEGER NOT NULL,
                count INTEGER NOT NULL DEFAULT 0,
                PRIMARY KEY (prev, curr)
            );
            CREATE TABLE IF NOT EXISTS tag_bigrams (
                prev2 INTEGER NOT NULL,
                prev INTEGER NOT NULL,
                curr INTEGER NOT NULL,
                count INTEGER NOT NULL DEFAULT 0,
                PRIMARY KEY (prev2, prev, curr)
            );
            CREATE TABLE IF NOT EXISTS tag_trigrams (
                prev INTEGER NOT NULL,
                curr INTEGER NOT NULL,
                next INTEGER NOT NULL,
                count INTEGER NOT NULL DEFAULT 0,
                PRIMARY KEY (prev, curr, next)
            );
        )";
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "Error creando tablas TagStats: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }

    // Auxiliar para ejecutar INSERT OR UPDATE
    static void updateCount(const char* table, const std::vector<int>& keys, int inc) {
        sqlite3* db = DatabaseManager::instance().getHandle(patternDbPath_);
        if (!db) return;

        std::string sql = "INSERT INTO " + std::string(table) + " (";
        for (size_t i = 0; i < keys.size(); ++i) {
            if (i > 0) sql += ",";
            sql += (i == 0) ? "prev" : (i == 1) ? "curr" : "next";
        }
        sql += ") VALUES (";
        for (size_t i = 0; i < keys.size(); ++i) {
            if (i > 0) sql += ",";
            sql += "?";
        }
        sql += ") ON CONFLICT(";
        for (size_t i = 0; i < keys.size(); ++i) {
            if (i > 0) sql += ",";
            sql += (i == 0) ? "prev" : (i == 1) ? "curr" : "next";
        }
        sql += ") DO UPDATE SET count = count + ?";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Error preparando updateCount: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        for (size_t i = 0; i < keys.size(); ++i) {
            sqlite3_bind_int(stmt, static_cast<int>(i+1), keys[i]);
        }
        sqlite3_bind_int(stmt, static_cast<int>(keys.size()+1), inc);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void updateUnigram(TipoPalabra prev, TipoPalabra curr, int inc) {
        sqlite3* db = DatabaseManager::instance().getHandle(patternDbPath_);
        if (!db) return;
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO tag_unigrams (prev, curr, count) VALUES (?,?,?) "
                          "ON CONFLICT(prev,curr) DO UPDATE SET count = count + ?";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, static_cast<int>(prev));
        sqlite3_bind_int(stmt, 2, static_cast<int>(curr));
        sqlite3_bind_int(stmt, 3, inc);
        sqlite3_bind_int(stmt, 4, inc);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void updateBigram(TipoPalabra prev2, TipoPalabra prev, TipoPalabra curr, int inc) {
        sqlite3* db = DatabaseManager::instance().getHandle(patternDbPath_);
        if (!db) return;
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO tag_bigrams (prev2, prev, curr, count) VALUES (?,?,?,?) "
                          "ON CONFLICT(prev2,prev,curr) DO UPDATE SET count = count + ?";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, static_cast<int>(prev2));
        sqlite3_bind_int(stmt, 2, static_cast<int>(prev));
        sqlite3_bind_int(stmt, 3, static_cast<int>(curr));
        sqlite3_bind_int(stmt, 4, inc);
        sqlite3_bind_int(stmt, 5, inc);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void updateTrigram(TipoPalabra prev, TipoPalabra curr, TipoPalabra next, int inc) {
        sqlite3* db = DatabaseManager::instance().getHandle(patternDbPath_);
        if (!db) return;
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO tag_trigrams (prev, curr, next, count) VALUES (?,?,?,?) "
                          "ON CONFLICT(prev,curr,next) DO UPDATE SET count = count + ?";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, static_cast<int>(prev));
        sqlite3_bind_int(stmt, 2, static_cast<int>(curr));
        sqlite3_bind_int(stmt, 3, static_cast<int>(next));
        sqlite3_bind_int(stmt, 4, inc);
        sqlite3_bind_int(stmt, 5, inc);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    static std::vector<std::pair<TipoPalabra, float>> getProbs(const std::string& table, const std::vector<int>& keys) {
        std::vector<std::pair<TipoPalabra, float>> result;
        sqlite3* db = DatabaseManager::instance().getHandle(patternDbPath_);
        if (!db) return result;

        std::string sql = "SELECT ";
        if (table == "tag_unigrams") sql += "curr, count FROM tag_unigrams WHERE prev = ?";
        else if (table == "tag_bigrams") sql += "curr, count FROM tag_bigrams WHERE prev2 = ? AND prev = ?";
        else sql += "curr, count FROM tag_trigrams WHERE prev = ? AND next = ?";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Error preparando getProbs: " << sqlite3_errmsg(db) << std::endl;
            return result;
        }
        for (size_t i = 0; i < keys.size(); ++i) {
            sqlite3_bind_int(stmt, static_cast<int>(i+1), keys[i]);
        }
        int total = 0;
        std::map<int, int> counts;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int tag = sqlite3_column_int(stmt, 0);
            int cnt = sqlite3_column_int(stmt, 1);
            counts[tag] = cnt;
            total += cnt;
        }
        sqlite3_finalize(stmt);

        for (int t = 0; t < NUM_TAGS; ++t) {
            int cnt = counts[t];
            float prob = (cnt + SMOOTHING_K) / (total + SMOOTHING_K * NUM_TAGS);
            if (prob > 0.0f) {
                result.emplace_back(static_cast<TipoPalabra>(t), prob);
            }
        }
        std::sort(result.begin(), result.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        return result;
    }

    std::vector<std::pair<TipoPalabra, float>> getUnigramProbs(TipoPalabra prev) {
        return getProbs("tag_unigrams", {static_cast<int>(prev)});
    }

    std::vector<std::pair<TipoPalabra, float>> getBigramProbs(TipoPalabra prev2, TipoPalabra prev) {
        return getProbs("tag_bigrams", {static_cast<int>(prev2), static_cast<int>(prev)});
    }

    std::vector<std::pair<TipoPalabra, float>> getTrigramProbs(TipoPalabra prev, TipoPalabra next) {
        return getProbs("tag_trigrams", {static_cast<int>(prev), static_cast<int>(next)});
    }

    // Carga de datos estáticos iniciales (adaptada a la nueva estructura)
    void loadDefaultFromStatic() {
        sqlite3* db = DatabaseManager::instance().getHandle(patternDbPath_);
        if (!db) return;

        // Verificar si ya hay datos
        sqlite3_stmt* check;
        const char* check_sql = "SELECT COUNT(*) FROM tag_unigrams";
        sqlite3_prepare_v2(db, check_sql, -1, &check, nullptr);
        int count = 0;
        if (sqlite3_step(check) == SQLITE_ROW) count = sqlite3_column_int(check, 0);
        sqlite3_finalize(check);
        if (count > 0) return;

        updateUnigram(TipoPalabra::ART, TipoPalabra::ADJT, 80);
        updateUnigram(TipoPalabra::ART, TipoPalabra::SUST, 88);
        updateUnigram(TipoPalabra::ART, TipoPalabra::NUM, 85);
        updateUnigram(TipoPalabra::ART, TipoPalabra::PRON, 75);

        updateUnigram(TipoPalabra::PREP, TipoPalabra::SUST, 45);
        updateUnigram(TipoPalabra::PREP, TipoPalabra::PRON, 85);

        updateUnigram(TipoPalabra::SUST, TipoPalabra::VERB, 45);
        updateUnigram(TipoPalabra::SUST, TipoPalabra::ADJT, 70);

        updateUnigram(TipoPalabra::VERB, TipoPalabra::ADV, 40);
        updateUnigram(TipoPalabra::VERB, TipoPalabra::PRON, 70);

        updateUnigram(TipoPalabra::ADV, TipoPalabra::ADJT, 65);
        updateUnigram(TipoPalabra::ADV, TipoPalabra::VERB, 75);

        updateUnigram(TipoPalabra::PRON, TipoPalabra::VERB, 80);

        updateUnigram(TipoPalabra::CONJ, TipoPalabra::PRON, 90);

        updateUnigram(TipoPalabra::NUM, TipoPalabra::SUST, 85);
        updateUnigram(TipoPalabra::NUM, TipoPalabra::ADJT, 70);

        updateUnigram(TipoPalabra::ADJT, TipoPalabra::SUST, 60);

        updateUnigram(TipoPalabra::RELT, TipoPalabra::VERB, 50);

        updateUnigram(TipoPalabra::DEMS, TipoPalabra::SUST, 80);

        // Bigramas (prev2, prev, curr) – dos anteriores → actual
        updateBigram(TipoPalabra::ART, TipoPalabra::SUST, TipoPalabra::ADJT, 75);
        updateBigram(TipoPalabra::ART, TipoPalabra::SUST, TipoPalabra::NUM, 15);
        updateBigram(TipoPalabra::ART, TipoPalabra::ADJT, TipoPalabra::SUST, 88);

        updateBigram(TipoPalabra::PREP, TipoPalabra::ART, TipoPalabra::SUST, 75);
        updateBigram(TipoPalabra::PREP, TipoPalabra::SUST, TipoPalabra::VERB, 45);

        updateBigram(TipoPalabra::VERB, TipoPalabra::ADV, TipoPalabra::ADJT, 45);
        updateBigram(TipoPalabra::VERB, TipoPalabra::PRON, TipoPalabra::VERB, 70);

        updateBigram(TipoPalabra::SUST, TipoPalabra::VERB, TipoPalabra::ADV, 50);
        updateBigram(TipoPalabra::SUST, TipoPalabra::ADJT, TipoPalabra::VERB, 60);

        updateBigram(TipoPalabra::ADV, TipoPalabra::VERB, TipoPalabra::ADJT, 65);

        updateBigram(TipoPalabra::PRON, TipoPalabra::VERB, TipoPalabra::ADV, 50);

        updateBigram(TipoPalabra::CONJ, TipoPalabra::PRON, TipoPalabra::VERB, 80);

        updateBigram(TipoPalabra::NUM, TipoPalabra::SUST, TipoPalabra::ADJT, 65);

        updateBigram(TipoPalabra::PREG, TipoPalabra::VERB, TipoPalabra::ADV, 60);

        // Trigramas (prev, curr, next)
        updateTrigram(TipoPalabra::ART, TipoPalabra::SUST, TipoPalabra::ADJT, 75);
        updateTrigram(TipoPalabra::ART, TipoPalabra::SUST, TipoPalabra::NUM, 15);
        updateTrigram(TipoPalabra::ART, TipoPalabra::SUST, TipoPalabra::PRON, 5);
        updateTrigram(TipoPalabra::ART, TipoPalabra::SUST, TipoPalabra::DEMS, 2);

        updateTrigram(TipoPalabra::ART, TipoPalabra::ADJT, TipoPalabra::SUST, 88);
        updateTrigram(TipoPalabra::ART, TipoPalabra::ADJT, TipoPalabra::NUM, 6);
        updateTrigram(TipoPalabra::ART, TipoPalabra::ADJT, TipoPalabra::ADV, 3);
        updateTrigram(TipoPalabra::ART, TipoPalabra::ADJT, TipoPalabra::CONJ, 2);
        updateTrigram(TipoPalabra::ART, TipoPalabra::ADJT, TipoPalabra::PRON, 1);

        updateTrigram(TipoPalabra::ART, TipoPalabra::VERB, TipoPalabra::SUST, 80);
        updateTrigram(TipoPalabra::ART, TipoPalabra::VERB, TipoPalabra::PRON, 12);
        updateTrigram(TipoPalabra::ART, TipoPalabra::VERB, TipoPalabra::ADJT, 5);
        updateTrigram(TipoPalabra::ART, TipoPalabra::VERB, TipoPalabra::NUM, 3);

        updateTrigram(TipoPalabra::ART, TipoPalabra::PRON, TipoPalabra::SUST, 70);
        updateTrigram(TipoPalabra::ART, TipoPalabra::PRON, TipoPalabra::ADJT, 20);
        updateTrigram(TipoPalabra::ART, TipoPalabra::PRON, TipoPalabra::NUM, 10);

        updateTrigram(TipoPalabra::PREP, TipoPalabra::SUST, TipoPalabra::ART, 75);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::SUST, TipoPalabra::NUM, 10);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::SUST, TipoPalabra::PRON, 8);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::SUST, TipoPalabra::DEMS, 5);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::SUST, TipoPalabra::ADJT, 2);

        updateTrigram(TipoPalabra::PREP, TipoPalabra::VERB, TipoPalabra::ART, 50);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::VERB, TipoPalabra::PRON, 25);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::VERB, TipoPalabra::SUST, 15);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::VERB, TipoPalabra::DEMS, 5);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::VERB, TipoPalabra::NUM, 5);

        updateTrigram(TipoPalabra::PREP, TipoPalabra::PRON, TipoPalabra::VERB, 70);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::PRON, TipoPalabra::PREP, 15);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::PRON, TipoPalabra::ADV, 10);
        updateTrigram(TipoPalabra::PREP, TipoPalabra::PRON, TipoPalabra::CONJ, 5);

        updateTrigram(TipoPalabra::VERB, TipoPalabra::VERB, TipoPalabra::ADV, 50);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::VERB, TipoPalabra::CONJ, 25);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::VERB, TipoPalabra::PRON, 15);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::VERB, TipoPalabra::PREP, 10);

        updateTrigram(TipoPalabra::VERB, TipoPalabra::ADJT, TipoPalabra::ADV, 55);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::ADJT, TipoPalabra::VERB, 20);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::ADJT, TipoPalabra::PRON, 15);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::ADJT, TipoPalabra::PREP, 10);

        updateTrigram(TipoPalabra::VERB, TipoPalabra::ADV, TipoPalabra::ADJT, 45);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::ADV, TipoPalabra::VERB, 25);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::ADV, TipoPalabra::PREP, 15);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::ADV, TipoPalabra::PRON, 10);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::ADV, TipoPalabra::CONJ, 5);

        updateTrigram(TipoPalabra::VERB, TipoPalabra::SUST, TipoPalabra::ART, 50);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::SUST, TipoPalabra::PREP, 25);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::SUST, TipoPalabra::PRON, 15);
        updateTrigram(TipoPalabra::VERB, TipoPalabra::SUST, TipoPalabra::ADV, 10);

        updateTrigram(TipoPalabra::SUST, TipoPalabra::VERB, TipoPalabra::ADJT, 55);
        updateTrigram(TipoPalabra::SUST, TipoPalabra::VERB, TipoPalabra::PREP, 25);
        updateTrigram(TipoPalabra::SUST, TipoPalabra::VERB, TipoPalabra::PRON, 10);
        updateTrigram(TipoPalabra::SUST, TipoPalabra::VERB, TipoPalabra::CONJ, 7);
        updateTrigram(TipoPalabra::SUST, TipoPalabra::VERB, TipoPalabra::ADV, 3);

        updateTrigram(TipoPalabra::SUST, TipoPalabra::ADJT, TipoPalabra::VERB, 60);
        updateTrigram(TipoPalabra::SUST, TipoPalabra::ADJT, TipoPalabra::PREP, 30);
        updateTrigram(TipoPalabra::SUST, TipoPalabra::ADJT, TipoPalabra::CONJ, 15);
        updateTrigram(TipoPalabra::SUST, TipoPalabra::ADJT, TipoPalabra::ADV, 5);

        updateTrigram(TipoPalabra::ADV, TipoPalabra::ADV, TipoPalabra::CONJ, 45);
        updateTrigram(TipoPalabra::ADV, TipoPalabra::ADV, TipoPalabra::ADJT, 30);
        updateTrigram(TipoPalabra::ADV, TipoPalabra::ADV, TipoPalabra::VERB, 15);
        updateTrigram(TipoPalabra::ADV, TipoPalabra::ADV, TipoPalabra::PREP, 10);

        updateTrigram(TipoPalabra::ADV, TipoPalabra::VERB, TipoPalabra::ADJT, 65);
        updateTrigram(TipoPalabra::ADV, TipoPalabra::VERB, TipoPalabra::ADV, 15);
        updateTrigram(TipoPalabra::ADV, TipoPalabra::VERB, TipoPalabra::PREP, 10);
        updateTrigram(TipoPalabra::ADV, TipoPalabra::VERB, TipoPalabra::CONJ, 10);

        updateTrigram(TipoPalabra::ADV, TipoPalabra::ADJT, TipoPalabra::SUST, 60);
        updateTrigram(TipoPalabra::ADV, TipoPalabra::ADJT, TipoPalabra::VERB, 25);
        updateTrigram(TipoPalabra::ADV, TipoPalabra::ADJT, TipoPalabra::ADV, 10);
        updateTrigram(TipoPalabra::ADV, TipoPalabra::ADJT, TipoPalabra::PREP, 5);

        updateTrigram(TipoPalabra::PRON, TipoPalabra::PRON, TipoPalabra::VERB, 85);
        updateTrigram(TipoPalabra::PRON, TipoPalabra::PRON, TipoPalabra::ADV, 8);
        updateTrigram(TipoPalabra::PRON, TipoPalabra::PRON, TipoPalabra::CONJ, 5);
        updateTrigram(TipoPalabra::PRON, TipoPalabra::PRON, TipoPalabra::PREP, 2);
        updateTrigram(TipoPalabra::PRON, TipoPalabra::VERB, TipoPalabra::ADV, 50);
        updateTrigram(TipoPalabra::PRON, TipoPalabra::VERB, TipoPalabra::PREP, 25);
        updateTrigram(TipoPalabra::PRON, TipoPalabra::VERB, TipoPalabra::CONJ, 15);
        updateTrigram(TipoPalabra::PRON, TipoPalabra::VERB, TipoPalabra::PRON, 10);

        updateTrigram(TipoPalabra::CONJ, TipoPalabra::SUST, TipoPalabra::VERB, 60);
        updateTrigram(TipoPalabra::CONJ, TipoPalabra::SUST, TipoPalabra::ADJT, 20);
        updateTrigram(TipoPalabra::CONJ, TipoPalabra::SUST, TipoPalabra::PREP, 10);
        updateTrigram(TipoPalabra::CONJ, TipoPalabra::SUST, TipoPalabra::ADV, 10);

        updateTrigram(TipoPalabra::ADJT, TipoPalabra::SUST, TipoPalabra::ADV, 55);
        updateTrigram(TipoPalabra::ADJT, TipoPalabra::SUST, TipoPalabra::CUANT, 20);
        updateTrigram(TipoPalabra::ADJT, TipoPalabra::SUST, TipoPalabra::VERB, 15);
        updateTrigram(TipoPalabra::ADJT, TipoPalabra::SUST, TipoPalabra::CONJ, 10);
        updateTrigram(TipoPalabra::ADJT, TipoPalabra::VERB, TipoPalabra::ADV, 60);
        updateTrigram(TipoPalabra::ADJT, TipoPalabra::VERB, TipoPalabra::PREP, 20);
        updateTrigram(TipoPalabra::ADJT, TipoPalabra::VERB, TipoPalabra::CONJ, 15);
        updateTrigram(TipoPalabra::ADJT, TipoPalabra::VERB, TipoPalabra::PRON, 5);

        updateTrigram(TipoPalabra::NUM, TipoPalabra::SUST, TipoPalabra::ADJT, 65);
        updateTrigram(TipoPalabra::NUM, TipoPalabra::SUST, TipoPalabra::ART, 20);
        updateTrigram(TipoPalabra::NUM, TipoPalabra::SUST, TipoPalabra::NUM, 10);
        updateTrigram(TipoPalabra::NUM, TipoPalabra::SUST, TipoPalabra::PREP, 5);
        updateTrigram(TipoPalabra::NUM, TipoPalabra::ADJT, TipoPalabra::SUST, 80);
        updateTrigram(TipoPalabra::NUM, TipoPalabra::ADJT, TipoPalabra::NUM, 10);
        updateTrigram(TipoPalabra::NUM, TipoPalabra::ADJT, TipoPalabra::PREP, 10);

        updateTrigram(TipoPalabra::PREG, TipoPalabra::SUST, TipoPalabra::ADJT, 75);
        updateTrigram(TipoPalabra::PREG, TipoPalabra::SUST, TipoPalabra::NUM, 15);
        updateTrigram(TipoPalabra::PREG, TipoPalabra::SUST, TipoPalabra::VERB, 10);
        updateTrigram(TipoPalabra::PREG, TipoPalabra::VERB, TipoPalabra::ADV, 60);
        updateTrigram(TipoPalabra::PREG, TipoPalabra::VERB, TipoPalabra::SUST, 30);
        updateTrigram(TipoPalabra::PREG, TipoPalabra::VERB, TipoPalabra::ADJT, 10);
    }
}
