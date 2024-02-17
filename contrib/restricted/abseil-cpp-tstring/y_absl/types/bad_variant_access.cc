// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "y_absl/types/bad_variant_access.h"

#ifndef Y_ABSL_USES_STD_VARIANT

#include <cstdlib>
#include <stdexcept>

#include "y_absl/base/config.h"
#include "y_absl/base/internal/raw_logging.h"

namespace y_absl {
Y_ABSL_NAMESPACE_BEGIN

//////////////////////////
// [variant.bad.access] //
//////////////////////////

bad_variant_access::~bad_variant_access() = default;

const char* bad_variant_access::what() const noexcept {
  return "Bad variant access";
}

namespace variant_internal {

void ThrowBadVariantAccess() {
#ifdef Y_ABSL_HAVE_EXCEPTIONS
  throw bad_variant_access();
#else
  Y_ABSL_RAW_LOG(FATAL, "Bad variant access");
  abort();  // TODO(calabrese) Remove once RAW_LOG FATAL is noreturn.
#endif
}

void Rethrow() {
#ifdef Y_ABSL_HAVE_EXCEPTIONS
  throw;
#else
  Y_ABSL_RAW_LOG(FATAL,
               "Internal error in y_absl::variant implementation. Attempted to "
               "rethrow an exception when building with exceptions disabled.");
  abort();  // TODO(calabrese) Remove once RAW_LOG FATAL is noreturn.
#endif
}

}  // namespace variant_internal
Y_ABSL_NAMESPACE_END
}  // namespace y_absl

#else

// https://github.com/abseil/abseil-cpp/issues/1465
// CMake builds on Apple platforms error when libraries are empty.
// Our CMake configuration can avoid this error on header-only libraries,
// but since this library is conditionally empty, including a single
// variable is an easy workaround.
#ifdef __APPLE__
namespace y_absl {
Y_ABSL_NAMESPACE_BEGIN
namespace types_internal {
extern const char kAvoidEmptyBadVariantAccessLibraryWarning;
const char kAvoidEmptyBadVariantAccessLibraryWarning = 0;
}  // namespace types_internal
Y_ABSL_NAMESPACE_END
}  // namespace y_absl
#endif  // __APPLE__

#endif  // Y_ABSL_USES_STD_VARIANT
