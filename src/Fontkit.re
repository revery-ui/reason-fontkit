exception FontKitLoadFaceException(string);
exception FontKitRenderGlyphException(string);

type fk_return('a) =
  | Success('a)
  | Error(string);

type fk_face;
type successCallback = fk_face => unit;
type failureCallback = string => unit;

type bitmap = Bigarray.Array2.t(int, Bigarray.int8_unsigned_elt, Bigarray.c_layout);

type fk_glyph = {
  width: int,
  height: int,
  bearingX: int,
  bearingY: int,
  advance: int,
  bitmap: bitmap,
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

let dummyFont = fk_dummy_font;

let load = (fontFile, size) => {
  let (promise, resolver) = Lwt.task();

  let success = face => Lwt.wakeup_later(resolver, face);
  let failure = message =>
    Lwt.wakeup_later_exn(resolver, FontKitLoadFaceException(message));

  fk_new_face(fontFile, size, success, failure);
  promise;
};
let debugImageIndex = ref(0);

let _saveDebugImage = pixels => {
  open Bigarray;
  /* open Reglfw; */
  let width = Array2.dim2(pixels);
  let height = Array2.dim1(pixels);
  let debugImagePixels =
    Array2.create(
      int8_unsigned,
      c_layout,
      height,
      width * 4 /* RGBA */
    );
  Array2.fill(debugImagePixels, 0);
  for (y in 0 to height - 1) {
    for (x in 0 to width - 1) {
      Array2.set(debugImagePixels, y, x * 4, Array2.get(pixels, y, x));
      Array2.set(debugImagePixels, y, x * 4 + 1, Array2.get(pixels, y, x));
      Array2.set(debugImagePixels, y, x * 4 + 2, Array2.get(pixels, y, x));
      Array2.set(debugImagePixels, y, x * 4 + 3, Array2.get(pixels, y, x));
    };
  };
  let debugImage = Reglfw.Image.create(debugImagePixels);
  Reglfw.Image.save(
    debugImage,
    "debugImage" ++ string_of_int(debugImageIndex^) ++ ".tga",
  );
  debugImageIndex := debugImageIndex^ + 1;
};

module Memoize = {
  type t('a, 'b) = 'a => 'b;

  let make = (f: t('a, 'b)): t('a, 'b) => {
    let tbl: Hashtbl.t('a, 'b) = Hashtbl.create(16);

    let ret = (arg: 'a) => {
      let cv = Hashtbl.find_opt(tbl, arg);
      switch (cv) {
      | Some(x) => x
      | None =>
        let r = f(arg);
        Hashtbl.add(tbl, arg, r);
        r;
      };
    };
    ret;
  };
};

let derps: ref(list(bitmap)) = ref([]);

let _renderGlyph = ((face, glyphId)) => {
  let glyph = fk_load_glyph(face, glyphId);
  print_endline("rendering glyph " ++ string_of_int(glyphId));
  switch (glyph) {
  | Success(g) =>
    /* saveDebugImage(g.bitmap); */
    derps := List.append([g.bitmap], derps^);
    g;
  | Error(msg) => raise(FontKitRenderGlyphException(msg))
  };
};

let _memoizedRenderGlyph = Memoize.make(_renderGlyph);

let renderGlyph = (face, glyphId) => _memoizedRenderGlyph((face, glyphId));
/* let renderGlyph = (face, glyphId) => _renderGlyph((face, glyphId)); */
