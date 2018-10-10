open Reglfw;

type ft_freetype;

type ft_return('a) =
| Success('a)
| Error(string)

type ft_face;

type ft_character;

type ft_metrics = (int, int, int);

external ft_init: unit => ft_return(ft_freetype) = "caml_ft_init_freetype";

external ft_new_face: (ft_freetype, string, int) => ft_return(ft_face) = "caml_ft_new_face";
external ft_load_char: (ft_face, char) => ft_return(ft_character) = "caml_ft_load_char";

external ft_char_get_image: (ft_character) => Image.t = "caml_ft_char_get_image";
external ft_char_get_metrics: (ft_character) => ft_metrics = "caml_ft_char_get_bearing";

external ft_done_face: (ft_face) => unit = "caml_ft_done_face";
external ft_done_freetype: (ft_freetype) => unit = "caml_ft_done_freetype"

