// ORDER MATTERS
//
// clang-format off
#include "idl/private_url_generated.h"
#include "idl/shortening_request_generated.h"
#include "idl/shortening_response_generated.h"
#include "idl/lookup_request_generated.h"
#include "idl/lookup_response_generated.h"
#include "idl/trusted_shortening_request_generated.h"
#include "idl/trusted_shortening_response_generated.h"
#include "idl/trusted_lookup_request_generated.h"
#include "idl/trusted_lookup_response_generated.h"
// clang-format on

//
// Unfortunately the build rules for flatc don't/cant #include the
// generated headers of corresponding to the *.fbs files that were
// included in the .fbs schema
//
//  ._________________.
//  |                 |
//  | private_url.fbs |
//  |                 |
//  ._________________.
//              ^
//              |
//              |
//          ┌──────────────────────────────┐
//          │                              |
//          │  shortening service          |
//          │  ├── lookup_request.fbs      |
//          │  ├── lookup_response.fbs     |
//          │  ├── shortening_request.fbs  |
//          │  ├── shortening_response.fbs |
//          │  │   ...                     |
//          └──────────────────────────────┘
//
//
//
//
