#include <iostream>
#include <fstream>

#include <cxxu/utils.hpp>
#include <cxxu/logging.hpp>

#include <nx/json_collection.hpp>

namespace nx {

json_collection_base::json_collection_base(const std::string& path)
: id_(0),
path_(clean_path(path))
{}

json_collection_base::~json_collection_base()
{}

json_collection_base::id_type
json_collection_base::next_id()
{ return make_next_id_(id_); }

const std::string&
json_collection_base::path() const
{ return path_; }

void
json_collection_base::set_dir(const std::string& dir)
{ dir_ = dir; }

jsonv::value
json_collection_base::load() const
{
    jsonv::value coll;

    if (!dir_.empty()) {
        auto file = cxxu::cat_file(dir_, path_ + ".json");

        if (cxxu::file_exists(file)) {
            std::ifstream ifs(file);

            coll = jsonv::parse(ifs);
        }
    }

    return coll;
}

bool
json_collection_base::save(const jsonv::value& coll) const
{
    if (dir_.empty()) {
        // No save dir specified
        return true;
    }

    auto file = cxxu::cat_file(dir_, path_ + ".json");

    if (!cxxu::mkfilepath(file)) {
        cxxu::warning() << "failed to create " << file;
        return false;
    }

    std::ofstream ofs(file);
    ofs << coll;

    return true;
}

json_collection::json_collection(const std::string& path)
: json_collection_base(path)
{}

json_collection::values_type&
json_collection::get()
{ return c_; }

const json_collection::values_type&
json_collection::get() const
{ return c_; }

void
json_collection::load()
{
    auto coll = base_type::load();
    put_collection(coll);
}

void
json_collection::save() const
{ base_type::save(get_collection()); }

route_cb
json_collection::GET(const collection_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        rep << get_collection();
    };
}

route_cb
json_collection::PUT(const collection_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        if (data.empty()) {
            throw BadRequest;
        }

        c_.clear();

        json arr(data);
        put_collection(arr.value());
        save();
    };
}

route_cb
json_collection::POST(const collection_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        if (data.empty()) {
            throw BadRequest;
        }

        auto id = next_id();
        json item(data);

        item.value()["id"] = id;
        c_.emplace(id, item.value());
        save();
        handler(tags::on_item_added)(id);

        rep
            << Created
            << header{ location, join("/", path(), id) }
            ;
    };
}

route_cb
json_collection::DELETE(const collection_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        c_.clear();
        save();
    };
}

route_cb
json_collection::GET(const item_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        auto id = nx::to_num<id_type>(req.a("id"));
        auto it = c_.find(id);

        if (it != c_.end()) {
            rep << it->second;
        } else {
            rep << NotFound;
        }
    };
}

route_cb
json_collection::PUT(const item_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        auto id = nx::to_num<id_type>(req.a("id"));

        json item(data);

        item.value()["id"] = id;
        c_[id] = item.value();
        save();
        handler(tags::on_item_added)(id);
    };
}

route_cb
json_collection::DELETE(const item_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        auto id = nx::to_num<id_type>(req.a("id"));

        handler(tags::on_item_removed)(id);
        c_.erase(id);
        save();
    };
}

void
json_collection::put_collection(const jsonv::value& c)
{
    for (auto& v : c.as_array()) {
        auto it = v.find("id");

        if (it == v.end_object()) {
            continue;
        }

        auto id = nx::to_num<id_type>(
            jsonv::extract<std::string>(it->second)
        );

        c_[id] = v;
        handler(tags::on_item_added)(id);
    }
}

jsonv::value
json_collection::get_collection() const
{
    jsonv::value coll = jsonv::array();

    for (const auto& p : c_) {
        coll.push_back(p.second);
    }

    return coll;
}

} // namespace nx
