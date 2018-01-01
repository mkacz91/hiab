#pragma once

#include <cassert>
#include <stdexcept>
#include <utility>
#include <string>

namespace hiab {

using std::string;
using std::to_string;

inline string quote(const string& s) { return "\"" + s + "\""; }
inline string squote(const string& s) { return "'" + s + "'"; }
inline string paren(const string& s) { return "(" + s + ")"; }

inline bool string_starts_with(string const& s, string const& pattern)
{
    if (s.length() < pattern.length())
        return false;
    int length = (int)pattern.length();
    for (int i = 0; i < length; ++i)
    {
        if (s[i] != pattern[i])
            return false;
    }
    return true;
}

template <typename T>
T const& view_as(void const* data)
{
    return *reinterpret_cast<T const*>(data);
}

class not_implemented : public std::logic_error
{
public:
    not_implemented(string const& message) : logic_error("Not implemented: " + message) { }
};

#define not_implemented_here not_implemented( \
    "line " + to_string(__LINE__) + ", file '" __FILE__ "', ")

} // namespace hiab
