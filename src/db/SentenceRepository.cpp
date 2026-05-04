/**==============================================================================
    Admin8.2.1 - SentenceRepository.cpp
    Proposito: Implementación de persistencia de oraciones usando SQLite.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "SentenceRepository.hpp"
#include "DatabaseManager.hpp"
#include <iostream>
#include <algorithm>

std::string SentenceRepository::dbPath_;

void SentenceRepository::setDatabasePath(const std::string& path) {
    dbPath_ = path;
    DatabaseManager::instance().init(dbPath_);
}

std::string SentenceRepository::getDatabasePath() {
    return dbPath_;
}

void SentenceRepository::initializeTables() {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return;

    const char* sql =
        "CREATE TABLE IF NOT EXISTS oraciones ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "clave_palabra TEXT, tipo_clave INTEGER, frecuencia REAL,"
        "tiempo_oracion INTEGER, num_bloques INTEGER"
        ");"
        "CREATE TABLE IF NOT EXISTS bloques ("
        "oracion_id INTEGER, posicion INTEGER, bloque_texto TEXT, tipo_palabra INTEGER,"
        "FOREIGN KEY(oracion_id) REFERENCES oraciones(id)"
        ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Error creando tablas de SentenceRepository: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

void SentenceRepository::save(Sentence& sentence) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return;

    int newId = sentence.getId();

    if (newId > 0) {
        // UPDATE
        const char* sql_upd = "UPDATE oraciones SET clave_palabra=?, tipo_clave=?, frecuencia=?, tiempo_oracion=?, num_bloques=? WHERE id=?";
        sqlite3_stmt* stmt = nullptr;
        if (!prepareSqlStatement(db, sql_upd, &stmt)) return;
        sqlite3_bind_text(stmt, 1, sentence.getKey().text.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, static_cast<int>(sentence.getKey().type));
        sqlite3_bind_double(stmt, 3, sentence.getFrequency());
        sqlite3_bind_int(stmt, 4, static_cast<int>(sentence.getTense()));
        sqlite3_bind_int(stmt, 5, sentence.getNumBlocks());
        sqlite3_bind_int(stmt, 6, newId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        // Eliminar bloques antiguos
        const char* sql_del = "DELETE FROM bloques WHERE oracion_id=?";
        if (!prepareSqlStatement(db, sql_del, &stmt)) return;
        sqlite3_bind_int(stmt, 1, newId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        // INSERT
        const char* sql_ins = "INSERT INTO oraciones (clave_palabra, tipo_clave, frecuencia, tiempo_oracion, num_bloques) VALUES (?,?,?,?,?)";
        sqlite3_stmt* stmt = nullptr;
        if (!prepareSqlStatement(db, sql_ins, &stmt)) return;
        sqlite3_bind_text(stmt, 1, sentence.getKey().text.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, static_cast<int>(sentence.getKey().type));
        sqlite3_bind_double(stmt, 3, sentence.getFrequency());
        sqlite3_bind_int(stmt, 4, static_cast<int>(sentence.getTense()));
        sqlite3_bind_int(stmt, 5, sentence.getNumBlocks());
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error insertando oración: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return;
        }
        newId = static_cast<int>(sqlite3_last_insert_rowid(db));
        sqlite3_finalize(stmt);
        sentence.setId(newId);
    }

    // Insertar bloques
    const char* sql_bloque = "INSERT INTO bloques (oracion_id, posicion, bloque_texto, tipo_palabra) VALUES (?,?,?,?)";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql_bloque, &stmt)) return;
    const auto& blocks = sentence.getBlocks();
    for (size_t i = 0; i < blocks.size(); ++i) {
        sqlite3_bind_int(stmt, 1, newId);
        sqlite3_bind_int(stmt, 2, static_cast<int>(i));
        sqlite3_bind_text(stmt, 3, blocks[i].text.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, static_cast<int>(blocks[i].type));
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);
}

Sentence SentenceRepository::loadById(int id) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    Sentence sentence;
    if (!db) return sentence;

    // Obtener cabecera
    const char* sql_oracion = "SELECT clave_palabra, tipo_clave, frecuencia, tiempo_oracion FROM oraciones WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql_oracion, &stmt)) return sentence;
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return sentence;
    }
    std::string keyWord = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    TipoPalabra keyType = static_cast<TipoPalabra>(sqlite3_column_int(stmt, 1));
    float frequency = static_cast<float>(sqlite3_column_double(stmt, 2));
    Tiempo tense = static_cast<Tiempo>(sqlite3_column_int(stmt, 3));
    sqlite3_finalize(stmt);

    // Cargar bloques
    const char* sql_bloques = "SELECT bloque_texto, tipo_palabra FROM bloques WHERE oracion_id = ? ORDER BY posicion";
    if (!prepareSqlStatement(db, sql_bloques, &stmt)) return sentence;
    sqlite3_bind_int(stmt, 1, id);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        TipoPalabra type = static_cast<TipoPalabra>(sqlite3_column_int(stmt, 1));
        sentence.addBlock(text, type);
    }
    sqlite3_finalize(stmt);

    sentence.setId(id);
    sentence.setKey({keyWord, keyType});
    sentence.setFrequency(frequency);
    sentence.setTense(tense);

    return sentence;
}

Sentence SentenceRepository::loadByKey(const std::string& keyWord, TipoPalabra keyType) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    Sentence sentence;
    if (!db) return sentence;

    const char* sql = "SELECT id FROM oraciones WHERE clave_palabra = ? AND tipo_clave = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql, &stmt)) return sentence;
    sqlite3_bind_text(stmt, 1, keyWord.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, static_cast<int>(keyType));
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return sentence;
    }
    int id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return loadById(id);
}

bool SentenceRepository::remove(int id) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return false;
    // Eliminar bloques primero (por FK)
    const char* sql_del_bloques = "DELETE FROM bloques WHERE oracion_id = ?";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql_del_bloques, &stmt)) return false;
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    const char* sql_del_oracion = "DELETE FROM oraciones WHERE id = ?";
    if (!prepareSqlStatement(db, sql_del_oracion, &stmt)) return false;
    sqlite3_bind_int(stmt, 1, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

std::vector<Sentence> SentenceRepository::loadAll() {
    std::vector<Sentence> result;
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return result;

    const char* sql = "SELECT id FROM oraciones ORDER BY id";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql, &stmt)) return result;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        result.push_back(loadById(id));
    }
    sqlite3_finalize(stmt);
    return result;
}
