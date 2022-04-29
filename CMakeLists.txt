cmake_minimum_required(VERSION 3.21)
project(HotPotato)

set(CMAKE_CXX_STANDARD 14)

add_executable(RingMaster ringmaster.c socketInterface.c socketInterface.h)
add_executable(Player1 player.c socketInterface.c socketInterface.h)
add_executable(Player2 player.c socketInterface.c socketInterface.h)
add_executable(Player3 player.c socketInterface.c socketInterface.h)


