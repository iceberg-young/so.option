/**
 * @copyright 2015 Iceberg YOUNG
 * @license GNU Lesser General Public License version 3
 */

#include "option.hpp"
#include "option_parser.hpp"
#include "setting.hpp"

namespace so {
    using content_type = json::content_type;
    using error_type = option_parse_error::error_type;

    namespace {
        content_type deduce_type(const json& option, bool sub) {
            auto s = option[option_key::value].to_string();
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

        json default_option() {
            return {
              {
                {option_key::value, ""},
                {"required", false}
              }
            };
        }
    }

    void option_parser::push(const std::string& fragment) {
        if (this->option) {
            this->push_option(fragment);
        }
        else if (fragment.empty() or fragment[0] != '-') {
            this->parse_command(fragment);
        }
        else if (fragment.size() == 1) { // "-"
            this->push_default_option(fragment);
        }
        else if (fragment[1] != '-') {
            this->scan_abbr(fragment);
        }
        else if (fragment.size() == 2) { // "--"
            this->option = nullptr;
        }
        else {
            this->parse_option(fragment.substr(2));
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
                this->push_default_option(name);
            }
            return;
        }
        this->set_command(name);
    }

    void option_parser::parse_option(const std::string& name) {
        try {
            json copy = this->get_option(name);
            merge(copy, default_option());
            if (this->set_option(name, deduce_type(copy, false))) {
                this->option = &copy;
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

    void option_parser::push_default_option(const std::string& value) {
        // TODO
    }

    void option_parser::push_option(const std::string& value) {
        // this->option = nullptr; // TODO if type not array
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
            {option_key::option, json::content_type::array}
          }
        });
        this->result.push(&a.back());
    }

    bool option_parser::set_option(const std::string& name, json::content_type type) {
        auto& a = (*this->result.top())[option_key::option].as_array();
        a.emplace_back(json{
          {
            {option_key::name, name},
            {option_key::value, type}
          }
        });
        if (type != content_type::null) {
            this->result.push(&a.back());
            return true;
        }
        return false;
    }

    void option_parser::step_out() {
        this->option = nullptr;
        if (this->schema.size() > 1) {
            this->schema.pop();
        }
        if (this->result.size() > 1) {
            this->result.pop();
        }
    }
}
