#ifndef TEMPLATE_REPOSITORY_HPP
#define TEMPLATE_REPOSITORY_HPP

#include "../utils/ResponseTemplates.hpp"
#include <string>
#include <vector>
#include <optional>

class TemplateRepository {
public:
    static void setDatabasePath(const std::string& path);
    static void initializeTables();

    static void save(const ResponseTemplate& tmpl);
    static std::optional<ResponseTemplate> loadById(int id);
    static std::vector<ResponseTemplate> loadAll();
    static bool remove(int id);
    static void loadDefaultIfEmpty();

private:
    static std::string dbPath_;
};

#endif // TEMPLATE_REPOSITORY_HPP
