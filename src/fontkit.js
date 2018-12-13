var DUMMY_FONT = "__FONTKIT__DUMMY__FONT__";
var isDummyFont = function(face) {
  return face.length === 2 && face[0] === DUMMY_FONT;
};

function createSuccessValue(value) {
  // Important: The value `0` of the Success tag correspond to the ordering in
  // which fk_return variants are declared in the type declaration
  return [0, value];
}

// Provides: caml_fk_new_face
function caml_fk_new_face(
  fontName /*: string */,
  size /*: number */,
  successCallback /*: face => void */,
  failureCallback /*: string => void */
) {
  fetch(fontName.c)
    .then(function toBlob(res) {
      return res.blob();
    })
    .then(function toBuffer(blob) {
      // From: https://github.com/feross/blob-to-buffer/blob/fe48e780ac95ebea27387cc8f6aa6db1ec735ad0/index.js
      return new Promise(function(resolve, reject) {
        var reader = new FileReader();
        function onLoadEnd(e) {
          reader.removeEventListener("loadend", onLoadEnd, false);
          if (e.error) reject(e.error);
          else resolve(joo_global_object.Buffer.from(reader.result));
        }
        reader.addEventListener("loadend", onLoadEnd, false);
        reader.readAsArrayBuffer(blob);
      });
    })
    .then(function loadFont(buffer) {
      var fontFace = joo_global_object.Fontkit.create(buffer);
      successCallback(fontFace);
    })
    .catch(function onError(error) {
      failureCallback(error.message);
    });
  return undefined;
}

// Provides: caml_fk_load_glyph
// Requires: caml_new_string
function caml_fk_load_glyph(face /*: fk_face */, codePoint /*: number */) {
  var retImg;
  var isDummy = isDummyFont(face);
  if (!isDummy) {
    var glyph = face.getGlyph(codePoint);
    // TODO: Can we reuse the same canvas element?
    var canvas = document.createElement("canvas");
    canvas.width = 24; // TODO: sizes
    canvas.height = 24; // TODO: sizes
    var ctx = canvas.getContext("2d");
    glyph.render(ctx, 24); // TODO: sizes
    retImg = canvas;
  } else {
    var img = new Image();
    img.src =
      "data:image/gif;base64,R0lGODlhAQABAIAAAP///wAAACwAAAAAAQABAAACAkQBADs=";
    retImg = img;
  }
  return createSuccessValue([
    /* <jsoo_empty> */ 0,
    /* width */ isDummy ? face[1] : 24,
    /* height */ isDummy ? face[1] : 24,
    /* bearingX */ 0,
    /* bearingY */ 0,
    /* advance */ 0,
    /* image */ retImg
  ]);
}

// Provides: caml_fk_shape
function caml_fk_shape(face /*: fk_face */, text /*: string */) {
  // TODO: Is there any available function to get the JS string from an OCaml
  // string? Would be better than relying on internal representation
  var str = text.c;
  var isDummy = isDummyFont(face);
  var ret;
  if (isDummy) {
    ret = str.split("").map(function mapper(_char) {
      return [/* <jsoo_empty> */ 0, /* codepoint */ 0, /* cluster */ 0];
    });
  } else {
    var glyphs = face.glyphsForString(str);
    ret = glyphs.map(function mapper(_glyph) {
      // TODO: What about glyphs with multiple codepoints?
      // TODO: What is "cluster"?
      return [
        /* <jsoo_empty> */ 0,
        /* codepoint */ _glyph.codePoints[0],
        /* cluster */ 0
      ];
    });
  }
  // Adding the leading `0` to make it a jsoo friendly array
  ret.unshift(0);
  return ret;
}

// Provides: caml_fk_dummy_font
function caml_fk_dummy_font(size) {
  return [DUMMY_FONT, size];
}
