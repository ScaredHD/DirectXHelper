
function(CurrentDemoRelativeDir outDir)
    set(absDir ${CMAKE_CURRENT_LIST_DIR})
    set(rootDir ${CMAKE_SOURCE_DIR})
    string(REGEX REPLACE "^${rootDir}[/\\]" "" suffix ${absDir})
    set(${outDir} ${suffix} PARENT_SCOPE)
endfunction()

function(CurrentDemoRuntimeDir outDir)
    set(binDir ${CMAKE_BINARY_DIR})
    CurrentDemoRelativeDir(suffix)
    set(${outDir} "${binDir}/${suffix}" PARENT_SCOPE)
endfunction()

function(CopyMatchedFilesAfterBuild targetName srcPath pattern destPath recursive shouldEcho)
    set(filePattern ${srcPath}/${pattern})

    if (recursive)
        file(GLOB_RECURSE matchedFiles CMAKE_CONFIGURE_DEPENDS ${filePattern})
    else()
        file(GLOB matchedFiles CMAKE_CONFIGURE_DEPENDS ${filePattern})
    endif()

    if (matchedFiles)
        add_custom_command(
            TARGET ${targetName}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${matchedFiles}
            ${destPath}
        )

        get_filename_component(absSrcPath ${srcPath} ABSOLUTE)
        set(absFilePattern ${absSrcPath}/${pattern})

        get_filename_component(absDestPath ${destPath} ABSOLUTE)

        if(shouldEcho)
            message(STATUS "[${targetName}] Post-build copy: '${absFilePattern}' -> '${absDestPath}'")
        endif()
    endif()
endfunction()

function(CopyShadersToOutputDir targetName shaderPath shouldEcho)
    get_target_property(outputDir ${targetName} RUNTIME_OUTPUT_DIRECTORY)
    CopyMatchedFilesAfterBuild(${targetName} ${shaderPath} "*.hlsl" ${outputDir} FALSE ${shouldEcho})
endfunction()

function(SetupDemoOutput targetName copyShaders)
    CurrentDemoRuntimeDir(outputDir)
    set_target_properties(
        ${targetName} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${outputDir}
    )
    if (${copyShaders})
        CopyShadersToOutputDir(${targetName} . TRUE)
    endif()
endfunction()