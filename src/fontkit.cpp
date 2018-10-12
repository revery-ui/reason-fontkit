#include <stdio.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ot.h>

#include "reglfw_image.h"

/* #define TEST_FONT "E:/FiraCode-Regular.ttf" */
/* #define TEST_FONT "E:/Hasklig-Medium.otf" */
/* #define TEST_FONT "E:/Cassandra.ttf" */
/* #define TEST_FONT "E:/Lato-Regular.ttf" */

extern "C" {

    static FT_Library _ftLibrary;
    static bool _fHasInitedLibrary = false;

    struct FontKitFace {
        FT_Face* pFreeTypeFace;
        hb_font_t* pHarfbuzzFace;
    };

    struct FontCharacterInfo {
        ReglfwImageInfo* image;
        int bearingX;
        int bearingY;
        int advance;
    };

    CAMLprim value
    Val_success(value v) {
        CAMLparam1(v);
        CAMLlocal1(some);
        some = caml_alloc(1, 0);
        Store_field(some, 0, v);
        CAMLreturn(some);
    }

    CAMLprim value
    Val_error(const char* szMsg) {
        CAMLparam0();
        CAMLlocal1(error);
        error = caml_alloc(1, 1);
        Store_field(error, 0, caml_copy_string(szMsg));
        return error;
        CAMLreturn(error);
    }

    /* Use native open type implementation to load font
      https://github.com/harfbuzz/harfbuzz/issues/255 */
    hb_font_t*
    get_font_ot(const char *filename, int size)
    {
      FILE* file = fopen(filename, "rb");
      fseek(file, 0, SEEK_END);
      unsigned int length = ftell(file);
      fseek(file, 0, SEEK_SET);

      char* data = (char *)malloc(length);
      fread(data, length, 1, file);
      fclose(file);

      hb_blob_t* blob = hb_blob_create(data, length, HB_MEMORY_MODE_WRITABLE, (void*)data, NULL);
      hb_face_t* face = hb_face_create(blob, 0);
      hb_font_t* font = hb_font_create(face);

      hb_ot_font_set_funcs(font);
      hb_font_set_scale(font, size, size);

      return font;
    }

    CAMLprim value
    caml_fk_new_face(value vString, value vInt)  {
        CAMLparam2(vString, vInt);

        if (!_fHasInitedLibrary) {
            if(FT_Init_FreeType(&_ftLibrary)) {
                return Val_error("[ERROR]: Initializing freetype library failed\n");
            }
            _fHasInitedLibrary = true;
        }

        FT_Library ft = _ftLibrary;

        const char *szFont = String_val(vString);
        printf("- Loading font: %s\n", szFont);
        int iSize = Int_val(vInt);

        FT_Face *face = (FT_Face *)malloc(sizeof(FT_Face));

        if (FT_New_Face(ft, szFont, 0, face)) {
            return Val_error("[ERROR]: Unable to load font at FT_New_Face\n");
        }

        FT_Set_Pixel_Sizes(*face, 0, iSize);

        hb_font_t *hb_font;
        hb_font = get_font_ot(szFont, iSize * 64);

        FontKitFace* pFontKitFace = (FontKitFace *)malloc(sizeof(FontKitFace));
        pFontKitFace->pFreeTypeFace = face;
        pFontKitFace->pHarfbuzzFace = hb_font;

        return Val_success((value)pFontKitFace);
    }

    CAMLprim value
    caml_fk_load_glyph(value vFace, value vGlyphId) {
        CAMLparam2(vFace, vGlyphId);

        FontKitFace *pFontKitFace = (FontKitFace *)vFace;
        long glyphId = Int_val(vGlyphId);

        FT_Face face = *(pFontKitFace->pFreeTypeFace);

        if (FT_Load_Glyph(face, glyphId, FT_LOAD_RENDER)) {
            return Val_error("[ERROR]: Unable to render character at FT_Load_Char\n");
        }

        ReglfwImageInfo *p = (ReglfwImageInfo *)malloc(sizeof(ReglfwImageInfo));
        p->numChannels = 1;
        p->channelSize = 1;
        p->width = face->glyph->bitmap.width;
        p->height = face->glyph->bitmap.rows;

        unsigned char *originalBuffer = face->glyph->bitmap.buffer;

        int size = face->glyph->bitmap.width * face->glyph->bitmap.rows;
        unsigned char* data = (unsigned char *)malloc(size);
        memcpy(data, originalBuffer, size);

        p->data = data;

        CAMLlocal1(ret);
        ret = caml_alloc(6, 0);
        Store_field(ret, 0, Val_int(p->width));
        Store_field(ret, 1, Val_int(p->height));
        Store_field(ret, 2, Val_int(face->glyph->bitmap_left));
        Store_field(ret, 3, Val_int(face->glyph->bitmap_top));
        Store_field(ret, 4, Val_int(face->glyph->advance.x));
        Store_field(ret, 5, (value)p);

        return Val_success(ret);
    }

    static value createShapeTuple(unsigned int codepoint, unsigned int cluster) {
        CAMLparam0();

        CAMLlocal1(ret);
        ret = caml_alloc(2, 0);
        Store_field(ret, 0, Val_int(codepoint));
        Store_field(ret, 1, Val_int(cluster));
        CAMLreturn(ret);
    }

    CAMLprim value
    caml_fk_shape(value vFace, value vString) {
        CAMLparam2(vFace, vString);

        FontKitFace *pFontKitFace = (FontKitFace *)vFace;
        hb_font_t *hb_font = pFontKitFace->pHarfbuzzFace;

        hb_buffer_t* hb_buffer;
        hb_buffer = hb_buffer_create();
        hb_buffer_add_utf8(hb_buffer, String_val(vString), -1, 0, -1);
        hb_buffer_guess_segment_properties(hb_buffer);

        hb_shape(hb_font, hb_buffer, NULL, 0);

        unsigned int len = hb_buffer_get_length(hb_buffer);
        hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);

        CAMLlocal1(ret);
        ret = caml_alloc(len, 0);
        for(int i = 0; i < len; i++) {
            Store_field(ret, i, createShapeTuple(info[i].codepoint, info[i].cluster));
        }

        hb_buffer_destroy(hb_buffer);
        CAMLreturn(ret);
    }
}
