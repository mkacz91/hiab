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

class not_implemented : public std::logic_error
{
public:
    not_implemented(string const& message) : logic_error("Not implemented: " + message) { }
};

#define not_implemented_here not_implemented( \
    "line " + to_string(__LINE__) + ", file '" __FILE__ "', ")

} // namespace hiab
