
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

function(CopyMatchedFilesAfterBuild targetName srcPath pattern destPath recursive)
    set(filePattern ${srcPath}/${pattern})

    if (recursive)
        file(GLOB_RECURSE matchedFiles CMAKE_CONFIGURE_DEPENDS ${filePattern})
    else()
        file(GLOB matchedFiles CMAKE_CONFIGURE_DEPENDS ${filePattern})
    endif()

    foreach(matchedFile ${matchedFiles})
        get_filename_component(matchedFileName ${matchedFile} NAME)
        set(destFile "${destPath}/${matchedFileName}")
        message(STATUS "Setup copy: ${matchedFile} -> ${destFile}")
        add_custom_command(
            OUTPUT "${destFile}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${matchedFile}" "${destFile}"
            DEPENDS ${matchedFile}
            COMMENT "Copying: ${matchedFile} -> ${destFile}"
        )
        target_sources(${targetName} PRIVATE "${destFile}")
    endforeach()
endfunction()

function(CopyShadersToOutputDir targetName shaderPath)
    get_target_property(outputDir ${targetName} RUNTIME_OUTPUT_DIRECTORY)
    CopyMatchedFilesAfterBuild(${targetName} ${shaderPath} "*.hlsl" ${outputDir} FALSE)
endfunction()

function(SetupDemoOutput targetName copyShaders)
    CurrentDemoRuntimeDir(outputDir)
    set_target_properties(
        ${targetName} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${outputDir}
    )
    if (${copyShaders})
        file(GLOB_RECURSE shaderFiles CONFIGURE_DEPENDS "*.hlsl")
        CopyShadersToOutputDir(${targetName} ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endfunction()