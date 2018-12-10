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
      successCallback(joo_global_object.Fontkit.create(buffer));
    })
    .catch(function onError(error) {
      failureCallback(error.message);
    });
  return undefined;
}

// Provides: caml_fk_load_glyph
// Requires: caml_new_string
function caml_fk_load_glyph(face /*: fk_face */, glyphId /*: number */) {
  console.log(Fontkit);
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
