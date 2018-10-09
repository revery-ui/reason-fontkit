#include <stdio.h>

#include <caml/mlvalues.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "reglfw_image.h"

extern "C" {
    CAMLprim value
    caml_hello() {
        printf("Hey from c, again");

        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            printf("ERROR INITING FREETYPE\n");
        }

        FT_Face face;
        if (FT_New_Face(ft, "Roboto-Regular.ttf", 0, &face)) {
            printf("Error loading font\n");
        }

        FT_Set_Pixel_Sizes(face, 0, 48);

        if (FT_Load_Char(face, 'A', FT_LOAD_RENDER)) {
            printf("Error rendering glyph");
        }

        ReglfwImageInfo *p = (ReglfwImageInfo *)malloc(sizeof(ReglfwImageInfo));
        p->numChannels = 1;
        p->channelSize = 1;
        p->width = face->glyph->bitmap.width;
        p->height = face->glyph->bitmap.rows;
        p->data = face->glyph->bitmap.buffer;

        return (value)p;
    }
}
