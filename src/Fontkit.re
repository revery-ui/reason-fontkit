open Reglfw;

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
image: Image.t
};

type fk_shape = {
    codepoint: int,
    cluster: int
};

external fk_new_face: (string, int, successCallback, failureCallback) => unit = "caml_fk_new_face";
external fk_load_glyph: (fk_face, int) => fk_return(fk_glyph) = "caml_fk_load_glyph";
external fk_shape: (fk_face, string) => array(fk_shape) = "caml_fk_shape";

let load = (fontFile, size) => {
  let (promise, resolver) = Lwt.task();

  let success = face => Lwt.wakeup_later(resolver, face);
  let failure = message =>
    Lwt.wakeup_later_exn(resolver, FontKitLoadFaceException(message));

  fk_new_face(fontFile, size, success, failure);
  promise;
};

let renderGlyph = (face, size) => {
    let glyph = fk_load_glyph(face, size);
    switch (glyph) {
    | Success(g) => g
    | Error(msg) => raise(FontKitRenderGlyphException(msg))
}
};
