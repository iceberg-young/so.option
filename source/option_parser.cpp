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
        if (fragment[1] != '-') {
            this->scan_abbr(fragment);
            return;
        }
        if (fragment.size() != 2) { // "--"
            auto p = fragment.find('=');
            this->parse_option(fragment.substr(2, p - 2));
            if (p != std::string::npos) {
                this->selected.assign(fragment.substr(p + 1));
            }
        }
    }

    void option_parser::parse_command(const std::string& name) {
        const json* s = nullptr;
        try {
            s = &this->get_command(name);
        }
        catch (std::out_of_range) {
            try {
                s = &this->get_alias(name);
            }
            catch (std::out_of_range) {
                this->fallback.assign(name);
            }
            this->parse_command(*s);
            return;
        }
        this->schema.push(s);
        this->add_command(name);
        this->step_in();
    }

    void option_parser::parse_option(const std::string& name) {
        this->selected.close();
        this->add_option(name, this->selected, true);
    }

    void option_parser::parse_option(char abbr) {
        const json* s = nullptr;
        try {
            s = &this->get_abbr(abbr);
        }
        catch (std::out_of_range) {
            throw option_parse_error{error_type::invalid_abbr, abbr};
        }
        this->parse_option(s->to_string());
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
            this->parse_option(abbr);
        }
    }

    void option_parser::step_in() {
        this->fallback.clear();
        this->add_option("", this->fallback, false);
        this->selected.close();
    }

    void option_parser::step_out() {
        if (this->schema.size() > 1) {
            this->schema.pop();
        }
        if (this->result.size() > 1) {
            this->result.pop();
        }
        this->fallback.clear();
        this->selected.close();
    }

    void option_parser::add_command(const std::string& command) {
        auto& a = this->get_commands().as_array();
        a.emplace_back(json::object_t{
          {
            {option_key::name, command},
          }
        });
        this->result.push(&a.back());
    }

    void option_parser::add_option(const std::string& option, option_target& target, bool required) {
        const json* s = nullptr;
        try {
            s = &this->get_option(option);
        }
        catch (std::out_of_range) {
            if (not required) return;
            throw option_parse_error{error_type::invalid_option, option};
        }
        target.initialize(option, s, &this->get_options());
    }
}
