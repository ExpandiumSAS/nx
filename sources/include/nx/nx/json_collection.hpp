#ifndef __NX_JSON_COLLECTION_H__
#define __NX_JSON_COLLECTION_H__

#include <unordered_map>
#include <string>
#include <type_traits>

#include <jsonv/serialization_builder.hpp>

#include <nx/config.h>
#include <nx/json.hpp>
#include <nx/http.hpp>
#include <nx/route.hpp>
#include <nx/utils.hpp>

namespace nx {

struct item_tag {};
struct collection_tag {};

template <typename T>
struct next_id
{};

template <>
struct next_id<std::size_t>
{
    std::size_t operator()(std::size_t& id) const
    { return ++id; }
};

class NX_API json_collection_base
{
public:
    json_collection_base(const std::string& path);
    virtual ~json_collection_base();

    const std::string& path() const;

    virtual route_cb GET(const collection_tag& t) = 0;
    virtual route_cb PUT(const collection_tag& t) = 0;
    virtual route_cb POST(const collection_tag& t) = 0;
    virtual route_cb DELETE(const collection_tag& t) = 0;
    virtual route_cb GET(const item_tag& t) = 0;
    virtual route_cb PUT(const item_tag& t) = 0;
    virtual route_cb DELETE(const item_tag& t) = 0;

private:
    std::string path_;
};

class json_collection : public json_collection_base
{
public:
    using value_type = jsonv::value;
    using id_type = std::size_t;
    using next_id_type = next_id<id_type>;
    using values_type = std::unordered_map<id_type, value_type>;

    json_collection(const std::string& path);

    values_type& get();
    const values_type& get() const;

    virtual route_cb GET(const collection_tag& t);
    virtual route_cb PUT(const collection_tag& t);
    virtual route_cb POST(const collection_tag& t);
    virtual route_cb DELETE(const collection_tag& t);
    virtual route_cb GET(const item_tag& t);
    virtual route_cb PUT(const item_tag& t);
    virtual route_cb DELETE(const item_tag& t);

private:
    id_type id_;
    next_id_type next_id_;
    values_type c_;
};

template <typename T>
class typed_json_collection : public json_collection_base
{
public:
    using this_type = typed_json_collection<T>;
    using value_type = T;
    using id_type = decltype(std::declval<T&>().id);
    using next_id_type = next_id<id_type>;
    using values_type = std::unordered_map<id_type, value_type>;

    typed_json_collection(const std::string& path, const jsonv::formats& fmt)
    : json_collection_base(path),
    id_(0)
    {
        // Register T format
        add_json_format<T>(fmt);

        // Register collection format
        add_json_format<values_type>(
            jsonv::formats_builder().register_container<values_type>()
        );
    }

    values_type& get()
    { return c_; }

    const values_type& get() const
    { return c_; }

    route_cb GET(const collection_tag& t)
    {
        return [&](const request& req, buffer& data, reply& rep) {
            rep << json(c_);
        };
    }

    route_cb PUT(const collection_tag& t)
    {
        return [&](const request& req, buffer& data, reply& rep) {
            c_.clear();
            json(data) >> c_;
        };
    }

    route_cb POST(const collection_tag& t)
    {
        return [&](const request& req, buffer& data, reply& rep) {
            auto id = next_id_(id_);
            value_type item;

            if (data.empty()) {
                throw BadRequest;
            }

            json(data) >> item;

            item.id = id;
            c_.emplace(id, item);

            rep
                << Created
                << header{ location, join("/", path_, id) }
                ;
        };
    }

    route_cb DELETE(const collection_tag& t)
    {
        return [&](const request& req, buffer& data, reply& rep) {
            c_.clear();
        };
    }

    route_cb GET(const item_tag& t)
    {
        return [&](const request& req, buffer& data, reply& rep) {
            auto id = nx::to_num<id_type>(req.a("id"));
            auto it = c_.find(id);

            if (it != c_.end()) {
                rep << json(it->second);
            } else {
                rep << NotFound;
            }
        };
    }

    route_cb PUT(const item_tag& t)
    {
        return [&](const request& req, buffer& data, reply& rep) {
            auto id = nx::to_num<id_type>(req.a("id"));

            value_type item;
            json(data) >> item;

            item.id = id;
            c_[id] = item;
        };
    }

    route_cb DELETE(const item_tag& t)
    {
        return [&](const request& req, buffer& data, reply& rep) {
            auto id = nx::to_num<id_type>(req.a("id"));

            c_.erase(id);
        };
    }

private:
    std::string path_;
    id_type id_;
    next_id_type next_id_;
    values_type c_;
};

template <typename T>
struct json_collection_type
{
    using type = typed_json_collection<T>;
};

template <>
struct json_collection_type<jsonv::value>
{
    using type = json_collection;
};

template <typename T = jsonv::value>
struct make_json_collection
{
    using type = typename json_collection_type<T>::type;
};

} // namespace nx

#endif // __NX_JSON_COLLECTION_H__
