// Provides: DUMMY_FONT
var DUMMY_FONT = "__FONTKIT__DUMMY__FONT__";

// Provides: DUMMY_BITMAP_DIMENSION
var DUMMY_BITMAP_DIMENSION = 12;

// Provides: DUMMY_PIXELS
// Requires: DUMMY_BITMAP_DIMENSION
var DUMMY_PIXELS = new joo_global_object.Uint8Array(
  DUMMY_BITMAP_DIMENSION * DUMMY_BITMAP_DIMENSION
);

// Provides: isDummyFont
// Requires: DUMMY_FONT
function isDummyFont(face) {
  return face.length === 2 && face[0] === DUMMY_FONT;
}

// Provides: createSuccessValue
function createSuccessValue(value) {
  // Important: The value `0` of the Success tag correspond to the ordering in
  // which fk_return variants are declared in the type declaration
  return [0, value];
}

// Provides: caml_fk_new_face
// Requires: createSuccessValue
function caml_fk_new_face(
  fontName /*: string */,
  size /*: number */,
  successCallback /*: face => void */,
  failureCallback /*: string => void */
) {
  joo_global_object
    .fetch(fontName.c)
    .then(function toBlob(res) {
      return res.blob();
    })
    .then(function toBuffer(blob) {
      // From: https://github.com/feross/blob-to-buffer/blob/fe48e780ac95ebea27387cc8f6aa6db1ec735ad0/index.js
      return new joo_global_object.Promise(function(resolve, reject) {
        var reader = new joo_global_object.FileReader();
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
      // HACK: attaching `size` to `fontFace` here to be able to get it later in `caml_fk_load_glyph`
      fontFace.size = size;
      successCallback(fontFace);
    })
    .catch(function onError(error) {
      failureCallback(error.message);
    });
  return undefined;
}

// Provides: caml_fk_get_metrics
// Requires: isDummyFont
function caml_fk_get_metrics(
  face /*fk_face */
) {

    var isDummy = isDummyFont(face);
    if (isDummy) {
      return [0, face[1], 0, 0, 0, 0, 1, face[1]];
    } else {
      var height = Math.abs(face.ascent) + Math.abs(face.descent) + face.lineGap;

      return [0, height, face.ascent, face.descent, face.underlinePosition, face.underlineThickness, face.unitsPerEm, face.size];
    }
}

// Provides: caml_fk_load_glyph
// Requires: isDummyFont, createSuccessValue, caml_ba_create_from, DUMMY_BITMAP_DIMENSION, DUMMY_PIXELS
function caml_fk_load_glyph(face /*: fk_face */, glyphId /*: number */) {
  var isDummy = isDummyFont(face);
  // `texImage2D` allows for the texture pixels to be passed as image, canvas or other elements:
  // https://developer.mozilla.org/en-US/docs/Web/API/WebGLRenderingContext/texImage2D
  if (isDummy) {
    var dummyBitmap = caml_ba_create_from(
      DUMMY_PIXELS,
      null,
      0, // general type
      3, // kind: uint8
      0, // c layout
      [DUMMY_BITMAP_DIMENSION, DUMMY_BITMAP_DIMENSION]
    );
    return createSuccessValue([
      /* <jsoo_empty> */ 0,
      /* width */ DUMMY_BITMAP_DIMENSION,
      /* height */ DUMMY_BITMAP_DIMENSION,
      /* bearingX */ 0,
      /* bearingY */ 0,
      /* advance */ 0,
      /* bitmap */ dummyBitmap
    ]);
  } else {
    var glyph = face.getGlyph(glyphId);
    // TODO: Can we reuse the same canvas element?
    var canvas = document.createElement("canvas");
    var scale = (1 / face.unitsPerEm) * face.size;
    var advanceWidth = glyph.advanceWidth * scale;
    var bearingX = glyph._metrics.leftBearing * scale;
    var bearingY = glyph._metrics.topBearing * scale;
    var glyphWidth = Math.max(
      Math.ceil((glyph.bbox.maxX - glyph.bbox.minX) * scale),
      0
    );
    var glyphHeight = Math.max(
      Math.ceil((glyph.bbox.maxY - glyph.bbox.minY) * scale),
      0
    );
    canvas.width = glyphWidth;
    canvas.height = glyphHeight;
    var ctx = canvas.getContext("2d");
    ctx.translate(-bearingX, glyph.bbox.maxY * scale);
    ctx.scale(1, -1);
    glyph.render(ctx, face.size);
    var numChannels = 4; // RGBA
    var imageData =
      glyphHeight > 0 && glyphWidth > 0
        ? ctx.getImageData(0, 0, canvas.width, canvas.height)
        : null;
    var pixels = new joo_global_object.Uint8Array(imageData && imageData.data);
    var bitmap = caml_ba_create_from(
      pixels,
      null,
      0, // general type
      3, // kind: uint8
      0, // c layout
      [glyphHeight, glyphWidth * numChannels]
    );
    return createSuccessValue([
      /* <jsoo_empty> */ 0,
      /* width */ glyphWidth,
      /* height */ glyphHeight,
      /* bearingX */ bearingX,
      /* bearingY */ face.size - bearingY,
      /* advance */ advanceWidth * 64,
      /* bitmap */ bitmap
    ]);
  }
}

// Provides: caml_fk_shape
// Requires: isDummyFont, caml_to_js_string
function caml_fk_shape(face /*: fk_face */, text /*: string */) {
  var str = caml_to_js_string(text);
  var isDummy = isDummyFont(face);
  var ret;
  if (isDummy) {
    ret = str.split("").map(function mapper(_char) {
      return [/* <jsoo_empty> */ 0, /* glyphId */ 0, /* cluster */ 0];
    });
  } else {
    var run = face.layout(str);
    ret = run.glyphs.map(function mapper(_glyph, index) {
      return [
        /* <jsoo_empty> */ 0,
        /* glyphId */ _glyph.id,
        /* cluster */ 0 // What is "cluster"?
      ];
    });
  }
  // Adding the leading `0` to make it a jsoo compatible array
  ret.unshift(0);
  return ret;
}

// Provides: caml_fk_dummy_font
// Requires: DUMMY_FONT
function caml_fk_dummy_font(size) {
  return [DUMMY_FONT, size];
}
