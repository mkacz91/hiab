#include "files.h"
#include <vector>

namespace hiab {

std::vector<string> file_search_prefixes = { "." };

file_error::file_error(string const& name, string const& message) :
    runtime_error("Error loading file " + squote(name) + ": " + message),
    m_name(name)
{ }

string const& file_error::name() const { return m_name; }

file_not_found::file_not_found(string const& name) :
    file_error(name, "file not found.")
{ }

string join_path(string const& path0, string const& path1)
{
    return path0 + FILE_PATH_SEPARATOR + path1;
}

void add_file_search_prefix(string const& prefix)
{
    file_search_prefixes.push_back(prefix);
}
template <typename Action>
void dispatch_on_located_file(string const& name, Action action)
{
    std::ifstream stream;
    for (string const& prefix : file_search_prefixes)
    {
        string path = join_path(prefix, name);
        stream.open(path);
        if (stream.is_open())
            return action(std::move(path), std::move(stream));
    }
    throw file_not_found(name);
}

string get_file_path(string const& name)
{
    string result;
    dispatch_on_located_file(name, [&](string&& name, std::ifstream const& _)
        { std::swap(result, name); });
    return std::move(result);
}

std::ifstream open_file_for_reading(string const& name)
{
    std::ifstream result;
    dispatch_on_located_file(name, [&](string const& _, std::ifstream&& stream)
        { std::swap(result, stream); });
    return std::move(result);
}

string read_all_text_from_file(string const& name)
{
    auto istr = open_file_for_reading(name);
    auto start_pos = (int)istr.tellg();
    istr.seekg(0, std::istream::end);
    int stream_size = (int)istr.tellg() - start_pos;
    string result(stream_size, '\0');
    istr.seekg(start_pos, std::istream::beg);
    istr.read(&result[0], stream_size);
    return std::move(result);
}

} // namespace hiab
