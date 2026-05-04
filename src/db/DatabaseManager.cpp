/**==============================================================================
    Admin8.2.1 - DatabaseManager.cpp
    Proposito: Implementación del gestor de múltiples bases de datos SQLite.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Cada ruta tiene su propia conexión.
==============================================================================*/

#include "DatabaseManager.hpp"
#include <iostream>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

bool DatabaseManager::init(const std::string& dbPath) {
    // Si ya está abierta, no hacer nada
    if (connections_.find(dbPath) != connections_.end()) {
        return true;
    }

    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error abriendo base de datos " << dbPath << ": "
                  << sqlite3_errmsg(db) << std::endl;
        if (db) sqlite3_close(db);
        return false;
    }

    // Configurar pragmas para rendimiento (opcional)
    const char* pragmas = "PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL;";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, pragmas, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Advertencia al configurar pragmas: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    connections_[dbPath] = db;
    return true;
}

void DatabaseManager::close(const std::string& dbPath) {
    auto it = connections_.find(dbPath);
    if (it != connections_.end()) {
        sqlite3_close(it->second);
        connections_.erase(it);
    }
}

void DatabaseManager::closeAll() {
    for (auto& pair : connections_) {
        sqlite3_close(pair.second);
    }
    connections_.clear();
}

sqlite3* DatabaseManager::getHandle(const std::string& dbPath) const {
    auto it = connections_.find(dbPath);
    return (it != connections_.end()) ? it->second : nullptr;
}

bool DatabaseManager::prepareStatement(const std::string& dbPath, const char* sql, sqlite3_stmt** stmt) const {
    sqlite3* db = getHandle(dbPath);
    if (!db) return false;
    return prepareSqlStatement(db, sql, stmt);
}
