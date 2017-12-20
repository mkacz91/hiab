#include "files.h"
#include <vector>

namespace hiab {

std::vector<string> file_search_prefixes = { "" };

file_not_found::file_not_found(string const& name) :
    runtime_error("File " + squote(name) + " could not be found."),
    m_name(name)
{ }

string const& file_not_found::name() const { return m_name; }

void add_file_search_prefix(string const& prefix)
{
    file_search_prefixes.push_back(prefix);
}

std::ifstream open_file_for_reading(string const& name)
{
    std::ifstream result;
    for (string const& prefix : file_search_prefixes)
    {
        result.open(prefix + FILE_PATH_SEPARATOR + name);
        if (result.is_open())
            return std::move(result);
    }
    throw file_not_found(name);
}

string read_all_text_from_file(string const& name)
{
    auto istr = open_file_for_reading(name);
    int start_pos = istr.tellg();
    istr.seekg(0, std::istream::end);
    int stream_size = (int)istr.tellg() - start_pos;
    string result(stream_size, '\0');
    istr.seekg(start_pos, std::istream::beg);
    istr.read(&result[0], stream_size);
    return std::move(result);
}

} // namespace hiab
