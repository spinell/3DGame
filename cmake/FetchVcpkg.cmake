#
# Fetch vcpkg from github
#

include(FetchContent)
FetchContent_Declare(vcpkg
    GIT_REPOSITORY https://github.com/microsoft/vcpkg/
    GIT_TAG 2024.12.16 # b322364

)

FetchContent_MakeAvailable(vcpkg)
