# Merge pc/mods into the build output mods folder:
# - copy/overwrite files that exist in the source tree
# - leave extra files in the destination untouched (e.g. local test.lua)
if(NOT DEFINED src OR NOT DEFINED dst)
    message(FATAL_ERROR "copy_mods.cmake requires -Dsrc= and -Ddst=")
endif()

file(MAKE_DIRECTORY "${dst}")

file(GLOB_RECURSE mod_files LIST_DIRECTORIES false RELATIVE "${src}" "${src}/*")
foreach(rel ${mod_files})
    get_filename_component(rel_dir "${rel}" DIRECTORY)
    if(rel_dir)
        file(MAKE_DIRECTORY "${dst}/${rel_dir}")
    endif()
    file(COPY_FILE "${src}/${rel}" "${dst}/${rel}" ONLY_IF_DIFFERENT)
endforeach()
