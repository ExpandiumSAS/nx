#include <stdlib.h>
#include <cxxabi.h>

#include <regex>
#include <algorithm>
#include <stdexcept>

#include <cxxu/utils.hpp>

#include <nx/utils.hpp>

namespace nx {

std::string
demangle_type_name(const std::string& mangled)
{
    char* buffer;
    int status;

    buffer = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);

    if (status == 0) {
        std::string n(buffer);
        free(buffer);

        return n;
    } else {
        return std::string("demangle failure");
    }

    return std::string("unsupported");
}

std::string
lc(const std::string& s)
{
    std::string t = s;
    std::transform(s.begin(), s.end(), t.begin(), ::tolower);

    return t;
}

void
split(const std::string& re, const std::string& expr, strings& list)
{
    list.clear();

    if (expr.empty()) {
        return;
    }

    std::regex delim(re);

    auto cur = std::sregex_token_iterator(
        expr.begin(), expr.end(),
        delim,
        -1
    );
    auto end = std::sregex_token_iterator();

    for( ; cur != end; ++cur ) {
        list.push_back(*cur);
    }

    if (list.empty() && expr.size() > 0) {
        list.push_back(expr);
    }
}

bool
match(const std::string& expr, const std::string& re)
{
    std::regex mre(re);

    return regex_match(expr, mre);
}

bool
match(const std::string& expr, const std::string& re, std::smatch& m)
{
    std::regex mre(re);

    return regex_match(expr, m, mre);
}

std::string
subst(const std::string& s, const std::string& re, const std::string& what)
{
    return std::regex_replace(
        s,
        std::regex(re),
        what
    );
}

std::string
clean_path(const std::string& path)
{
    // Note: simpler char-based algorithms will not work with UTF8 data...
    std::string cleaned;
    auto parts = split("/", path);

    for (const auto& part : parts) {
        if (part.empty()) {
            continue;
        }

        cleaned += '/';
        cleaned += part;
    }

    return cleaned;
}

std::string
slurp_file(const std::string& path)
{
    if (!cxxu::file_exists(path)) {
        throw std::runtime_error("cannot read " + path);
    }

    std::ifstream ifs(path, std::ios::binary);

    if (!ifs.is_open()) {
        throw std::runtime_error("failed to open " + path);
    }

    auto length = cxxu::file_size(path);
    std::string s(length, ' ');

    ifs.read(&*s.begin(), length);
    ifs.close();

    return s;
}

} // namespace nx
