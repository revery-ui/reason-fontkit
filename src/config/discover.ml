type os =
    | Windows
    | Mac
    | Linux
    | Unknown

let uname () =
    let ic = Unix.open_process_in "uname" in
    let uname = input_line ic in
    let () = close_in ic in
    uname;;

let get_os =
    match Sys.os_type with
    | "Win32" -> Windows
    | _ -> match uname () with
        | "Darwin" -> Mac
        | "Linux" -> Linux
        | _ -> Unknown

let c_flags = ["-I";  (Sys.getenv "FREETYPE2_INCLUDE_PATH"); "-I"; (Sys.getenv "HARFBUZZ_INCLUDE_PATH")]

let ccopt s = ["-ccopt"; s]
let cclib s = ["-cclib"; s]

let extraFlags = 
    match get_os with
    | Windows -> []
    | _ -> []
    @ ccopt ("-L/usr/lib")
    @ ccopt ("-L/usr/local/lib")
    @ cclib ("-lbz2")
    @ cclib ("-lpng")
    @ cclib ("-lz")

let flags = []
    @ ccopt "-L."
    @ ccopt ("-L" ^ (Sys.getenv "FREETYPE2_LIB_PATH"))
    @ ccopt ("-L" ^ (Sys.getenv "HARFBUZZ_LIB_PATH"))
    @ cclib ("-lfreetype")
    @ cclib ("-lharfbuzz")
    @ extraFlags
;;

let cxx_flags =
    match get_os with
    | Windows -> c_flags @ ["-fno-exceptions"; "-fno-rtti"; "-lstdc++"]
    | _ -> c_flags
;;

Configurator.V1.Flags.write_sexp "c_flags.sexp" c_flags;
Configurator.V1.Flags.write_sexp "cxx_flags.sexp" cxx_flags;
Configurator.V1.Flags.write_sexp "flags.sexp" flags;
