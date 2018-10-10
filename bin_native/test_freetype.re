open Reglfw;
open Reglfw.Glfw;

open Refreetype;

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

  let font = Font.load("Roboto.ttf", 32);
  let character = Font.getGlyph(font, 'A');

  let vsSource = {|
        #ifndef GL_ES
        #define lowp
        #endif

        attribute vec2 aVertexPosition;
        attribute vec2 aTexCoord;

        varying lowp vec2 vTexCoord;

        void main() {
            gl_Position = vec4(aVertexPosition.x, aVertexPosition.y, 0.0, 1.0);
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
            gl_FragColor = vec4(0.0, 0.0, color.a, 1.0);
        }
    |};

  let positions = [|(-0.5), 0.5, 0.5, 0.5, 0.5, (-0.5), (-0.5), (-0.5)|];
  let textureCoordinates = [|0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0|];
  let indices = [|0, 1, 2, 0, 2, 3|];

  let vArray = Float32Array.of_array(positions);
  let tArray = Float32Array.of_array(textureCoordinates);
  let iArray = Uint16Array.of_array(indices);

  let shaderProgram = initShaderProgram(vsSource, fsSource);
  let vb = glCreateBuffer();
  let tb = glCreateBuffer();
  let ib = glCreateBuffer();

  glBindBuffer(GL_ARRAY_BUFFER, vb);
  glBufferData(GL_ARRAY_BUFFER, vArray, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, tb);
  glBufferData(GL_ARRAY_BUFFER, tArray, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, iArray, GL_STATIC_DRAW);

  let posAttribute = glGetAttribLocation(shaderProgram, "aVertexPosition");
  let texAttribute = glGetAttribLocation(shaderProgram, "aTexCoord");

  glfwRenderLoop((_t) => {
    glClearColor(1.0, 0., 0., 1.);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, character.texture);

    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glVertexAttribPointer(posAttribute, 2, GL_FLOAT, false);
    glEnableVertexAttribArray(posAttribute);

    glBindBuffer(GL_ARRAY_BUFFER, tb);
    glVertexAttribPointer(texAttribute, 2, GL_FLOAT, false);
    glEnableVertexAttribArray(texAttribute);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glfwSwapBuffers(w);
    glfwPollEvents();
    glfwWindowShouldClose(w);
  });

  Lwt.return();
};

Lwt_main.run(run());
