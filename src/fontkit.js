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
function caml_fk_load_glyph(face /*: fk_face */, glyphId /*: number */) {
  var isDummy = isDummyFont(face);
  // `texImage2D` allows for the texture pixels to be passed as image, canvas or other formats:
  // https://developer.mozilla.org/en-US/docs/Web/API/WebGLRenderingContext/texImage2D
  if (!isDummy) {
    var glyph = face.getGlyph(glyphId);
    // TODO: Can we reuse the same canvas element?
    var canvas = document.createElement("canvas");
    canvas.width = 24; // TODO: sizes
    canvas.height = 24; // TODO: sizes
    var ctx = canvas.getContext("2d");
    ctx.translate(0, 24);
    ctx.scale(1, -1);
    ctx.save();
    ctx.beginPath();
    glyph.render(ctx, 24); // TODO: sizes
    ctx.restore();
    return createSuccessValue([
      /* <jsoo_empty> */ 0,
      /* width */ 24, // TODO: sizes
      /* height */ 24, // TODO: sizes
      /* bearingX */ 0,
      /* bearingY */ 0,
      /* advance */ glyph.advanceWidth,
      /* image */ canvas
    ]);
  } else {
    var img = new Image();
    img.src =
      "data:image/gif;base64,R0lGODlhAQABAIAAAP///wAAACwAAAAAAQABAAACAkQBADs=";
    return createSuccessValue([
      /* <jsoo_empty> */ 0,
      /* width */ face[1],
      /* height */ face[1],
      /* bearingX */ 0,
      /* bearingY */ 0,
      /* advance */ 10,
      /* image */ img
    ]);
  }
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
      return [/* <jsoo_empty> */ 0, /* glyphId */ 0, /* cluster */ 0];
    });
  } else {
    var glyphs = face.glyphsForString(str);
    ret = glyphs.map(function mapper(_glyph) {
      return [
        /* <jsoo_empty> */ 0,
        /* glyphId */ _glyph.id,
        /* cluster */ 0 // TODO: What is "cluster"?
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
