#include "TemplateRepository.hpp"
#include "DatabaseManager.hpp"
#include <sstream>
#include <iostream>

std::string TemplateRepository::dbPath_;

void TemplateRepository::setDatabasePath(const std::string& path) {
    dbPath_ = path;
    DatabaseManager::instance().init(dbPath_);
}

void TemplateRepository::initializeTables() {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return;
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS response_templates (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            pattern_type INTEGER NOT NULL,
            template_text TEXT NOT NULL,
            slots TEXT,               -- JSON o CSV, pero podemos generarlo automáticamente
            priority INTEGER DEFAULT 0,
            context_keywords TEXT     -- palabras separadas por comas
        );
    )";
    char* err = nullptr;
    sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (err) std::cerr << "Error creando tabla response_templates: " << err << std::endl;
    sqlite3_free(err);
}

static std::string serializeSlots(const std::vector<std::string>& slots) {
    std::stringstream ss;
    for (size_t i = 0; i < slots.size(); ++i) {
        if (i > 0) ss << ",";
        ss << slots[i];
    }
    return ss.str();
}

static std::vector<std::string> deserializeSlots(const std::string& str) {
    std::vector<std::string> res;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) res.push_back(item);
    }
    return res;
}

static std::string serializeKeywords(const std::vector<std::string>& kw) {
    return serializeSlots(kw);
}

static std::vector<std::string> deserializeKeywords(const std::string& str) {
    return deserializeSlots(str);
}

void TemplateRepository::save(const ResponseTemplate& tmpl) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return;
    sqlite3_stmt* stmt;
    if (tmpl.id == -1) {
        const char* sql = "INSERT INTO response_templates (pattern_type, template_text, slots, priority, context_keywords) VALUES (?,?,?,?,?)";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, static_cast<int>(tmpl.patternType));
        sqlite3_bind_text(stmt, 2, tmpl.templateStr.c_str(), -1, SQLITE_STATIC);
        std::string slotsStr = serializeSlots(tmpl.slots);
        sqlite3_bind_text(stmt, 3, slotsStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, tmpl.priority);
        std::string kwStr = serializeKeywords(tmpl.contextKeywords);
        sqlite3_bind_text(stmt, 5, kwStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        const char* sql = "UPDATE response_templates SET pattern_type=?, template_text=?, slots=?, priority=?, context_keywords=? WHERE id=?";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, static_cast<int>(tmpl.patternType));
        sqlite3_bind_text(stmt, 2, tmpl.templateStr.c_str(), -1, SQLITE_STATIC);
        std::string slotsStr = serializeSlots(tmpl.slots);
        sqlite3_bind_text(stmt, 3, slotsStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, tmpl.priority);
        std::string kwStr = serializeKeywords(tmpl.contextKeywords);
        sqlite3_bind_text(stmt, 5, kwStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, tmpl.id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

std::optional<ResponseTemplate> TemplateRepository::loadById(int id) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return std::nullopt;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT pattern_type, template_text, slots, priority, context_keywords FROM response_templates WHERE id=?";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    ResponseTemplate tmpl;
    tmpl.id = id;
    tmpl.patternType = static_cast<TipoPatron>(sqlite3_column_int(stmt, 0));
    tmpl.templateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    tmpl.slots = deserializeSlots(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
    tmpl.priority = sqlite3_column_int(stmt, 3);
    tmpl.contextKeywords = deserializeKeywords(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
    sqlite3_finalize(stmt);
    return tmpl;
}

std::vector<ResponseTemplate> TemplateRepository::loadAll() {
    std::vector<ResponseTemplate> result;
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return result;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, pattern_type, template_text, slots, priority, context_keywords FROM response_templates";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ResponseTemplate tmpl;
        tmpl.id = sqlite3_column_int(stmt, 0);
        tmpl.patternType = static_cast<TipoPatron>(sqlite3_column_int(stmt, 1));
        tmpl.templateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        tmpl.slots = deserializeSlots(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        tmpl.priority = sqlite3_column_int(stmt, 4);
        tmpl.contextKeywords = deserializeKeywords(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        result.push_back(tmpl);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool TemplateRepository::remove(int id) {
    sqlite3* db = DatabaseManager::instance().getHandle(dbPath_);
    if (!db) return false;
    sqlite3_stmt* stmt;
    const char* sql = "DELETE FROM response_templates WHERE id=?";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

void TemplateRepository::loadDefaultIfEmpty() {
    auto all = loadAll();
    if (!all.empty()) return;
    TemplateMatcher defaultMatcher;
    defaultMatcher.loadDefaultTemplates();
    for (const auto& tmpl : defaultMatcher.getAll()) {
        save(tmpl);
    }
}
