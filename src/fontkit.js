// Important: The value `0` of the Success tag correspond to the ordering in
// which fk_return variants are declared in the type declaration
function createSuccessValue(value) {
  return [0, value];
}

// Provides: caml_fk_new_face
function caml_fk_new_face(fontName /*: string */, size /*: number */) {
  console.log(size);
  // var jsName = fontName.c;
  return createSuccessValue([size]);
}

// Provides: caml_fk_load_glyph
// Requires: caml_new_string
function caml_fk_load_glyph(face /*: fk_face */, glyphId /*: number */) {
  console.log(face);
  var img = new Image();
  img.src =
    "data:image/gif;base64,R0lGODlhAQABAIAAAP///wAAACwAAAAAAQABAAACAkQBADs=";
  return createSuccessValue([
    /* <empty> */ 0,
    /* width */ face[0],
    /* height */ face[0],
    /* bearingX */ 0,
    /* bearingY */ 0,
    /* advance */ 0,
    /* image */ img
  ]);
}

// Provides: caml_fk_shape
function caml_fk_shape(face /*: fk_face */, text /*: string */) {
  // TODO: Is there a way to do this with a function?
  // Instead of relying on internal representation
  var str = text.c;
  return str.split("").map(function mapper(_char) {
    return [/* codepoint */ 0, /* cluster */ 0];
  });
}
