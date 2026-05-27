include_guard(GLOBAL)

include(FetchContent)

function(add_reson)
    set(options)
    set(one_value_args
        SOURCE_DIR
        GIT_REPOSITORY
        GIT_TAG
    )
    set(multi_value_args)

    cmake_parse_arguments(
        ADD_RESON
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    if (TARGET reson::reson)
        return()
    endif()

    if (ADD_RESON_SOURCE_DIR)
        FetchContent_Declare(
            reson
            SOURCE_DIR "${ADD_RESON_SOURCE_DIR}"
        )
    else()
        if (NOT ADD_RESON_GIT_REPOSITORY)
            message(FATAL_ERROR
                "add_reson requires either SOURCE_DIR or GIT_REPOSITORY"
            )
        endif()

        if (NOT ADD_RESON_GIT_TAG)
            set(ADD_RESON_GIT_TAG main)
        endif()

        FetchContent_Declare(
            reson
            GIT_REPOSITORY "${ADD_RESON_GIT_REPOSITORY}"
            GIT_TAG "${ADD_RESON_GIT_TAG}"
        )
    endif()

    set(RESON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(RESON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(RESON_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(reson)

    if (NOT TARGET reson::reson)
        message(FATAL_ERROR
            "reson was added, but target reson::reson was not created"
        )
    endif()
endfunction()