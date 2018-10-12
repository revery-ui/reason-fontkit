open Reglfw;

exception FontKitLoadFaceException(string);
exception FontKitRenderGlyphException(string);

type fk_return('a) =
| Success('a)
| Error(string);

type fk_face;

type fk_glyph = {
width: int,
height: int,
bearingX: int,
bearingY: int,
advance: int,
image: Image.t
};

external fk_new_face: (string, int) => fk_return(fk_face) = "caml_fk_new_face";

external fk_load_glyph: (fk_face, int) => fk_return(fk_glyph) = "caml_fk_load_glyph";

let load = (fontFile, size) => {
    let face = fk_new_face(fontFile, size);
    switch (face) {
    | Success(f) => f
    | Error(msg) => raise(FontKitLoadFaceException(msg))
    }
};

let renderGlyph = (face, size) => {
    let glyph = fk_load_glyph(face, size);
    switch (glyph) {
    | Success(g) => 
        let {width, height, image, advance, _} = g;
        
        print_endline ("HEY!" ++ string_of_int(width) ++ string_of_int(height) ++ "|" ++ string_of_int(advance));
        image;
    | Error(msg) => raise(FontKitRenderGlyphException(msg))
}
};
