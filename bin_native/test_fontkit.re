open Reglfw;
open Reglfw.Glfw;
open Reglm;
open Fontkit;

let isNative =
  switch (Sys.backend_type) {
  | Native => true
  | Bytecode => true
  | _ => false
  };

Printexc.record_backtrace(true);

let getExecutingDirectory = () =>
  isNative ? Filename.dirname(Sys.argv[0]) ++ Filename.dir_sep : "";

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
  | LinkSuccess => print_endline("Shader linked successfully.")
  | LinkFailure(v) => print_endline("Failed to link shader: " ++ v)
  };
  shaderProgram;
};

let run = () => {
  let _ = glfwInit();
  let w = glfwCreateWindow(800, 600, "freetype example");

  glfwMakeContextCurrent(w);
  glViewport(0, 0, 800, 600);

  let%lwt font =
    Fontkit.load(getExecutingDirectory() ++ "Roboto-Regular.ttf", 24);

  let metrics = Fontkit.fk_get_metrics(font);
  print_endline ("-- height: " ++ string_of_int(metrics.height));
  print_endline ("-- ascent: " ++ string_of_int(metrics.ascent));
  print_endline ("-- descent: " ++ string_of_int(metrics.descent));
  print_endline ("-- underlinePosition: " ++ string_of_int(metrics.underlinePosition));
  print_endline ("-- underlineThickness: " ++ string_of_int(metrics.underlineThickness));
  print_endline ("-- unitsPerEm: " ++ string_of_int(metrics.unitsPerEm));

  let metrics = Fontkit.fk_get_metrics(font);
  print_endline ("-- height: " ++ string_of_int(metrics.height));
  print_endline ("-- ascent: " ++ string_of_int(metrics.ascent));
  print_endline ("-- descent: " ++ string_of_int(metrics.descent));
  print_endline ("-- underlinePosition: " ++ string_of_int(metrics.underlinePosition));
  print_endline ("-- underlineThickness: " ++ string_of_int(metrics.underlineThickness));
  print_endline ("-- unitsPerEm: " ++ string_of_int(metrics.unitsPerEm));

  let vsSource = {|
        #ifndef GL_ES
        #define lowp
        #endif

        attribute vec2 aTexCoord;

        uniform mat4 uProjection;
        uniform lowp vec4 uPosition;

        varying lowp vec2 vTexCoord;

        void main() {
            vec4 pos = vec4(uPosition.x + (aTexCoord.x * uPosition.z), uPosition.y + (aTexCoord.y * uPosition.w), 0.1, 1.0);
            gl_Position = uProjection * pos;
            /* gl_Position = vec4(1 * aTexCoord.x, 1 * (aTexCoord.y), 0.0, 1.0); */
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
            vec4 color = texture2D(texture, vec2(vTexCoord.x, 1.0 - vTexCoord.y));
            /* gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); */
            gl_FragColor = vec4(0.0, 0.0, color.a, 1.0);
        }
    |};

  let shaderProgram = initShaderProgram(vsSource, fsSource);

  let texAttribute = glGetAttribLocation(shaderProgram, "aTexCoord");
  let projectionUniform = glGetUniformLocation(shaderProgram, "uProjection");
  let positionUniform = glGetUniformLocation(shaderProgram, "uPosition");

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

  let projection = Mat4.create();
  Mat4.ortho(projection, 0.0, 800.0, 0.0, 600.0, -0.01, -100.0);

  let render = (s: Fontkit.fk_shape, x: float, y: float) => {
    let glyph = Fontkit.renderGlyph(font, s.glyphId);

    let {bitmap, width, height, bearingX, bearingY, advance, _} = glyph;

    glUniformMatrix4fv(projectionUniform, projection);
    glUniform4f(
      positionUniform,
      x +. float_of_int(bearingX),
      y -. (float_of_int(height) -. float_of_int(bearingY)),
      float_of_int(width),
      float_of_int(height),
    );

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    let texture = glCreateTexture();
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_ALPHA,
      GL_ALPHA,
      GL_UNSIGNED_BYTE,
      bitmap,
    );

    glBindBuffer(GL_ARRAY_BUFFER, tb);
    glVertexAttribPointer(texAttribute, 2, GL_FLOAT, false);
    glEnableVertexAttribArray(texAttribute);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    x +. float_of_int(advance) /. 64.0;
  };

  let renderString = (str: string, x: float, y: float) => {
    let shapes = Fontkit.fk_shape(font, str);
    let startX = ref(x);
    Array.iter(
      s => {
        let nextPosition = render(s, startX^, y);
        startX := nextPosition;
      },
      shapes,
    );
  };

  glfwRenderLoop(_t => {
    glClearColor(0.0, 0., 0., 1.);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glUseProgram(shaderProgram);

    renderString(string_of_float(Unix.gettimeofday()), 0.0, 500.0);
    renderString("pfi hello=>world !== fiq", 100., 100.);

    Gc.full_major();

    glfwSwapBuffers(w);
    glfwPollEvents();
    glfwWindowShouldClose(w);
  });

  Lwt.return();
};

Lwt_main.run(run());