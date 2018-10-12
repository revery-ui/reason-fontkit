/* exception FreeTypeInitializeException(string); */
/* exception FreeTypeLoadFaceException(string); */
/* exception FreeTypeRenderCharacterException(string); */

/* open FreeType_Bindings; */
/* open Reglfw; */
/* open Reglfw.Glfw; */

/* type face = ft_face; */

/* type character = { */
/*     image: Image.t, */
/*     bearingX: int, */
/*     bearingY: int, */
/*     advance: int */
/* }; */

/* let freeTypeInstance: ref(option(ft_freetype)) = ref(None); */

/* let initOrGet = () => { */
/*     switch (freeTypeInstance^) { */
/*     | Some(ft) => ft */
/*     | None => { */
/*         let ft = ft_init(); */
/*         switch (ft) { */
/*         | Success(f) => { */
/*             freeTypeInstance := Some(f); */
/*             f; */
/*         } */ 
/*         | Error(msg) => raise(FreeTypeInitializeException(msg)); */
/*         } */
/*     } */
/*     } */ 
/* } */

/* let load = (fontFile: string, size: int) => { */
/*     let ft = initOrGet(); */

/*     let face = ft_new_face(ft, fontFile, size); */
/*     switch (face) { */
/*     | Success(f) => f */
/*     | Error(msg) => raise(FreeTypeLoadFaceException(msg)); */
/*     } */
/* } */

/* let getCharacter = (face, character: char) => { */
/*     let loadingChar = ft_load_char(face, character); */
/*     let char = switch (loadingChar) { */
/*     | Success(c) => c */
/*     | Error(msg) => raise(FreeTypeRenderCharacterException(msg)) */
/*     }; */

/* print_endline("loading character"); */
/*     let image = ft_char_get_image(char); */
/*     /1* let (bearingX, bearingY, advance) = ft_char_get_metrics(char); *1/ */

/*     let ret: character = { */
/*         image, */
/*         bearingX: 0, */
/*         bearingY: 0, */
/*         advance: 0, */
/*     }; */
/*     ret; */
/* }; */

/* type textureCoordinates = { */
/*     startX: float, */
/*     startY: float, */
/*     endX: float, */
/*     endY: float */
/* } */

/* type glyph = { */
/*     texture: Glfw.texture, */
/*     textureCoordinates: textureCoordinates, */
/*     width: int, */
/*     height: int, */
/*     bearingX: int, */
/*     bearingY: int, */
/*     advance: int */
/* } */

/* let getGlyph = (face: face, character: char) => { */
/*     /1* TODO: Cache value! *1/ */
/*     /1* TODO: Move to a character atlas! *1/ */

/*     let character = getCharacter(face, character); */

/*     glPixelStorei(GL_PACK_ALIGNMENT, 1); */
/*     glPixelStorei(GL_UNPACK_ALIGNMENT, 1); */

/*     let texture = glCreateTexture(); */
/*     glBindTexture(GL_TEXTURE_2D, texture); */
/*     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); */
/*     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); */
/*     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); */
/*     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); */
/*     glTexImage2D(GL_TEXTURE_2D, GL_UNSIGNED_BYTE, character.image); */

/*     let texCoords: textureCoordinates = { */
/*         startX: 0.0, */
/*         startY: 0.0, */
/*         endX: 0.0, */
/*         endY: 0.0 */
/*     }; */

/*     let ret: glyph = { */
/*         texture, */
/*         textureCoordinates: texCoords, */
/*         bearingX: character.bearingX, */
/*         bearingY: character.bearingY, */ 
/*         advance: character.advance, */
/*         width: 0, */
/*         height: 0, */
/*     } */

/*     ret; */
/* }; */

/*
let layout = (face: face, line: string) => {

};

let draw = (face: face, line: string, x: int, y: int) => {

};
*/
