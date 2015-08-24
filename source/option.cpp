/**
 * @copyright 2015 Iceberg YOUNG
 * @license GNU Lesser General Public License version 3
 */

#include "option.hpp"
#include "option_parser.hpp"

namespace so {
    option::option(const json& schema, char** fragments, int count) {
        option_parser p{&schema, this};
        for (int i = 0; i < count; ++i) {
            p.push(fragments[i]);
        }
        p.done();
    }
}
