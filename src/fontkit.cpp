#include <stdio.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/bigarray.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ot.h>

/* #define TEST_FONT "E:/FiraCode-Regular.ttf" */
/* #define TEST_FONT "E:/Hasklig-Medium.otf" */
/* #define TEST_FONT "E:/Cassandra.ttf" */
/* #define TEST_FONT "E:/Lato-Regular.ttf" */

extern "C" {

    static FT_Library _ftLibrary;
    static bool _fHasInitedLibrary = false;
    static char* dummyData = (char*) calloc(144, sizeof(char));

    struct FontKitFace {
        FT_Face* pFreeTypeFace;
        hb_font_t* pHarfBuzzFace;
        int iSize;
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
        CAMLreturn(error);
    }

    bool is_dummy(char *face) {
        char maybeDummy[8];
        strncpy(maybeDummy, face, 5);
        maybeDummy[5] = 0;
        return strcmp(maybeDummy,"dummy") == 0;
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
    caml_fk_new_face(value vString, value vInt, value vSuccess, value vFailure)  {
        CAMLparam4(vString, vInt, vSuccess, vFailure);
        CAMLlocal1(ret);

        if (!_fHasInitedLibrary) {
            if(FT_Init_FreeType(&_ftLibrary)) {
                caml_callback(vFailure, caml_copy_string("[ERROR]: Initializing freetype library failed\n"));
                CAMLreturn(Val_unit);
            }
            _fHasInitedLibrary = true;
        }

        FT_Library ft = _ftLibrary;

        const char *szFont = String_val(vString);
        int iSize = Int_val(vInt);

        FT_Face *face = (FT_Face *)malloc(sizeof(FT_Face));

        if (FT_New_Face(ft, szFont, 0, face)) {
            caml_callback(vFailure, caml_copy_string("[ERROR]: Unable to load font at FT_New_Face\n"));
        } else {
            FT_Set_Pixel_Sizes(*face, 0, iSize);

            hb_font_t *hb_font;
            hb_font = get_font_ot(szFont, iSize * 64);

            FontKitFace* pFontKitFace = (FontKitFace *)malloc(sizeof(FontKitFace));
            pFontKitFace->pFreeTypeFace = face;
            pFontKitFace->pHarfBuzzFace = hb_font;
            pFontKitFace->iSize = iSize;

            caml_callback(vSuccess, (value)pFontKitFace);
        }
        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_fk_load_glyph(value vFace, value vGlyphId) {
        CAMLparam2(vFace, vGlyphId);
        CAMLlocal3(ret, record, bitmapBigarray);

        if (is_dummy((char *)vFace)) {
            bitmapBigarray = caml_ba_alloc_dims(CAML_BA_UINT8 | CAML_BA_C_LAYOUT, 2, dummyData, 12, 12);
            record = caml_alloc(6, 0);
            Store_field(record, 0, 12);
            Store_field(record, 1, 12);
            Store_field(record, 2, 0);
            Store_field(record, 3, 0);
            Store_field(record, 4, 0);
            Store_field(record, 5, bitmapBigarray);

            ret = Val_success(record);
        } else {
            FontKitFace *pFontKitFace = (FontKitFace *)vFace;
            long glyphId = Int_val(vGlyphId);

            FT_Face face = *(pFontKitFace->pFreeTypeFace);

            if (FT_Load_Glyph(face, glyphId, FT_LOAD_RENDER)) {
                ret = Val_error("[ERROR]: Unable to render character at FT_Load_Char\n");
            } else {
                FT_Bitmap bitmap = face->glyph->bitmap;
                int bitmapDataLength = bitmap.rows * abs(bitmap.pitch);
                char* c = (char *)malloc(sizeof(char) * bitmapDataLength);
                bitmapBigarray = caml_ba_alloc_dims(CAML_BA_UINT8 | CAML_BA_C_LAYOUT, 2, (void *)c, bitmap.rows, abs(bitmap.pitch));
                memcpy(Caml_ba_data_val(bitmapBigarray), (const char *)bitmap.buffer, bitmapDataLength);

                record = caml_alloc(6, 0);
                Store_field(record, 0, Val_int(bitmap.width));
                Store_field(record, 1, Val_int(bitmap.rows));
                Store_field(record, 2, Val_int(face->glyph->bitmap_left));
                Store_field(record, 3, Val_int(face->glyph->bitmap_top));
                Store_field(record, 4, Val_int(face->glyph->advance.x));
                Store_field(record, 5, bitmapBigarray);

                ret = Val_success(record);
            }
        }
        CAMLreturn(ret);
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
        CAMLlocal1(ret);

        FontKitFace *pFontKitFace = (FontKitFace *)vFace;

        hb_buffer_t* hb_buffer;
        hb_buffer = hb_buffer_create();
        hb_buffer_add_utf8(hb_buffer, String_val(vString), -1, 0, -1);
        hb_buffer_guess_segment_properties(hb_buffer);

        unsigned int len = hb_buffer_get_length(hb_buffer);
        hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);

        ret = caml_alloc(len, 0);

        if (is_dummy((char *)vFace)) {
            for(int i = 0; i < len; i++) {
                Store_field(ret, i, createShapeTuple(0, 0));
            }
            hb_buffer_destroy(hb_buffer);
            CAMLreturn(ret);
        } else {
            hb_font_t *hb_font = pFontKitFace->pHarfBuzzFace;
            hb_shape(hb_font, hb_buffer, NULL, 0);

            for(int i = 0; i < len; i++) {
                Store_field(ret, i, createShapeTuple(info[i].codepoint, info[i].cluster));
            }
            hb_buffer_destroy(hb_buffer);
            CAMLreturn(ret);
        }

    }

    CAMLprim value
    caml_fk_get_metrics(value vFace) {
        CAMLparam1(vFace);
        CAMLlocal1(ret);

        ret = caml_alloc(7, 0);

        if (is_dummy((char *)vFace)) {
            Store_field(ret, 0, Val_int(1));
            Store_field(ret, 1, Val_int(1));
            Store_field(ret, 2, Val_int(1));
            Store_field(ret, 3, Val_int(1));
            Store_field(ret, 4, Val_int(1));
            Store_field(ret, 5, Val_int(1));
            Store_field(ret, 6, Val_int(1));
        } else {
            FontKitFace *pFontKitFace = (FontKitFace *)vFace;
            FT_Face* pFreeTypeFace = pFontKitFace->pFreeTypeFace;

            int lineGap = -1;
            int ascent = -1;
            int descent = -1;
            int underlinePosition = 0;
            int underlineThickness = 1;
            int unitsPerEm = 1;
            int size = pFontKitFace->iSize;

            FT_Face face = *pFreeTypeFace;

            if (FT_IS_SCALABLE(face)) {
                lineGap = face->height;
                ascent = face->ascender;
                descent = face->descender;
                underlinePosition = face->underline_position;
                underlineThickness = face->underline_thickness;
                unitsPerEm = face->units_per_EM;
            }

            Store_field(ret, 0, Val_int(lineGap));
            Store_field(ret, 1, Val_int(ascent));
            Store_field(ret, 2, Val_int(descent));
            Store_field(ret, 3, Val_int(underlinePosition));
            Store_field(ret, 4, Val_int(underlineThickness));
            Store_field(ret, 5, Val_int(unitsPerEm));
            Store_field(ret, 6, Val_int(size));
        }

        CAMLreturn(ret);
    }

    CAMLprim value
    caml_fk_dummy_font() {
        CAMLparam0();
        CAMLlocal1(ret);

        FT_Face *face = (FT_Face *)malloc(sizeof(FT_Face));
        char *faceChar = (char *)face;
        faceChar[0] = 'd';
        faceChar[1] = 'u';
        faceChar[2] = 'm';
        faceChar[3] = 'm';
        faceChar[4] = 'y';
        ret = (value)face;
        CAMLreturn(ret);
    }
}
