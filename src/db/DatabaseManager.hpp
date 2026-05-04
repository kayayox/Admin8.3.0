/**==============================================================================
    Admin8.2.1 - DatabaseManager.hpp
    Proposito: Gestión de conexiones SQLite. Permite abrir múltiples bases de
               datos simultáneamente (identificadas por su ruta de archivo).
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Singleton que mantiene un mapa de rutas -> sqlite3*. Se usa para
           las tres bases de datos: semántica, patrones, y futura temporal.
==============================================================================*/

#ifndef ADMIN821_DATABASE_MANAGER_HPP
#define ADMIN821_DATABASE_MANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

class DatabaseManager {
public:
    // Obtener instancia única
    static DatabaseManager& instance();

    // Inicializar/conectar una base de datos por su ruta.
    // Retorna true si se abrió correctamente (o ya estaba abierta).
    bool init(const std::string& dbPath);

    // Cerrar una base de datos específica.
    void close(const std::string& dbPath);

    // Cerrar todas las conexiones.
    void closeAll();

    // Obtener handle de una base de datos (nullptr si no está abierta).
    sqlite3* getHandle(const std::string& dbPath) const;

    // Prepara una sentencia SQL usando la conexión adecuada.
    // Devuelve true si se preparó correctamente.
    bool prepareStatement(const std::string& dbPath, const char* sql, sqlite3_stmt** stmt) const;

    // Deshabilitar copia
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

private:
    DatabaseManager() = default;
    ~DatabaseManager() { closeAll(); }

    std::unordered_map<std::string, sqlite3*> connections_;
};

// Función auxiliar global para preparar sentencias (comodidad)
inline bool prepareSqlStatement(sqlite3* db, const char* sql, sqlite3_stmt** stmt) {
    int rc = sqlite3_prepare_v2(db, sql, -1, stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparando SQL: " << sql << "\n"
                  << "Mensaje: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

#endif // ADMIN821_DATABASE_MANAGER_HPP
