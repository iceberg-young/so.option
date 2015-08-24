/**
 * @copyright 2015 Iceberg YOUNG
 * @license GNU Lesser General Public License version 3
 */

#include "option_target.hpp"

namespace so {
    namespace {
        using error_type = option_parse_error::error_type;

        std::string lower_ascii(const std::string& s) {
            std::string t;
            t.reserve(s.size());
            for (char c : s) {
                t += std::tolower(c);
            }
            return t;
        }

        bool boolean_value(const std::string& value) {
            auto la = lower_ascii(value);
            if (la == "yes" or la == "y" or la == "true" or la == "t") {
                return true;
            }
            if (la == "no" or la == "n" or la == "false" or la == "f") {
                return false;
            }
            throw option_parse_error{error_type::not_boolean, value};
        }

        double number_value(const std::string& value) {
            char* e = nullptr;
            double n = std::strtod(value.c_str(), &e);
            if (not *e) return n;
            throw option_parse_error{error_type::not_number, value};
        }

        json option_value(json::content_type type, const std::string& value) {
            switch (type) {
                case json::content_type::boolean: {
                    return boolean_value(value);
                }
                case json::content_type::number: {
                    return number_value(value);
                }
                case json::content_type::string: {
                    return value;
                }
                default: {
                    throw type;
                }
            }
        }

        json::content_type container_type(char t) {
            switch (t) {
                case 0:
                case 'b':
                case 'n':
                case 's': {
                    return json::content_type::null;
                }
                case 'a': {
                    return json::content_type::array;
                }
                case 'o': {
                    return json::content_type::object;
                }
            }
            throw option_parse_error{error_type::invalid_container_type, t};
        }

        json::content_type element_type(char t) {
            switch (t) {
                case 0: {
                    return json::content_type::null;
                }
                case 'b': {
                    return json::content_type::boolean;
                }
                case 'n': {
                    return json::content_type::number;
                }
                case 's':
                case 'a':
                case 'o': {
                    return json::content_type::string;
                }
            }
            throw option_parse_error{error_type::invalid_element_type, t};
        }
    }

    void option_target::initialize(const std::string& name, const json* schema, json* result) {
        auto& o = schema->as_object();
        auto f = o.find(option_key::value);
        std::string s = f != o.end() ? f->second : "";
        this->element = element_type(s.back());
        if (this->element == json::content_type::null) {
            this->clear();
            return;
        }
        this->container = container_type(s.front());
        this->name = name;
        this->result = result;
        this->set_default_value(o);
    }

    void option_target::assign(const std::string& value) {
        if (not this->is_set()) {
            throw option_parse_error{error_type::futile_value, value};
        }

        auto& r = (*this->result)[this->name];
        switch (this->container) {
            case json::content_type::null: {
                r = option_value(this->element, value);
                this->clear();
                break;
            }
            case json::content_type::array: {
                r.as_array().emplace_back(option_value(this->element, value));
                break;
            }
            case json::content_type::object: {
                auto p = value.find('=');
                if (p == std::string::npos) {
                    throw option_parse_error{error_type::not_kv, value};
                }
                auto v = option_value(this->element, value.substr(p + 1));
                r.as_object().emplace(
                  std::make_pair(value.substr(0, p), std::move(v))
                );
                break;
            }
            default: {
                throw this->container;
            }
        }
    }

    void option_target::clear() {
        this->container = json::content_type::null;
        this->element = json::content_type::string;
        this->name.erase();
        this->result = nullptr;
    }

    void option_target::close() {
        if (this->is_set() and this->container == json::content_type::null) {
            throw option_parse_error{error_type::absent_value, this->name};
        }
        this->clear();
    }

    void option_target::set_default_value(const json::object_t& schema) {
        auto& o = this->result->as_object();
        if (o.find(this->name) != o.end()) return;

        auto f = schema.find("default");
        if (f != schema.end()) {
            o.emplace(std::make_pair(this->name, f->second));
        }
        else if (this->container != json::content_type::null) {
            o.emplace(std::make_pair(this->name, this->container));
        }
        else if (this->element != json::content_type::boolean) {
            o.emplace(std::make_pair(this->name, this->element));
        }
        else {
            o.emplace(std::make_pair(this->name, true));
        }
    }
}
