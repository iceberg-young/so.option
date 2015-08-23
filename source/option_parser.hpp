/**
 * @copyright 2015 Iceberg YOUNG
 * @license GNU Lesser General Public License version 3
 */
#pragma once

#include "option_target.hpp"
#include <stack>

namespace so {
    class option_parser {
     public:
        option_parser(const json* schema, json* result) {
            this->schema.push(schema);
            this->result.push(result);
            this->step_in();
        }

     public:
        void push(const std::string& fragment);

     protected:
        void parse_command(const std::string& name);

        void parse_option(const std::string& name);

        void parse_option(char abbr);

        void scan_abbr(const std::string& fragment);

     protected:
        void step_in();

        void step_out();

     protected:
        const json& get_schema(const std::string& category) const {
            return (*this->schema.top())[category];
        }

        const json& get_abbr(char abbr) const {
            return this->get_schema(option_key::abbr)[std::string{abbr}];
        }

        const json& get_alias(const std::string& alias) const {
            return this->get_schema(option_key::alias)[alias];
        }

        const json& get_command(const std::string& command) const {
            return this->get_schema(option_key::command)[command];
        }

        const json& get_option(const std::string& option) const {
            return this->get_schema(option_key::option)[option];
        }

     protected:
        json& get_result(const std::string& category) const {
            return (*this->result.top())[category];
        }

        void add_command(const std::string& command);

        void add_option(const std::string& option, option_target& target);

     private:
        std::stack<const json*> schema;
        std::stack<json*> result;

        option_target fallback;
        option_target selected;
    };
}
