include(FetchContent)

set(DEP_NAMES )

# Fetch nlohmann_json
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz
)
list(APPEND DEP_NAMES nlohmann_json)

FetchContent_MakeAvailable(${DEP_NAMES})