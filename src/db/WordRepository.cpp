/**==============================================================================
    Admin8.3.0 - WordRepository.cpp
    Proposito: Implementación de persistencia de palabras usando SQLite.
    Autor: Soubhi Khayat Najjar (modificado)
    Año: 2026
==============================================================================*/

#include "WordRepository.hpp"
#include "DatabaseManager.hpp"
#include <iostream>
#include <algorithm>

std::string WordRepository::dbPath_;

// ---------------------------------------------------------------------
// Funciones auxiliares (no miembro) para manejo de errores de SQLite
// ---------------------------------------------------------------------
static bool prepareStatement(sqlite3* db, const char* sql, sqlite3_stmt** stmt) {
    int rc = sqlite3_prepare_v2(db, sql, -1, stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparando SQL: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

static int getWordId(sqlite3* db, const std::string& palabra, bool createIfMissing = true) {
    // Primero buscar ID existente
    const char* sql_select = "SELECT id FROM palabras WHERE palabra = ?";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareStatement(db, sql_select, &stmt)) return -1;
    sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);
    int id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    // Si no existe y se pide crearla, insertamos una fila mínima
    if (id == -1 && createIfMissing) {
        const char* sql_insert = "INSERT INTO palabras (palabra) VALUES (?)";
        if (!prepareStatement(db, sql_insert, &stmt)) return -1;
        sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error insertando palabra temporal: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return -1;
        }
        id = sqlite3_last_insert_rowid(db);
        sqlite3_finalize(stmt);
    }
    return id;
}

// ---------------------------------------------------------------------
// Métodos públicos
// ---------------------------------------------------------------------
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
        "FOREIGN KEY(palabra_id) REFERENCES palabras(id) ON DELETE CASCADE,"
        "FOREIGN KEY(relacionada_id) REFERENCES palabras(id) ON DELETE CASCADE,"
        "PRIMARY KEY (palabra_id, relacionada_id)"
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

    // 1. Guardar o actualizar los datos principales
    const char* sql_upsert =
        "INSERT INTO palabras (palabra, significado, tipo, cantidad, tiempo, genero, grado, persona, confianza) "
        "VALUES (?,?,?,?,?,?,?,?,?) "
        "ON CONFLICT(palabra) DO UPDATE SET "
        "significado=excluded.significado, tipo=excluded.tipo, cantidad=excluded.cantidad, "
        "tiempo=excluded.tiempo, genero=excluded.genero, grado=excluded.grado, "
        "persona=excluded.persona, confianza=excluded.confianza";

    sqlite3_stmt* stmt = nullptr;
    if (!prepareStatement(db, sql_upsert, &stmt)) return;

    sqlite3_bind_text(stmt, 1, word.getPalabra().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, word.getSignificado().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, static_cast<int>(word.getTipo()));
    sqlite3_bind_int(stmt, 4, static_cast<int>(word.getCantidad()));
    sqlite3_bind_int(stmt, 5, static_cast<int>(word.getTiempo()));
    sqlite3_bind_int(stmt, 6, static_cast<int>(word.getGenero()));
    sqlite3_bind_int(stmt, 7, static_cast<int>(word.getGrado()));
    sqlite3_bind_int(stmt, 8, static_cast<int>(word.getPersona()));
    sqlite3_bind_double(stmt, 9, word.getConfianza());

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Error guardando palabra: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);

    // 2. Obtener el ID de la palabra (recién insertada o existente)
    int palabraId = getWordId(db, word.getPalabra(), false);
    if (palabraId == -1) {
        std::cerr << "Error: no se pudo obtener ID para " << word.getPalabra() << std::endl;
        return;
    }
    // 3. Cargar relaciones existentes desde la base de datos
    std::map<std::string, double> mergedRels;
    // Primero, añadir las relaciones actuales del objeto Word
    for (const auto& [relWord, valor] : word.getRelated()) {
        auto it = mergedRels.find(relWord);
        if (it == mergedRels.end() || valor > it->second) {
            mergedRels[relWord] = valor;
        }
    }

    // Segundo, cargar desde la BD las relaciones ya almacenadas para esta palabra
    const char* sql_loadRels =
        "SELECT p2.palabra, r.valor FROM relaciones r "
        "JOIN palabras p2 ON r.relacionada_id = p2.id "
        "WHERE r.palabra_id = ?";
    sqlite3_stmt* stmtLoad = nullptr;
    if (prepareStatement(db, sql_loadRels, &stmtLoad)) {
        sqlite3_bind_int(stmtLoad, 1, palabraId);
        while (sqlite3_step(stmtLoad) == SQLITE_ROW) {
            std::string relWord = reinterpret_cast<const char*>(sqlite3_column_text(stmtLoad, 0));
            double valor = sqlite3_column_double(stmtLoad, 1);
            auto it = mergedRels.find(relWord);
            if (it == mergedRels.end() || valor > it->second) {
                mergedRels[relWord] = valor;
            }
        }
        sqlite3_finalize(stmtLoad);
    }

    // 4. Limitar a 15 relaciones (las de mayor valor)
    std::vector<std::pair<std::string, double>> finalRels(mergedRels.begin(), mergedRels.end());
    std::sort(finalRels.begin(), finalRels.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    if (finalRels.size() > 15) {
        finalRels.resize(15);
    }

    // 5. Borrar las relaciones actuales de la BD
    const char* sql_delRel = "DELETE FROM relaciones WHERE palabra_id = ?";
    sqlite3_stmt* stmtDel = nullptr;
    if (prepareStatement(db, sql_delRel, &stmtDel)) {
        sqlite3_bind_int(stmtDel, 1, palabraId);
        sqlite3_step(stmtDel);
        sqlite3_finalize(stmtDel);
    }

    // 6. Insertar la lista final (las 15 mejores)
    const char* sql_insRel = "INSERT INTO relaciones (palabra_id, relacionada_id, valor) VALUES (?, ?, ?)";
    for (const auto& [relWord, valor] : finalRels) {
        int relId = getWordId(db, relWord, true);
        if (relId == -1) continue;
        sqlite3_stmt* stmtIns = nullptr;
        if (!prepareStatement(db, sql_insRel, &stmtIns)) continue;
        sqlite3_bind_int(stmtIns, 1, palabraId);
        sqlite3_bind_int(stmtIns, 2, relId);
        sqlite3_bind_double(stmtIns, 3, valor);
        sqlite3_step(stmtIns);
        sqlite3_finalize(stmtIns);
    }
}

bool WordRepository::load(const std::string& palabra, Word& outWord) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return false;

    // Cargar atributos principales
    const char* sql_main = "SELECT significado, tipo, cantidad, tiempo, genero, grado, persona, confianza FROM palabras WHERE palabra = ?";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareStatement(db, sql_main, &stmt)) return false;
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

    // Cargar las relaciones
    outWord.clearRelated();  // limpiar cualquier relación previa

    int palabraId = getWordId(db, palabra, false);
    if (palabraId == -1) {
        // No debería ocurrir porque acabamos de leer la palabra
        return true; // No hay relaciones
    }

    const char* sql_rels =
        "SELECT p2.palabra, r.valor FROM relaciones r "
        "JOIN palabras p2 ON r.relacionada_id = p2.id "
        "WHERE r.palabra_id = ?";
    if (!prepareStatement(db, sql_rels, &stmt)) return true;
    sqlite3_bind_int(stmt, 1, palabraId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string relWord = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double valor = sqlite3_column_double(stmt, 1);
        outWord.addRelated(relWord, valor);
    }
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
    if (!prepareStatement(db, sql, &stmt)) return false;
    sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}
