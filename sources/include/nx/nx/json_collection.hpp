#ifndef __NX_JSON_COLLECTION_H__
#define __NX_JSON_COLLECTION_H__

#include <unordered_map>
#include <string>
#include <type_traits>
#include <mutex>

#include <jsonv/serialization_builder.hpp>

#include <nx/config.h>
#include <nx/json.hpp>
#include <nx/http.hpp>
#include <nx/route.hpp>
#include <nx/utils.hpp>
#include <nx/callbacks.hpp>

namespace nx {

namespace tags {

struct on_collection_loaded_tag : callback_tag {};

const on_collection_loaded_tag on_collection_loaded = {};

struct on_item_added_tag : callback_tag {};
struct on_item_removed_tag : callback_tag {};
struct on_item_changed_tag : callback_tag {};

const on_item_added_tag on_item_added = {};
const on_item_removed_tag on_item_removed = {};
const on_item_changed_tag on_item_changed = {};

} // namespace tags

struct item_tag {};
struct collection_tag {};

class NX_API json_collection_base
{
public:
    using id_type = std::string;
    using callbacks = nx::callbacks<
        callback<tags::on_collection_loaded_tag, const jsonv::value&>,
        callback<tags::on_item_added_tag, id_type, const jsonv::value&, reply&>,
        callback<tags::on_item_removed_tag, id_type, const jsonv::value&, reply&>,
        callback<tags::on_item_changed_tag, id_type, const jsonv::value&, reply&>
    >;

    json_collection_base(const std::string& path);
    virtual ~json_collection_base();

    template <
        typename Tag,
        typename Enabled = typename std::enable_if<
            std::is_base_of<callback_tag, Tag>::value
        >::type
    >
    auto&
    handler(const Tag& t)
    { return callbacks_.get(t); }

    template <typename Tag>
    auto&
    operator[](const Tag& t)
    { return handler(t); }

    const std::string& path() const;

    void set_dir(const std::string& path);

    virtual route_cb GET(const collection_tag& t) = 0;
    virtual route_cb POST(const collection_tag& t) = 0;
    virtual route_cb GET(const item_tag& t) = 0;
    virtual route_cb PUT(const item_tag& t) = 0;
    virtual route_cb DELETE(const item_tag& t) = 0;

protected:
    id_type make_id();

    jsonv::value load() const;
    bool save(const jsonv::value& coll) const;

private:
    std::string path_;
    callbacks callbacks_;
    std::string dir_;
};

class NX_API json_collection : public json_collection_base
{
public:
    using base_type = json_collection_base;
    using value_type = jsonv::value;
    using id_type = json_collection_base::id_type;
    using values_type = std::unordered_map<id_type, value_type>;

    json_collection(const std::string& path);

    values_type& get();
    const values_type& get() const;

    value_type& get(id_type id);
    const value_type& get(id_type id) const;

    void load();
    void save() const;

    virtual route_cb GET(const collection_tag& t);
    virtual route_cb POST(const collection_tag& t);
    virtual route_cb GET(const item_tag& t);
    virtual route_cb PUT(const item_tag& t);
    virtual route_cb DELETE(const item_tag& t);

private:
    void locked(void_cb cb)
    {
        std::lock_guard<std::mutex> lock(cm_);

        cb();
    }

    void load_collection(const jsonv::value& c);
    jsonv::value get_collection() const;

    values_type c_;
    std::mutex cm_;
};

template <typename T>
class typed_json_collection : public json_collection_base
{
public:
    using this_type = typed_json_collection<T>;
    using value_type = T;
    using id_type = json_collection_base::id_type;
    using values_type = std::unordered_map<id_type, value_type>;

    typed_json_collection(const std::string& path, const jsonv::formats& fmt)
    : json_collection_base(path)
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

    route_cb POST(const collection_tag& t)
    {
        return [&](const request& req, buffer& data, reply& rep) {
            auto id = this->make_id();
            value_type item;

            if (data.empty()) {
                throw BadRequest("request body is empty");
            }

            try {
                json(data) >> item;

                item.id = id;
                c_.emplace(id, item);

                rep
                    << Created
                    << header{ location, join("/", path_, id) }
                    ;
            } catch (const std::exception& e) {
                throw BadRequest("bad JSON data: ", e);
            }
        };
    }

    route_cb GET(const item_tag& t)
    {
        return [&](const request& req, buffer& data, reply& rep) {
            const auto& id = req.a("id");
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
            const auto& id = req.a("id");

            value_type item;

            try {
                json(data) >> item;
            } catch (const std::exception& e) {
                throw BadRequest("bad JSON data: ", e);
            }

            item.id = id;
            c_[id] = item;
        };
    }

    route_cb DELETE(const item_tag& t)
    {
        return [&](const request& req, buffer& data, reply& rep) {
            const auto& id = req.a("id");

            c_.erase(id);
        };
    }

private:
    std::string path_;
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
