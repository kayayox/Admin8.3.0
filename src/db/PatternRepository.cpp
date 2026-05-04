/**==============================================================================
    Admin8.2.1 - PatternRepository.cpp
    Proposito: Implementación de persistencia de patrones.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "PatternRepository.hpp"
#include "DatabaseManager.hpp"
#include <sstream>
#include <iostream>

static std::string dbPath_;

void PatternRepository::setDatabasePath(const std::string& path) {
    dbPath_ = path;
    DatabaseManager::instance().init(dbPath_);
}

std::string PatternRepository::getDatabasePath() {
    return dbPath_;
}

void PatternRepository::initializeTables() {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return;

    const char* sql = "CREATE TABLE IF NOT EXISTS patrones ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "tipo_patron INTEGER, secuencia TEXT, frecuencia REAL DEFAULT 1.0"
                      ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creando tabla patrones: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

static std::string serializeSequence(const std::vector<TipoPalabra>& seq) {
    std::stringstream ss;
    for (size_t i = 0; i < seq.size(); ++i) {
        if (i > 0) ss << ",";
        ss << static_cast<int>(seq[i]);
    }
    return ss.str();
}

static std::vector<TipoPalabra> deserializeSequence(const std::string& str) {
    std::vector<TipoPalabra> res;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            res.push_back(static_cast<TipoPalabra>(std::stoi(item)));
        }
    }
    return res;
}

void PatternRepository::save(const Pattern& pattern) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return;

    std::string seqStr = serializeSequence(pattern.sequence);
    sqlite3_stmt* stmt = nullptr;
    const char* sql_check = "SELECT id, frecuencia FROM patrones WHERE tipo_patron = ? AND secuencia = ?";
    if (!prepareSqlStatement(db, sql_check, &stmt)) return;
    sqlite3_bind_int(stmt, 1, static_cast<int>(pattern.type));
    sqlite3_bind_text(stmt, 2, seqStr.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        float freq = static_cast<float>(sqlite3_column_double(stmt, 1));
        sqlite3_finalize(stmt);
        const char* sql_upd = "UPDATE patrones SET frecuencia = ? WHERE id = ?";
        if (!prepareSqlStatement(db, sql_upd, &stmt)) return;
        sqlite3_bind_double(stmt, 1, freq + 1.0f);
        sqlite3_bind_int(stmt, 2, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        sqlite3_finalize(stmt);
        const char* sql_ins = "INSERT INTO patrones (tipo_patron, secuencia, frecuencia) VALUES (?,?,?)";
        if (!prepareSqlStatement(db, sql_ins, &stmt)) return;
        sqlite3_bind_int(stmt, 1, static_cast<int>(pattern.type));
        sqlite3_bind_text(stmt, 2, seqStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, 1.0f);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

std::optional<Pattern> PatternRepository::findExactMatch(const std::vector<TipoPalabra>& sequence,
                                                          float& outSimilitud) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return std::nullopt;

    std::string targetSeq = serializeSequence(sequence);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT tipo_patron, secuencia FROM patrones WHERE secuencia = ?";
    if (!prepareSqlStatement(db, sql, &stmt)) return std::nullopt;
    sqlite3_bind_text(stmt, 1, targetSeq.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outSimilitud = 1.0f;
        TipoPatron tp = static_cast<TipoPatron>(sqlite3_column_int(stmt, 0));
        const char* seqStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::vector<TipoPalabra> seq = deserializeSequence(seqStr);
        sqlite3_finalize(stmt);
        return Pattern(seq, tp);
    }
    sqlite3_finalize(stmt);
    outSimilitud = 0.0f;
    return std::nullopt;
}

std::vector<Pattern> PatternRepository::loadAll() {
    std::vector<Pattern> result;
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return result;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT tipo_patron, secuencia FROM patrones";
    if (!prepareSqlStatement(db, sql, &stmt)) return result;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TipoPatron tp = static_cast<TipoPatron>(sqlite3_column_int(stmt, 0));
        const char* seqStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::vector<TipoPalabra> seq = deserializeSequence(seqStr);
        result.emplace_back(seq, tp);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool PatternRepository::remove(const Pattern& pattern) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return false;
    std::string seqStr = serializeSequence(pattern.sequence);
    const char* sql = "DELETE FROM patrones WHERE tipo_patron = ? AND secuencia = ?";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql, &stmt)) return false;
    sqlite3_bind_int(stmt, 1, static_cast<int>(pattern.type));
    sqlite3_bind_text(stmt, 2, seqStr.c_str(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}
