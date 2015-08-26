/**
 * @copyright 2015 Iceberg YOUNG
 * @license GNU Lesser General Public License version 3
 */

#include "option_parser.hpp"

namespace so {
    namespace {
        using error_type = option_parse_error::error_type;

        void verify_present(
          const json& schema,
          json::object_t& result,
          json::object_t::iterator& end,
          const std::string& name
        ) {
            if (is::boolean(schema)) {
                if (not schema) return;
                throw option_parse_error{error_type::required, name};
            }
            if (is::array(schema)) {
                for (auto& by : schema.as_array()) {
                    if (result.find(by) == end) continue;
                    // FIXME: option w/o a value is not presented
                    throw option_parse_error{
                      error_type::required_by,
                      name + '\t' + by.to_string()
                    };
                }
            }
        }
    }

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
        if (this->find_command_schema(name, s)) {
            this->add_command(s, name);
            this->step_in();
        }
        else if (this->find_alias_schema(name, s)) {
            this->parse_command(*s);
        }
        else {
            this->fallback.assign(name);
        }
    }

    void option_parser::parse_option(const std::string& name) {
        this->selected.close();
        this->add_option(name, this->selected, true);
    }

    void option_parser::parse_option(char abbr) {
        const json* s = nullptr;
        if (not this->find_abbr_schema(abbr, s)) {
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
                    this->step_out(1);
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

    void option_parser::step_out(size_t steps) {
        this->fallback.clear();
        this->selected.close();
        steps = std::min(steps, this->level.size() - 1);
        for (size_t i = 0; i < steps; ++i) {
            this->verify();
            this->level.pop();
        }
    }

    void option_parser::verify() const {
        const json::object_t* s = nullptr;
        if (not this->find_options_schema(s)) return;

        auto& r = this->get_options().as_object();
        auto e = r.end();
        for (auto& i : *s) {
            if (r.find(i.first) != e) continue;

            auto& o = i.second.as_object();
            auto x = o.find(option_key::required);
            if (x == o.end()) continue;

            verify_present(x->second, r, e, i.first);
        }
    }

    bool option_parser::find(const json::object_t& set, const std::string& key, const json*& sub) const {
        auto f = set.find(key);
        if (f != set.end()) {
            sub = &f->second;
            return true;
        }
        return false;
    }

    bool option_parser::find_options_schema(const json::object_t*& sub) const {
        const json* s = nullptr;
        if (this->find_schema(option_key::option, s)) {
            sub = &s->as_object();
            return not sub->empty();
        }
        return false;
    }

    void option_parser::add_command(const json* schema, const std::string& command) {
        auto& a = this->get_commands().as_array();
        a.emplace_back(json::object_t{
          {
            {option_key::name, command},
          }
        });
        this->level.push(std::make_pair(schema, &a.back()));
    }

    void option_parser::add_option(const std::string& option, option_target& target, bool required) {
        const json* s = nullptr;
        if (this->find_option_schema(option, s)) {
            target.initialize(option, s, &this->get_options());
        }
        else if (required) {
            throw option_parse_error{error_type::invalid_option, option};
        }
    }
}
