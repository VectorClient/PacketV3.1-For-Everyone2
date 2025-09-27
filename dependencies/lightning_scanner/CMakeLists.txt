message(STATUS "Fetching LightningScanner...")

FetchContent_Declare(
        LightningScanner
        GIT_REPOSITORY "https://github.com/localcc/LightningScanner.git"
        GIT_TAG 782f1a45567ad5d216ff6a71a1b97b891d026830
)
FetchContent_MakeAvailable(LightningScanner)