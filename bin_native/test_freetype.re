open Reglfw;
open Reglfw.Glfw;
open Reglm;
open Refreetype;

open FontKit;
print_endline("Hello, world!");

let loadShader = (shaderType, source) => {
  let shader = glCreateShader(shaderType);
  let () = glShaderSource(shader, source);
  let result = glCompileShader(shader);
  switch (result) {
  | CompilationSuccess => print_endline("Shader compiled successfully.")
  | CompilationFailure(v) => print_endline("Failed to compile shader: " ++ v)
  };
  shader;
};

let initShaderProgram = (vsSource, fsSource) => {
  let vsShader = loadShader(GL_VERTEX_SHADER, vsSource);
  let fsShader = loadShader(GL_FRAGMENT_SHADER, fsSource);
  let shaderProgram = glCreateProgram();
  let () = glAttachShader(shaderProgram, vsShader);
  let _ = glAttachShader(shaderProgram, fsShader);
  let result = glLinkProgram(shaderProgram);
  switch (result) {
  | LinkSuccess => print_endline("Shader linked successfully.");
  | LinkFailure(v) => print_endline("Failed to link shader: " ++ v);
  };
  shaderProgram;
};

let run = () => {
  let _ = glfwInit();
  let w = glfwCreateWindow(800, 600, "freetype example");

  glfwMakeContextCurrent(w);
  glViewport(0, 0, 800, 600);
  
  let%lwt image = Image.load("image4.jpg");
  Image.debug_print(image);

  let font = FontKit.load("E:/Lato-Regular.ttf", 32);

  /* let  image = FontKit.renderGlyph(font, 75); */

  let shapes = FontKit.fk_shape(font, "fi hello world");
    let f = (s: fk_shape) => print_endline("codepoint: " ++ string_of_int(s.codepoint) ++ " | " ++ "cluster: " ++ string_of_int(s.cluster));
  Array.iter(f, shapes);

  let vsSource = {|
        #ifndef GL_ES
        #define lowp
        #endif

        attribute vec2 aTexCoord;

        uniform mat4 uProjection;
        uniform lowp vec4 uPosition;

        varying lowp vec2 vTexCoord;

        void main() {
            /* vec4 pos = vec4(uPosition.x + (aTexCoord.x * uPosition.z), uPosition.y + (aTexCoord.y * uPosition.w), 0.0, 1.0); */
            vec4 pos = vec4(100 * aTexCoord.x, 100 * aTexCoord.y, 0.0, 1.0);
            gl_Position = uProjection * pos;
            /* gl_Position = vec4(1 * aTexCoord.x, 1 * (1 - aTexCoord.y), 0.0, 1.0); */
            vTexCoord = aTexCoord;
        }
    |};

  let fsSource = {|
        #ifndef GL_ES
        #define lowp
        #endif

        varying lowp vec2 vTexCoord;

        uniform sampler2D texture;

        void main() {
            /* gl_FragColor = vec4(vTexCoord.x, vTexCoord.y, 0.0, 1.0); */
            // gl_FragColor = vec4(vTexCoord, 0.0, 1.0);
            vec4 color = texture2D(texture, vTexCoord);
            /* gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); */
            gl_FragColor = vec4(0.0, 0.0, color.a, 1.0);
        }
    |};


      let shaderProgram = initShaderProgram(vsSource, fsSource);

      /* let posAttribute = glGetAttribLocation(shaderProgram, "aVertexPosition"); */
      let texAttribute = glGetAttribLocation(shaderProgram, "aTexCoord");
      let projectionUniform = glGetUniformLocation(shaderProgram, "uProjection");
      let _positionUniform = glGetUniformLocation(shaderProgram, "uPosition");

      let textureCoordinates = [|0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0|];
      let indices = [|0, 1, 2, 0, 2, 3|];

      let tArray = Float32Array.of_array(textureCoordinates);
      let iArray = Uint16Array.of_array(indices);

      let tb = glCreateBuffer();
      let ib = glCreateBuffer();

      glBindBuffer(GL_ARRAY_BUFFER, tb);
      glBufferData(GL_ARRAY_BUFFER, tArray, GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, iArray, GL_STATIC_DRAW);


   let render = (s: FontKit.fk_shape, _x: int, _y: int) => {

      let glyph = FontKit.renderGlyph(font, s.codepoint);
    
      let {image, _} = glyph;
        


      let projection = Mat4.create();
      Mat4.ortho(projection, 0.0, 800.0, 0.0, 600.0, 0.1, 100.0);

        print_endline("0, 0:" ++ string_of_float(Mat4.get(projection, 6)));
    glUniformMatrix4fv(projectionUniform, projection);


        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        let texture = glCreateTexture();
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, GL_UNSIGNED_BYTE, image);

    /* glUniform4f(positionUniform, 1.0, 1.0, 30.0, 30.0); */

    glBindBuffer(GL_ARRAY_BUFFER, tb);
    glVertexAttribPointer(texAttribute, 2, GL_FLOAT, false);
    glEnableVertexAttribArray(texAttribute);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
   };


  glfwRenderLoop((_t) => {
    glClearColor(1.0, 0., 0., 1.);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glUseProgram(shaderProgram);

    let shapes = FontKit.fk_shape(font, "fi hello world");
    Array.iter((s) => render(s, 0, 0), shapes);

    glfwSwapBuffers(w);
    glfwPollEvents();
    glfwWindowShouldClose(w);
  });

  Lwt.return();
};

Lwt_main.run(run());
