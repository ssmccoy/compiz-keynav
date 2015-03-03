find_package (Compiz REQUIRED)

SET(CMAKE_C_COMPILER "clang")

include (CompizPlugin)

compiz_plugin (keynav PLUGINDEPS)

compiz_add_git_dist()

cmake_minimum_required(VERSION 2.8)
