open Bigarray;

exception FontKitLoadFaceException(string);
exception FontKitRenderGlyphException(string);

type fk_return('a) =
  | Success('a)
  | Error(string);

type fk_face;
type successCallback = fk_face => unit;
type failureCallback = string => unit;

type fk_glyph = {
  width: int,
  height: int,
  bearingX: int,
  bearingY: int,
  advance: int,
  bitmap:
    Array2.t(int, int8_unsigned_elt, c_layout),
};

type fk_metrics = {
  /*
   * This represents the 'height' FT_Global_Metric -
   * From FreeType2 documentation:
   * https://www.freetype.org/freetype2/docs/tutorial/step2.html
   *
   * Essentially, a default line spacing for the font.
   */
  height: int,
  /*
   * Vertical distance from the horizontal baseline to the highest 'character' coordinate
   */
  ascent: int,
  /*
   * Vertical distance from the horizontal baseline to the lowest 'character' coordinate
   */
  descent: int,
  /*
   * Vertical position, relative to the baseline, of the undelrine bar's center.
   * Negative if below baseline
   */
  underlinePosition: int,
  /*
   * When displaying or rendering underlined text, this value corresponds to the
   * vertical thickness of the underline.
   */
  underlineThickness: int,
  /*
   * unitsPerEm
   */
  unitsPerEm: int,
  /*
   * The integer size passed in when loading the font
   */
  size: int,
};

type fk_shape = {
  glyphId: int,
  cluster: int,
};

external fk_new_face: (string, int, successCallback, failureCallback) => unit =
  "caml_fk_new_face";
external fk_load_glyph: (fk_face, int) => fk_return(fk_glyph) =
  "caml_fk_load_glyph";
external fk_shape: (fk_face, string) => array(fk_shape) = "caml_fk_shape";
external fk_dummy_font: int => fk_face = "caml_fk_dummy_font";

external fk_get_metrics: fk_face => fk_metrics = "caml_fk_get_metrics";

let dummyFont = fk_dummy_font;

let load = (fontFile, size) => {
  let (promise, resolver) = Lwt.task();

  let success = face => Lwt.wakeup_later(resolver, face);
  let failure = message =>
    Lwt.wakeup_later_exn(resolver, FontKitLoadFaceException(message));

  fk_new_face(fontFile, size, success, failure);
  promise;
};

let renderGlyph = (face, glyphId) => {
  let glyph = fk_load_glyph(face, glyphId);
  switch (glyph) {
  | Success(g) => g
  | Error(msg) => raise(FontKitRenderGlyphException(msg))
  };
};
