#pragma once

#include <sstream>

#include <rapidcheck.h>

// To support Catch2 v3 we check if the new header has already been included,
// otherwise we include the old header.
#ifndef CATCH_TEST_MACROS_HPP_INCLUDED
  #include "catch_amalgamated.hpp"
#endif

namespace rc {

/// For use with Catch2. Use this function wherever you would use a
/// SECTION for convenient checking of properties.
///
/// @param description  A description of the property.
/// @param testable     The object that implements the property (a callable).
/// @param verbose      Whether to print distribution info on success.
template <typename Testable>
void prop(const std::string &description, Testable &&testable, bool verbose = false) {
  SECTION(description) {
    const auto result = detail::checkTestable(std::forward<Testable>(testable));

    if (result.template is<SuccessResult>()) {
      if (verbose) {
        std::cout << "- " << description << std::endl;
        detail::printResultMessage(result, std::cout);
        std::cout << std::endl;
      }
      SUCCEED();
    } else {
      std::ostringstream ss;
      detail::printResultMessage(result, ss);
      INFO(ss.str() << "\n");
      FAIL();
    }
  }
}

} // namespace rc
