/**
 * @copyright 2015 Iceberg YOUNG
 * @license GNU Lesser General Public License version 3
 */

#include "option_parser.hpp"

namespace so {
    using error_type = option_parse_error::error_type;

    void option_parser::push(const std::string& fragment) {
        if (fragment.empty() or fragment[0] != '-') {
            this->selected.is_set()
              ? this->selected.assign(fragment)
              : this->parse_command(fragment);
            return;
        }
        if (fragment.size() == 1) { // "-"
            this->selected.is_set()
              ? this->selected.assign(fragment)
              : this->fallback.assign(fragment);
            return;
        }
        this->selected.clear();
        if (fragment[1] != '-') {
            this->scan_abbr(fragment);
            return;
        }
        if (fragment.size() != 2) { // "--"
            auto p = fragment.find('=');
            this->parse_option(fragment.substr(2, p));
            if (p != std::string::npos) {
                this->selected.assign(fragment.substr(p + 1));
            }
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
                this->fallback.assign(name);
            }
            return;
        }
        this->add_command(name);
        this->step_in();
    }

    void option_parser::parse_option(const std::string& name) {
        try {
            this->add_option(name, this->selected);
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

    void option_parser::scan_abbr(const std::string& fragment) {
        for (size_t i = 1; i < fragment.size(); ++i) {
            char abbr = fragment[i];
            switch (abbr) {
                case '=': {
                    this->selected.assign(fragment.substr(i + 1));
                    return;
                }
                case '^': {
                    this->step_out();
                    continue;
                }
            }
            if (this->selected.is_set()) {
                throw option_parse_error{error_type::incomplete_abbr, abbr};
            }
            this->parse_option(abbr);
        }
    }

    void option_parser::step_in() {
        this->selected.clear();

        try {
            this->add_option("", this->fallback);
        }
        catch (std::out_of_range) {
            this->fallback.clear();
        }
    }

    void option_parser::step_out() {
        if (this->schema.size() > 1) {
            this->schema.pop();
        }
        if (this->result.size() > 1) {
            this->result.pop();
        }
        this->fallback.clear();
        this->selected.clear();
    }

    void option_parser::add_command(const std::string& command) {
        auto& a = this->get_result(option_key::command).as_array();
        a.emplace_back(json{
          {
            {option_key::name, command},
            {option_key::option, json::content_type::object}
          }
        });
        this->result.push(&a.back());
    }

    void option_parser::add_option(const std::string& option, option_target& target) {
        auto& s = this->get_option(option);
        auto& r = this->get_result(option_key::option);
        target.initialize(option, &s, &r);
    }
}
