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

        void done() {
            this->step_out(-1);
        }

     protected:
        void parse_command(const std::string& name);

        void parse_option(const std::string& name);

        void parse_option(char abbr);

        void scan_abbr(const std::string& fragment);

     protected:
        void step_in();

        void step_out(size_t level);

        void verify() const;

     protected:
        bool find(const json::object_t& set, const std::string& key, const json*& sub) const;

        bool find_schema(const std::string& key, const json*& sub) const {
            return this->find(this->schema.top()->as_object(), key, sub);
        }

        bool find_schema(const std::string& key, const std::string& name, const json*& sub) const {
            return this->find_schema(key, sub)
              and this->find(sub->as_object(), name, sub);
        }

        bool find_abbr_schema(char abbr, const json*& sub) const {
            return this->find_schema(option_key::abbr, std::string{abbr}, sub);
        }

        bool find_alias_schema(const std::string& alias, const json*& sub) const {
            return this->find_schema(option_key::alias, alias, sub);
        }

        bool find_command_schema(const std::string& command, const json*& sub) const {
            return this->find_schema(option_key::command, command, sub);
        }

        bool find_option_schema(const std::string& option, const json*& sub) const {
            return this->find_schema(option_key::option, option, sub);
        }

        bool find_options_schema(const json::object_t*& sub) const;

     protected:
        json& get_result(const std::string& category) const {
            return (*this->result.top())(category);
        }

        json& get_commands() const {
            return this->get_result(option_key::command)
              .be(json::content_type::array);
        }

        json& get_options() const {
            return this->get_result(option_key::option)
              .be(json::content_type::object);
        }

        void add_command(const std::string& command);

        void add_option(const std::string& option, option_target& target, bool required);

     private:
        std::stack<const json*> schema;
        std::stack<json*> result;
        //std::stack<std::pair<const json*, json*>> level; //< TODO

        option_target fallback;
        option_target selected;
    };
}
