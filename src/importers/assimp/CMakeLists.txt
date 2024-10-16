file(GLOB_RECURSE SOURCE_FILES *.c*)

vision_add_plugin(assimp CATEGORY importer DEPENDENCE "vision-assimp_util" SOURCES ${SOURCE_FILES})

