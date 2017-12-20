#pragma once

#include <cassert>
#include <stdexcept>
#include <utility>
#include <string>
#include <sstream>

namespace hiab {

using std::string;

inline string quote(const string& s) { return "\"" + s + "\""; }
inline string squote(const string& s) { return "'" + s + "'"; }
inline string paren(const string& s) { return "(" + s + ")"; }

template <typename T> inline
std::string to_string(const T& t)
{
    std::ostringstream sout;
    sout << t;
    return sout.str();
}

} // namespace hiab
