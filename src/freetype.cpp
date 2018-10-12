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
#define TEST_FONT "E:/Lato-Regular.ttf"

extern "C" {

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

    CAMLprim value
    caml_ft_init() {
        FT_Library *ft = (FT_Library *)malloc(sizeof(FT_Library));
        if (FT_Init_FreeType(ft)) {
            return Val_error("[ERROR]: Initialization failed at FT_Init_FreeType\n");
        } else {
            return Val_success((value)ft);
        }
    }

    CAMLprim value
    caml_ft_new_face(value vFreeType, value vString, value vInt)  {
        CAMLparam3(vFreeType, vString, vInt);

        FT_Library* ft = (FT_Library *)vFreeType;
        /* const char *szFont = String_val(vString); */
        const char *szFont = TEST_FONT;
        printf("- Loading font: %s\n", szFont);
        int iSize = Int_val(vInt);

        FT_Face *face = (FT_Face *)malloc(sizeof(FT_Face));

        if (FT_New_Face(*ft, szFont, 0, face)) {
            return Val_error("[ERROR]: Unable to load font at FT_New_Face\n");
        }

        FT_Set_Pixel_Sizes(*face, 0, iSize);

        return Val_success((value)face);
    }

    CAMLprim value
    caml_ft_load_char(value vFace, value vChar) {
        CAMLparam2(vFace, vChar);

        FT_Face face = *((FT_Face *)vFace);
        char c = (char)Int_val(vChar);

        printf("Trying to load");
        if (FT_Load_Glyph(face, 262, FT_LOAD_RENDER)) {
            return Val_error("[ERROR]: Unable to render character at FT_Load_Char\n");
        }
        printf("Loaded char");

        FontCharacterInfo* info = (FontCharacterInfo *)malloc(sizeof(FontCharacterInfo));
        info->bearingX = face->glyph->bitmap_left;
        info->bearingY = face->glyph->bitmap_top;
        info->advance = face->glyph->advance.x >> 6; // Divide advance by 64 to get pixels

        printf("Got some info - width: %d height: %d\n", face->glyph->bitmap.width, face->glyph->bitmap.rows);

        // TODO: Copy buffer?
        ReglfwImageInfo *p = (ReglfwImageInfo *)malloc(sizeof(ReglfwImageInfo));
        p->numChannels = 1;
        p->channelSize = 1;
        p->width = face->glyph->bitmap.width;
        p->height = face->glyph->bitmap.rows;
        p->data = face->glyph->bitmap.buffer;


        info->image = p;
        printf("set image");

        CAMLreturn(Val_success((value)info));
    }

    CAMLprim value
    caml_ft_char_get_bearing(value vFontCharInfo) {
        return Val_unit;
    }

    CAMLprim value
    caml_ft_char_get_image(value vFontCharInfo) {
        FontCharacterInfo *pInfo = (FontCharacterInfo *)vFontCharInfo;
        printf("getting image...\n");
        return (value)pInfo->image;
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
    caml_hello() {

        printf("Hey from c, again");
          hb_font_t *hb_font;
          hb_font = get_font_ot (TEST_FONT, 18*64);

          printf("Loading harfbuzz");
  /* Create hb-buffer and populate. */
  hb_buffer_t *hb_buffer;
  hb_buffer = hb_buffer_create ();
  hb_buffer_add_utf8 (hb_buffer, "ffi help !== help!", -1, 0, -1);
  hb_buffer_guess_segment_properties (hb_buffer);

  int cluster_level = hb_buffer_get_cluster_level(hb_buffer);
  printf ("- Cluster level: %d\n", cluster_level);

  /* Shape it! */
  hb_shape (hb_font, hb_buffer, NULL, 0);

  /* Get glyph information and positions out of the buffer. */
  unsigned int len = hb_buffer_get_length (hb_buffer);
  hb_glyph_info_t *info = hb_buffer_get_glyph_infos (hb_buffer, NULL);
  hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);


  /* Print them out as is. */
  printf ("Raw buffer contents:\n");
  for (unsigned int i = 0; i < len; i++)
  {
    hb_codepoint_t gid   = info[i].codepoint;
    unsigned int cluster = info[i].cluster;
    double x_advance = pos[i].x_advance / 64.;
    double y_advance = pos[i].y_advance / 64.;
    double x_offset  = pos[i].x_offset / 64.;
    double y_offset  = pos[i].y_offset / 64.;

    char glyphname[32];
    hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof (glyphname));

    printf ("glyph='%s:%i'	cluster=%d	advance=(%g,%g)	offset=(%g,%g)\n",
            glyphname, gid, cluster, x_advance, y_advance, x_offset, y_offset);
  }

  printf ("Converted to absolute positions:\n");
  /* And converted to absolute positions. */
  {
    double current_x = 0;
    double current_y = 0;
    for (unsigned int i = 0; i < len; i++)
    {
      hb_codepoint_t gid   = info[i].codepoint;
      unsigned int cluster = info[i].cluster;
      double x_position = current_x + pos[i].x_offset / 64.;
      double y_position = current_y + pos[i].y_offset / 64.;


      char glyphname[32];
      hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof (glyphname));

      printf ("glyph='%s'	cluster=%d	position=(%g,%g)\n",
	      glyphname, cluster, x_position, y_position);

      current_x += pos[i].x_advance / 64.;
      current_y += pos[i].y_advance / 64.;
    }
  }
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
