#include "StdPolyfills.h"

struct lconv s_lconv;

struct lconv *localeconv_polyfill() {
    return &s_lconv;
}