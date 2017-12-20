#pragma once

#include "prefix.h"
#include <fstream>

namespace hiab {

#ifdef HIAB_WINDOWS
    constexpr char FILE_PATH_SEPARATOR = '\\';
#else
    constexpr char FILE_PATH_SEPARATOR = '/';
#endif

class file_not_found : public std::runtime_error
{
public:
    explicit file_not_found(string const& name);
    string const& name() const;

private:
    string m_name;
};

void add_file_search_prefix(string const& prefix);

std::ifstream open_file_for_reading(string const& name);

string read_all_text_from_file(string const& name);

} // namespace hiab
