do_run := true;
optimization := enum { DEBUG; DEVELOPMENT; SHIPPING; }.SHIPPING;

source_dir := "src";
output_dir := "out"; 

// indicate to jai compiler that this code should be run right now, rather than packaged into an exe/dll/whatever
#run {
    // tell compiler to not output a warning about having no main in this file
    set_build_options_dc({do_output=false});

    make_directory_if_it_does_not_exist(output_dir);
    output_file_path := tprint("%/index.js", output_dir);

    create_generated_code_header();
    build_success := build(output_file_path);

    if do_run {
        if build_success {
            log("running...");
            output_file_path = tprint("%/index.html", output_dir);
            run(output_file_path);
        } else {
            log("failed to build. not running.");
        }
    }
}

build :: (output_file_path: string) -> success: bool {
    // form the build command as an array of strings
    build_cmd: [..]string;

    // on windows, emscripten is invoked via this batch file
    array_add(*build_cmd, "emcc.bat");
    // enumerate project source files as compile args
    enum_files(source_dir, *build_cmd, .["cpp"]);
    array_add(*build_cmd, "-std=c++17");
    // allow output program heap size to grow
    array_add(*build_cmd, "-sALLOW_MEMORY_GROWTH");

    if optimization == {
    case .DEBUG;
        // no optimization
        array_add(*build_cmd, "-O0");
        // slower, but polices heap allocation
        array_add(*build_cmd, "-sSAFE_HEAP");
        // how much debug info to generate
        array_add(*build_cmd, "-g3");
        // don't inline functions, better for debugging
        array_add(*build_cmd, "-fno-inline");
    case .DEVELOPMENT;
        // minimal optimization
        array_add(*build_cmd, "-O1");
        // generate less debug info than in DEBUG
        array_add(*build_cmd, "-g1");
    case .SHIPPING;
        // max optimization
        array_add(*build_cmd, "-O3");
    }

    // shadowing names in source is a warning
    array_add(*build_cmd, "-Wshadow");
    // for non-emscripten-default includes
    array_add(*build_cmd, "-Ithird_party");
    // using SDL3 for video, input, (sound?)
    array_add(*build_cmd, "-s");
    array_add(*build_cmd, "USE_SDL=3");

    // array_add(*build_cmd, "-sSTACK_SIZE=128000");
    // array_add(*build_cmd, "-sMEMORY64=1");
    // don't need multiple threads for this game.
    // array_add(*build_cmd, "-s");
    // array_add(*build_cmd, "USE_PTHREADS=1");

    // whitelist some inputs for the browser (e.g. allow press f12 to open chrome dev console); not consumed by program
    array_add(*build_cmd, "--pre-js");
    array_add(*build_cmd, "src/whitelist.js");

    // where does output go
    array_add(*build_cmd, "-o");
    array_add(*build_cmd, output_file_path);

    // package assets
    array_add(*build_cmd, "--use-preload-plugins");
    asset_files: [..]string;
    enum_files("assets", *asset_files, .["png", "obj"]);
    for asset_files {
        array_add(*build_cmd, tprint("--preload-file=%@%", it, it));
    }

    log("running build command");
    log("=====================");
    for build_cmd {
        log(" %", it);
    }
    log("=====================");

    result := run_command(..build_cmd);
    return result.exit_code == 0;
}

run :: (output_file_path: string) {
    run_cmd: [..]string;
    // default python server to host the game provided by emscripten, for development. also creates an html page and opens it.
    array_add(*run_cmd, "emrun.bat");
    array_add(*run_cmd, output_file_path);
    run_command(..run_cmd);
}

// recursively enumerate files with any from a list of extensions
enum_files :: (path: string, files: *[..]string, extensions: []string) {
    Visit_Data :: struct {
        files: *[..]string;
        extensions: []string;
    }
    visitor :: (info: *File_Visit_Info, visit_data: Visit_Data) {
        extension := path_extension(info.full_name);
        for visit_data.extensions {
            if extension == it {
                array_add(visit_data.files, copy_string(info.full_name));
            }
        }
    }
    visit_files(path, recursive=true, Visit_Data.{files, extensions}, visitor);
}

create_generated_code_header :: () {
    GENERATED_FILENAME :: "generated.h";

    header_files : [..]string;
    enum_files(source_dir, *header_files, .["h"]);

    file_builder : String_Builder;
    print(*file_builder, 
#string HERE
#pragma once

HERE);

            print(*file_builder,
#string HERE
template <typename EnumType>
inline s32 EnumValueCount() { return 0; }
HERE
            );

    for file : header_files {
        file_contents := read_entire_file(file);
        defer free(file_contents);

        contents := file_contents;
        sanity_counter := 0;
        while sanity_counter < 10_000 {
            defer sanity_counter += 1;

            success, left, remain := split_from_left(contents, "enum class");
            if !success {
                break;
            }
            defer contents = remain;

            success, class_name:, remain = split_from_left(remain, "{");
            if !success {
                continue;
            }

            class_name_parts := split(class_name, #char ":");
            class_name = trim(class_name_parts[0]);
            print(*file_builder,
#string HERE
inline const char* ToString(% EnumVal)
{
    switch (EnumVal)
    {
HERE,
                class_name
            );

            success, enum_contents:, remain = split_from_left(remain, "}");
            if !success {
                continue;
            }

            lines := split(enum_contents, #char "\n");
            entry_count := 0;
            for line : lines {
                if contains(line, "//") {
                    continue;
                }
                entry_parts_1 := split(line, #char ",");
                entry_parts_2 := split(entry_parts_1[0], "=");
                entry := trim(entry_parts_2[0]); 
                if entry.count == 0 {
                    continue;
                }
                entry_count += 1;
                print(*file_builder,
#string HERE
    case %::%:
        return "%";
HERE,
                    class_name, entry, entry
                );
            }

            print(*file_builder,
#string HERE
    }
}

HERE
            );

            print(*file_builder,
#string HERE
template <>
inline s32 EnumValueCount<%>()
{
    return %;
}

HERE,
                class_name, entry_count
            );
        }
    }

    generated_contents := builder_to_string(*file_builder);
    generated_filepath := tprint("%/%", source_dir, GENERATED_FILENAME);
    write_entire_file(generated_filepath, generated_contents);
}

#import "Basic";
#import "File";
#import "File_Utilities";
#import "System";
#import "Process";
#import "String";
#import "Compiler";