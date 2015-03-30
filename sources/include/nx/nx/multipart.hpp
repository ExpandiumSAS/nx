#ifndef __NX_MULTIPART_H__
#define __NX_MULTIPART_H__

#include <string>
#include <vector>

#include <nx/config.h>
#include <nx/buffer.hpp>
#include <nx/headers.hpp>
#include <nx/attributes.hpp>

namespace nx {

struct part
{
    std::string name;
    std::string filename;
    headers h;
    attributes a;
    std::string value;
};

using parts = std::vector<part>;

class NX_API multipart_parser
{
public:
    multipart_parser(const std::string& boundary);
    virtual ~multipart_parser();

    std::size_t operator()(const char* buf, std::size_t len);

private:
    enum class state
    {
        uninitialized = 1,
        start,
        start_boundary,
        header_field_start,
        header_field,
        headers_almost_done,
        header_value_start,
        header_value,
        header_value_almost_done,
        part_data_start,
        part_data,
        part_data_almost_boundary,
        part_data_boundary,
        part_data_almost_end,
        part_data_end,
        part_data_final_hyphen,
        end
    };

    void on_header_field(const char* buf, std::size_t size);
    void on_header_value(const char* buf, std::size_t size);
    void on_part_data(const char* buf, std::size_t size);
    void on_part_data_begin();
    void on_headers_complete();
    void on_part_data_end();
    void on_body_end();

    std::string boundary_;
    std::size_t boundary_length_;
    std::string lookbehind_;
    std::size_t index_;
    state state_;

    std::string cur_header_name_;
    parts parts_;
};

} // namespace nx

#endif // __NX_MULTIPART_H__
