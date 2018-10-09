#include <stdio.h>

#include <caml/mlvalues.h>

#include <ft2build.h>
#include FT_FREETYPE_H

extern "C" {
    CAMLprim value
    caml_hello() {
        printf("Hey from c, again");

        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            printf("ERROR INITING FREETYPE\n");
        }

        FT_Face face;
        if (FT_New_Face(ft, "arial.ttf", 0, &face)) {
            printf("Error loading font\n");
        }

        return Val_unit;
    }
}
