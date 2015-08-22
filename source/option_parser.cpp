/**
 * @copyright 2015 Iceberg YOUNG
 * @license GNU Lesser General Public License version 3
 */

#include "option.hpp"
#include "option_parser.hpp"

namespace so {
    using content_type = json::content_type;
    using error_type = option_parse_error::error_type;

    namespace {
        content_type deduce_type(const json& option, bool sub) {
            auto f = option.as_object().find(option_key::value);
            std::string s = f != option.as_object().end() ? f->second : "";
            switch (sub ? s.front() : s.back()) {
                case 0: {
                    return content_type::null;
                }
                case 'b': {
                    return content_type::boolean;
                }
                case 'n': {
                    return content_type::number;
                }
                case 's': {
                    return content_type::string;
                }
                case 'a': {
                    return sub ? content_type::string : content_type::array;
                }
                case 'o': {
                    return sub ? content_type::string : content_type::object;
                }
                default: {
                    throw option_parse_error{error_type::invalid_value_type, s};
                }
            }
        }

        std::string ascii_lower(const std::string& s) {
            std::string t;
            t.reserve(s.size());
            for (char c : s) {
                t += std::tolower(c);
            }
            return t;
        }

        json option_value(const json& schema, const std::string value) {
            json j;
            switch (deduce_type(schema, true)) {
                case content_type::boolean: {
                    auto v = ascii_lower(value);
                    if (v == "yes" or v == "y" or v == "true" or v == "t") {
                        j = true;
                        break;
                    }
                    if (v == "no" or v == "n" or v == "false" or v == "f") {
                        j = false;
                        break;
                    }
                    throw option_parse_error{error_type::not_boolean, value};
                }
                case content_type::number: {
                    char* e = nullptr;
                    j = std::strtod(value.c_str(), &e);
                    if (*e) {
                        throw option_parse_error{error_type::not_number, value};
                    }
                    break;
                }
                case content_type::string: {
                    j = value;
                    break;
                }
                default: {
                    throw option_parse_error{error_type::futile_value, value};
                }
            }
            return j;
        }
    }

    void option_parser::push(const std::string& fragment) {
        if (fragment.empty() or fragment[0] != '-') {
            this->option
              ? this->push_option(fragment)
              : this->parse_command(fragment);
            return;
        }
        if (fragment.size() == 1) { // "-"
            this->push_option(fragment);
            return;
        }
        this->step_over();
        if (fragment[1] != '-') {
            this->scan_abbr(fragment);
            return;
        }
        if (fragment.size() != 2) { // "--"
            auto p = fragment.find('=');
            this->parse_option(fragment.substr(2, p));
            if (p == std::string::npos) return;

            if (not this->option) {
                throw option_parse_error{error_type::futile_value, fragment};
            }
            this->push_option(fragment.substr(p + 1));
        }
    }

    void option_parser::parse_command(const std::string& name) {
        try {
            this->schema.push(&this->get_command(name));
        }
        catch (std::out_of_range) {
            try {
                this->parse_command(this->get_alias(name));
            }
            catch (std::out_of_range) {
                this->push_option(name);
            }
            return;
        }
        this->set_command(name);
    }

    void option_parser::parse_option(const std::string& name) {
        try {
            auto& o = this->get_option(name);
            if (this->set_option(name, deduce_type(o, false))) {
                this->option = &o;
            }
        }
        catch (std::out_of_range) {
            throw option_parse_error{error_type::invalid_option, name};
        }
    }

    void option_parser::parse_option(char abbr) {
        try {
            this->parse_option(this->get_abbr(abbr).to_string());
        }
        catch (std::out_of_range) {
            throw option_parse_error{error_type::invalid_abbr, abbr};
        }
    }

    void option_parser::push_option(const std::string& value) {
        const json* o = nullptr;
        json* r = nullptr;
        if (this->option) {
            o = this->option;
            r = this->result.top();
        }
        else {
            o = &this->get_option(""); // FIXME catch out_of_range
            r = nullptr; // FIXME
        }
        if (is::array(*r)) {
            r->as_array().emplace_back(option_value(*o, value));
        }
        else if (is::object(*r)) {
            auto p = value.find('=');
            if (p == std::string::npos) {
                throw option_parse_error{error_type::incomplete_pair, value};
            }
            auto k = value.substr(0, p);
            auto v = option_value(*o, value.substr(p + 1));
            r->as_object().emplace(std::make_pair(k, std::move(v)));
        }
        else {
            *r = option_value(*o, value);
            this->step_over();
        }
    }

    void option_parser::scan_abbr(const std::string& fragment) {
        for (size_t i = 1; i < fragment.size(); ++i) {
            char abbr = fragment[i];
            switch (abbr) {
                case '=': {
                    this->push_option(fragment.substr(i + 1));
                    return;
                }
                case '^': {
                    this->step_out();
                    continue;
                }
            }
            if (this->option) {
                throw option_parse_error{error_type::incomplete_abbr, abbr};
            }
            this->parse_option(abbr);
        }
    }

    void option_parser::set_command(const std::string& name) {
        auto& a = (*this->result.top())[option_key::command].as_array();
        a.emplace_back(json{
          {
            {option_key::name, name},
            {option_key::option, content_type::array}
          }
        });
        this->result.push(&a.back());
    }

    bool option_parser::set_option(const std::string& name, content_type type) {
        auto& a = (*this->result.top())[option_key::option].as_array();
        a.emplace_back(json{
          {
            {option_key::name, name},
            {option_key::value, type} // FIXME default value
          }
        });
        if (type != content_type::null) {
            this->result.push(&a.back());
            return true;
        }
        return false;
    }

    void option_parser::step_out() {
        this->step_over();
        if (this->result.size() > 1) {
            this->result.pop();
        }
        if (this->schema.size() > 1) {
            this->schema.pop();
        }
    }

    void option_parser::step_over() {
        if (not this->option) return;

        this->option = nullptr;
        if (this->result.size() > 1) {
            this->result.pop();
        }
    }
}
