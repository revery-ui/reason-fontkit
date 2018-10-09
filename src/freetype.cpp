#include <stdio.h>

#include <caml/mlvalues.h>

/* #include <ft2build.h> */
/* #include FT_FREETYPE_H */

extern "C" {
    CAMLprim value
    caml_hello() {
        printf("Hey from c");
        return Val_unit;
    }
}
