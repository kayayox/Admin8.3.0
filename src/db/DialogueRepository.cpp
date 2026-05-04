/**==============================================================================
    Admin8.2.1 - DialogueRepository.cpp
    Proposito: Implementación de repositorio de diálogos.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "DialogueRepository.hpp"
#include "DatabaseManager.hpp"
#include "SentenceRepository.hpp"  // para guardar/obtener IDs
#include <iostream>

std::string DialogueRepository::dbPath_;

void DialogueRepository::setDatabasePath(const std::string& path) {
    dbPath_ = path;
    DatabaseManager::instance().init(dbPath_);
    SentenceRepository::setDatabasePath(path); // misma BD
}

std::string DialogueRepository::getDatabasePath() {
    return dbPath_;
}

void DialogueRepository::initializeTables() {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return;

    const char* sql =
        "CREATE TABLE IF NOT EXISTS dialogos ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "premisa_id INTEGER, hipotesis_id INTEGER, tipo_patron INTEGER,"
        "creatividad REAL, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE TABLE IF NOT EXISTS feedback ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "palabra TEXT, tipo_propuesto INTEGER, tipo_correcto INTEGER,"
        "acierto INTEGER, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creando tablas de diálogos: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

int DialogueRepository::getOrCreateSentenceId(const Sentence& sentence) {
    if (sentence.getId() > 0) return sentence.getId();
    // Guardar una copia modificable
    Sentence copy = sentence;
    SentenceRepository::save(copy);
    return copy.getId();
}

void DialogueRepository::saveDialogue(const Sentence& premise, const Sentence& hypothesis,
                                      const Pattern& pattern, float creativity) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return;

    int premId = getOrCreateSentenceId(premise);
    int hypId = getOrCreateSentenceId(hypothesis);

    // Verificar si ya existe
    const char* sql_check = "SELECT id FROM dialogos WHERE premisa_id = ? AND hipotesis_id = ? AND tipo_patron = ? AND creatividad = ?";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql_check, &stmt)) return;
    sqlite3_bind_int(stmt, 1, premId);
    sqlite3_bind_int(stmt, 2, hypId);
    sqlite3_bind_int(stmt, 3, static_cast<int>(pattern.type));
    sqlite3_bind_double(stmt, 4, creativity);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return; // ya existe
    }
    sqlite3_finalize(stmt);

    // Insertar
    const char* sql_ins = "INSERT INTO dialogos (premisa_id, hipotesis_id, tipo_patron, creatividad) VALUES (?,?,?,?)";
    if (!prepareSqlStatement(db, sql_ins, &stmt)) return;
    sqlite3_bind_int(stmt, 1, premId);
    sqlite3_bind_int(stmt, 2, hypId);
    sqlite3_bind_int(stmt, 3, static_cast<int>(pattern.type));
    sqlite3_bind_double(stmt, 4, creativity);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void DialogueRepository::registerFeedback(const std::string& word,
                                          TipoPalabra proposedType,
                                          TipoPalabra correctType,
                                          bool correct) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return;

    int acierto = correct ? 1 : 0;

    // Evitar duplicados exactos recientes (opcional). Simplificamos: insertamos siempre.
    const char* sql_ins = "INSERT INTO feedback (palabra, tipo_propuesto, tipo_correcto, acierto) VALUES (?,?,?,?)";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql_ins, &stmt)) return;
    sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, static_cast<int>(proposedType));
    sqlite3_bind_int(stmt, 3, static_cast<int>(correctType));
    sqlite3_bind_int(stmt, 4, acierto);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<std::string> DialogueRepository::buildCorpus() {
    std::vector<std::string> corpus;
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return corpus;

    const char* sql = "SELECT bloque_texto FROM bloques ORDER BY oracion_id, posicion";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql, &stmt)) return corpus;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* word = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        corpus.emplace_back(word);
    }
    sqlite3_finalize(stmt);
    return corpus;
}

DialogueHistory DialogueRepository::loadHistory() {
    DialogueHistory history;
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return history;

    const char* sql = "SELECT premisa_id, hipotesis_id, tipo_patron, creatividad FROM dialogos ORDER BY timestamp";
    sqlite3_stmt* stmt = nullptr;
    if (!prepareSqlStatement(db, sql, &stmt)) return history;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int premId = sqlite3_column_int(stmt, 0);
        int hypId = sqlite3_column_int(stmt, 1);
        TipoPatron tp = static_cast<TipoPatron>(sqlite3_column_int(stmt, 2));
        float creativity = static_cast<float>(sqlite3_column_double(stmt, 3));

        Sentence premise = SentenceRepository::loadById(premId);
        Sentence hypothesis = SentenceRepository::loadById(hypId);
        Pattern pattern(patternFromSequence(premise.getTypeSequence()).sequence, tp);
        history.addDialogue(premise, hypothesis, pattern, creativity);
    }
    sqlite3_finalize(stmt);
    return history;
}
