/**
 * @copyright 2015 Iceberg YOUNG
 * @license GNU Lesser General Public License version 3
 */
#pragma once

#include "json.hpp"

namespace so {
    class option :
      public json {
     public:
        static std::string usage(const json& schema);

     public:
        option(const json& schema, char** fragments, int count);
    };

    class option_parse_error :
      public std::domain_error {
     public:
        enum class error_type {
            incomplete_abbr,
            invalid_abbr,
            invalid_option,
            invalid_value_type,
        };

     public:
        option_parse_error(error_type type, const std::string& what) :
          domain_error(what),
          type(type) {}

     public:
        option_parse_error(error_type type, char abbr) :
          domain_error(std::string{abbr}),
          type(type) {}

     public:
        const error_type type;
    };

    namespace option_key {
        constexpr char abbr[]{"abbr"};
        constexpr char alias[]{"alias"};
        constexpr char command[]{"command"};
        constexpr char description[]{"description"};
        constexpr char option[]{"option"};

        constexpr char name[]{"name"};
        constexpr char value[]{"value"};
    }
}
