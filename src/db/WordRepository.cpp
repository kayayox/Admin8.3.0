/**==============================================================================
    Admin8.2.1 - WordRepository.cpp
    Proposito: Implementación de persistencia de palabras usando SQLite.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "WordRepository.hpp"
#include "DatabaseManager.hpp"
#include <iostream>

std::string WordRepository::dbPath_;

void WordRepository::setDatabasePath(const std::string& path) {
    dbPath_ = path;
    DatabaseManager::instance().init(dbPath_);
}

std::string WordRepository::getDatabasePath() {
    return dbPath_;
}

void WordRepository::initializeTables() {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return;

    const char* sql =
        "CREATE TABLE IF NOT EXISTS palabras ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "palabra TEXT UNIQUE NOT NULL,"
        "significado TEXT,"
        "tipo INTEGER,"
        "cantidad INTEGER,"
        "tiempo INTEGER,"
        "genero INTEGER,"
        "grado INTEGER,"
        "persona INTEGER,"
        "confianza REAL"
        ");"
        "CREATE TABLE IF NOT EXISTS relaciones ("
        "palabra_id INTEGER, relacionada_id INTEGER, valor REAL,"
        "FOREIGN KEY(palabra_id) REFERENCES palabras(id)"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creando tablas de palabras: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

void WordRepository::save(const Word& word) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) {
        std::cerr << "Error: BD semántica no inicializada en WordRepository::save" << std::endl;
        return;
    }

    // Verificar si ya existe
    const char* sql_check = "SELECT id FROM palabras WHERE palabra = ?";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql_check, &stmt)) return;
    sqlite3_bind_text(stmt, 1, word.getPalabra().c_str(), -1, SQLITE_STATIC);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    if (exists) {
        const char* sql_upd = "UPDATE palabras SET significado=?, tipo=?, cantidad=?, tiempo=?, genero=?, grado=?, persona=?, confianza=? WHERE palabra=?";
        if (!prepareSqlStatement(db, sql_upd, &stmt)) return;
        sqlite3_bind_text(stmt, 1, word.getSignificado().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, static_cast<int>(word.getTipo()));
        sqlite3_bind_int(stmt, 3, static_cast<int>(word.getCantidad()));
        sqlite3_bind_int(stmt, 4, static_cast<int>(word.getTiempo()));
        sqlite3_bind_int(stmt, 5, static_cast<int>(word.getGenero()));
        sqlite3_bind_int(stmt, 6, static_cast<int>(word.getGrado()));
        sqlite3_bind_int(stmt, 7, static_cast<int>(word.getPersona()));
        sqlite3_bind_double(stmt, 8, word.getConfianza());
        sqlite3_bind_text(stmt, 9, word.getPalabra().c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        const char* sql_ins = "INSERT INTO palabras (palabra, significado, tipo, cantidad, tiempo, genero, grado, persona, confianza) VALUES (?,?,?,?,?,?,?,?,?)";
        if (!prepareSqlStatement(db, sql_ins, &stmt)) return;
        sqlite3_bind_text(stmt, 1, word.getPalabra().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, word.getSignificado().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, static_cast<int>(word.getTipo()));
        sqlite3_bind_int(stmt, 4, static_cast<int>(word.getCantidad()));
        sqlite3_bind_int(stmt, 5, static_cast<int>(word.getTiempo()));
        sqlite3_bind_int(stmt, 6, static_cast<int>(word.getGenero()));
        sqlite3_bind_int(stmt, 7, static_cast<int>(word.getGrado()));
        sqlite3_bind_int(stmt, 8, static_cast<int>(word.getPersona()));
        sqlite3_bind_double(stmt, 9, word.getConfianza());
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

bool WordRepository::load(const std::string& palabra, Word& outWord) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return false;

    const char* sql = "SELECT significado, tipo, cantidad, tiempo, genero, grado, persona, confianza FROM palabras WHERE palabra = ?";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql, &stmt)) return false;
    sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    outWord.setPalabra(palabra);
    outWord.setSignificado(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    outWord.setTipo(static_cast<TipoPalabra>(sqlite3_column_int(stmt, 1)));
    outWord.setCantidad(static_cast<Cantidad>(sqlite3_column_int(stmt, 2)));
    outWord.setTiempo(static_cast<Tiempo>(sqlite3_column_int(stmt, 3)));
    outWord.setGenero(static_cast<Genero>(sqlite3_column_int(stmt, 4)));
    outWord.setGrado(static_cast<Grado>(sqlite3_column_int(stmt, 5)));
    outWord.setPersona(static_cast<Persona>(sqlite3_column_int(stmt, 6)));
    outWord.setConfianza(static_cast<float>(sqlite3_column_double(stmt, 7)));

    sqlite3_finalize(stmt);
    return true;
}

bool WordRepository::exists(const std::string& palabra) {
    Word dummy;
    return load(palabra, dummy);
}

bool WordRepository::remove(const std::string& palabra) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return false;
    const char* sql = "DELETE FROM palabras WHERE palabra = ?";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql, &stmt)) return false;
    sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}
