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

        void clear();

     public:
        bool is_set() const {
            return this->result != nullptr;
        }

     private:
        json::content_type container;
        json::content_type element;
        std::string name;
        json* result;
    };
}
