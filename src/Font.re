exception FreeTypeInitializeException(string);
exception FreeTypeLoadFaceException(string);
exception FreeTypeRenderCharacterException(string);

open Freetype;
open Reglfw;

type face = FT_Face;

type character = {
    image: Image.t,
    bearingX: int,
    bearingY: int,
    advance: int
};

let freeTypeInstance = ref(option(Freetype));

let initOrGet = () => {
    switch (freeTypeInstance) {
    | Some(ft) => ft
    | None => {
        let ft = FT_Init_FreeType();
        switch (ft) {
        | Success(f) => {
            freeTypeInstance := Some(f);
            f;
        } 
        | Error(msg) => raise(FreeTypeInitializeException(msg));
        }
    }
    } 
}

let open = (fontFile: string, size: int) => {
    let ft = initOrGet();

    let face = FT_New_Face(ft, fontFile, size);
    switch (ft) {
    | Success(f) => f
    | Error(msg) => raise(FreeTypeLoadFaceException(msg));
    }
}

let getCharacter = (face, character: char) => {
    let loadingChar = FT_Load_Char(face, character);
    let char = switch (loadingChar) {
    | Success(c) => c
    | Error(msg) => raise(FreeTypeRenderCharacterException(msg))
    };

    let image = FT_Char_Get_Image(char);
    let (bearingX, bearingY, advance) = FT_Char_Get_Metrics(char);

    let ret: Character = {
        image,
        bearingX,
        bearingY,
        advance
    };
    ret;
};

type TextureCoordinates = {
    startX: float,
    startY: float,
    endX: float,
    endY: float
}

type Glyph = {
    texture: Glfw.texture,
    textureCoordinates: TextureCoordinates,
    width: int,
    height: int,
    bearingX: int,
    bearingY: int,
    advance: int
}

let getGlyph = (face: Face, character: char) => {
    /* TODO: Cache value! */
    /* TODO: Move to a character atlas! */

    let character = getCharacter(face, character);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    let texture = glCreateTexture();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, GL_RGB, GL_UNSIGNED_BYTE, character.image);

    let texCoords: TextureCoordinates = {
        startX: 0.0,
        startY: 0.0,
        endX: 0.0,
        endY: 0.0
    };
};

let layout = (face: Face, line: string) => {

};

let draw = (face: Face, line: string, x: number, y: number) => {

};
