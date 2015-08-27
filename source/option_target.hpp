/**
 * @copyright 2015 Iceberg YOUNG
 * @license GNU Lesser General Public License version 3
 */
#pragma once

#include "option.hpp"
#include "json.hpp"

namespace so {
    class option_target {
     public:
        option_target() :
          container(json::content_type::null),
          element(json::content_type::string),
          result(nullptr) {}

     public:
        void initialize(const std::string& name, const json* schema, json* result);

        void assign(const std::string& value);

        void close();

     public:
        bool empty() const {
            return this->result == nullptr
              or this->element == json::content_type::null;
        }

     protected:
        void assign_default(const json::object_t& schema);

        void clear() {
            this->assigned = false;
            this->container = json::content_type::null;
            this->element = json::content_type::string;
            this->name.erase();
            this->result = nullptr;
        }

     private:
        bool assigned;
        json::content_type container;
        json::content_type element;
        std::string name;
        json* result;
    };
}
