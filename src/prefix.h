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

} // namespace hiab
