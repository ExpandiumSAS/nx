#include <uuid.h>

#include <ios>
#include <iostream>
#include <fstream>

#include <cxxu/utils.hpp>
#include <cxxu/logging.hpp>
#include <cxxu/rw_tx.hpp>

#include <nx/json_collection.hpp>

namespace nx {

json_collection_base::json_collection_base(const std::string& path)
: path_(clean_path(path))
{}

json_collection_base::~json_collection_base()
{}

json_collection_base::id_type
json_collection_base::make_id()
{
    static const char* const lut = "0123456789ABCDEF";

    uuid_t uuid;

    uuid_generate(uuid);

    id_type id;

    id.reserve(2 * sizeof(uuid_t));

    for (const auto& c : uuid) {
        id.push_back(lut[c >> 4]);
        id.push_back(lut[c & 15]);
    }

    return id;
}

const std::string&
json_collection_base::path() const
{ return path_; }

void
json_collection_base::set_dir(const std::string& dir)
{ dir_ = dir; }

jsonv::value
json_collection_base::load() const
{
    jsonv::value coll = jsonv::array();

    if (!dir_.empty()) {
        auto file = cxxu::cat_file(dir_, path_ + ".json");

        auto tx = cxxu::rw_tx(file);

        auto loaded = tx(
            [&coll](std::ifstream& ifs) {
                coll = jsonv::parse(ifs);
            }
        );

        if (!loaded) {
            cxxu::warning() << "collection at " << path_ << " was not loaded";
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

    auto tx = cxxu::rw_tx(file);

    auto saved = tx(
        [&coll](std::ofstream& ofs) {
            ofs << coll;
        }
    );

    if (!saved) {
        cxxu::warning() << "collection at " << path_ << " was not saved";
    }

    return saved;
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

json_collection::value_type&
json_collection::get(id_type id)
{ return c_[id]; }

const json_collection::value_type&
json_collection::get(id_type id) const
{ return c_.at(id); }

void
json_collection::load()
{
    auto coll = base_type::load();

    try {
        load_collection(coll);
    } catch (const std::exception& e) {
        cxxu::warning()
            << "collection '" << path() << "'"
            << " contains errors:\n"
            << e.what()
            ;
    }
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
json_collection::POST(const collection_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        if (data.empty()) {
            throw BadRequest("request body is empty");
        }

        auto id = make_id();

        locked([&]() {
            try {
                json item(data);

                item.value()["id"] = id;
                c_.emplace(id, item.value());
                save();
            } catch (const std::exception& e) {
                c_.erase(id);

                throw BadRequest("bad JSON data: ", e);
            }

            try {
                handler(tags::on_item_added)(id, c_[id], rep);

                rep
                    << Created
                    << header{ location, join("/", path(), id) }
                    ;
            } catch (const std::exception& e) {
                c_.erase(id);

                throw BadRequest("failed to create new item: ", e);
            }
        });

        rep | [this,id,&rep]() {
            if (rep.is_error()) {
                locked([&]() { c_.erase(id); });
            }
        };
    };
}

route_cb
json_collection::GET(const item_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        const auto& id = req.a("id");

        locked([this,id,&rep]() {
            auto it = c_.find(id);

            if (it != c_.end()) {
                rep << it->second;
            } else {
                rep << NotFound;
            }
        });
    };
}

route_cb
json_collection::PUT(const item_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        const auto& id = req.a("id");

        if (data.empty()) {
            throw BadRequest("request body is empty");
        }

        jsonv::value old_value;
        bool exists = false;

        locked([&]() {
            try {
                json item(data);

                item.value()["id"] = id;

                exists = (c_.find(id) != c_.end());

                if (exists) {
                    old_value = c_[id];
                }

                c_[id] = item.value();

                if (exists) {
                    handler(tags::on_item_changed)(id, c_[id], rep);
                } else {
                    handler(tags::on_item_added)(id, c_[id], rep);
                }

                save();
            } catch (const std::exception& e) {
                if (exists) {
                    c_[id] = old_value;
                    handler(tags::on_item_changed)(id, c_[id], rep);
                } else {
                    c_.erase(id);
                }

                throw BadRequest("bad JSON data: ", e);
            }
        });

        rep | [this,id,&rep]() {
            if (rep.is_error()) {
                locked([&]() { c_.erase(id); });
            }
        };
    };
}

route_cb
json_collection::DELETE(const item_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        const auto& id = req.a("id");

        locked([&]() {
            auto it = c_.find(id);

            if (it != c_.end()) {
                handler(tags::on_item_removed)(id, it->second, rep);
                c_.erase(id);
                save();
            } else {
                throw NotFound;
            }
        });
    };
}

void
json_collection::load_collection(const jsonv::value& c)
{
    try {
        for (auto& v : c.as_array()) {
            auto it = v.find("id");

            if (it == v.end_object()) {
                continue;
            }

            id_type id = it->second.as_string();
            c_[id] = v;
        }

        handler(tags::on_collection_loaded)(c);
    } catch (...) {
        c_.clear();
        throw;
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
