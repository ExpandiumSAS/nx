#include <hxx/multipart.hpp>
#include <hxx/utils.hpp>
#include <hxx/escape.hpp>

namespace hxx {

const char CR = 13;
const char LF = 10;

multipart_parser::multipart_parser(const std::string& boundary)
: boundary_(boundary),
boundary_length_(boundary_.size()),
lookbehind_(boundary_length_ + 9, '\0'),
index_(0),
state_(state::start)
{}

multipart_parser::~multipart_parser()
{}

std::size_t
multipart_parser::operator()(const char* buf, std::size_t len)
{
    size_t i = 0;
    size_t mark = 0;
    char c, cl;
    int is_last = 0;

    while (i < len) {
        c = buf[i];
        is_last = (i == (len - 1));

        switch (state_) {
            case state::start:
            index_ = 0;
            state_ = state::start_boundary;

            // fallthrough
            case state::start_boundary:
            if (index_ == boundary_length_) {
                if (c != CR) {
                    return i;
                }

                index_++;
                break;
            } else if (index_ == (boundary_length_ + 1)) {
                if (c != LF) {
                    return i;
                }

                index_ = 0;
                on_part_data_begin();
                state_ = state::header_field_start;
                break;
            }

            if (c != boundary_[index_]) {
                return i;
            }

            index_++;
            break;

            case state::header_field_start:
            mark = i;
            state_ = state::header_field;

            // fallthrough
            case state::header_field:
            if (c == CR) {
                state_ = state::headers_almost_done;
                break;
            }

            if (c == '-') {
                break;
            }

            if (c == ':') {
                on_header_field(buf + mark, i - mark);
                state_ = state::header_value_start;
                break;
            }

            cl = tolower(c);

            if (cl < 'a' || cl > 'z') {
                return i;
            }

            if (is_last) {
                on_header_field(buf + mark, (i - mark) + 1);
            }
            break;

            case state::headers_almost_done:
            if (c != LF) {
                return i;
            }

            state_ = state::part_data_start;
            break;

            case state::header_value_start:
            if (c == ' ') {
                break;
            }

            mark = i;
            state_ = state::header_value;

            // fallthrough
            case state::header_value:
            if (c == CR) {
                on_header_value(buf + mark, i - mark);
                state_ = state::header_value_almost_done;
            }

            if (is_last) {
                on_header_value(buf + mark, (i - mark) + 1);
            }
            break;

            case state::header_value_almost_done:
            if (c != LF) {
                return i;
            }
            state_ = state::header_field_start;
            break;

            case state::part_data_start:
            on_headers_complete();
            mark = i;
            state_ = state::part_data;

            // fallthrough
            case state::part_data:
            if (c == CR) {
                on_part_data(buf + mark, i - mark);
                mark = i;
                state_ = state::part_data_almost_boundary;
                lookbehind_[0] = CR;
                break;
            }

            if (is_last) {
                on_part_data(buf + mark, (i - mark) + 1);
            }
            break;

            case state::part_data_almost_boundary:
            if (c == LF) {
                state_ = state::part_data_boundary;
                lookbehind_[1] = LF;
                index_ = 0;
                break;
            }

            on_part_data(lookbehind_.data(), 1);
            state_ = state::part_data;
            mark = i--;
            break;

            case state::part_data_boundary:
            if (boundary_[index_] != c) {
                on_part_data(lookbehind_.data(), 2 + index_);
                state_ = state::part_data;
                mark = i--;
                break;
            }

            lookbehind_[2 + index_] = c;

            if ((++index_) == boundary_length_) {
                on_part_data_end();
                state_ = state::part_data_almost_end;
            }
            break;

            case state::part_data_almost_end:
            if (c == '-') {
                state_ = state::part_data_final_hyphen;
                break;
            }

            if (c == CR) {
                state_ = state::part_data_end;
                break;
            }

            return i;

            case state::part_data_final_hyphen:
            if (c == '-') {
                on_body_end();
                state_ = state::end;
                break;
            }
            return i;

            case state::part_data_end:
            if (c == LF) {
                state_ = state::header_field_start;
                on_part_data_begin();
                break;
            }
            return i;

            case state::end:
            break;

            default:
            // Throw something
            return 0;
        }

        ++i;
    }

    return len;
}

void
multipart_parser::on_header_field(const char* buf, std::size_t size)
{
    cur_header_name_.assign(buf, size);
    cur_header_name_ = lc(cur_header_name_);
}

void
multipart_parser::on_header_value(const char* buf, std::size_t size)
{ parts_.back().h[cur_header_name_].assign(buf, size); }

void
multipart_parser::on_headers_complete()
{
    auto& p = parts_.back();
    auto& cd = p.h[content_disposition_lc];

    if (cd.find("form-data;") == 0) {
        cd.erase(0, 10);
        p.a << attributes(cd);
    }
}

void
multipart_parser::on_part_data_begin()
{ parts_.emplace_back(); }

void
multipart_parser::on_part_data(const char* buf, std::size_t size)
{
    auto& p = parts_.back();

    if (!p.filename.empty()) {
        // TODO: write to a file
    } else {
        // "simple" part
        p.value.append(buf, buf + size);
    }
}

void
multipart_parser::on_part_data_end()
{}

void
multipart_parser::on_body_end()
{}

} // namespace hxx
