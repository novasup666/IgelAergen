project(IGELARGEN)
set(CMAKE_C_FLAGS, "${CMAKE_C_FLAGS} -Wall")
add_executable(prog src/main.c src/resizableCharArray.c src/affichage.c src/util.c)