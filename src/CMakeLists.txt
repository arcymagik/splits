set(CMAKE_CXX_COMPILER g++)

cmake_minimum_required (VERSION 2.8)
project(krechy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_FLAGS_OPTIMIZEDWITHASSERT "${CMAKE_CXX_FLAGS} -O3")

project(root)
include_directories(${root_SOURCE_DIR}/include)

add_library(splits rules/splits.cpp)
add_library(game_algorithm algorithms/game_algorithm)

add_library(random_game_algorithm algorithms/random_game_algorithm.cpp)
add_library(minimax algorithms/minimax.cpp)
add_library(alpha_beta algorithms/alpha_beta.cpp)
add_library(monte_carlo algorithms/monte_carlo.cpp)
add_library(mcts algorithms/mcts)

add_library(simple_grader algorithms/simple_grader.cpp)
add_library(advanced_grader algorithms/adv_grader.cpp)
add_library(zobrist_hasher algorithms/zobrist_hasher.cpp)
add_library(transposition_table algorithms/transposition_table.cpp)
add_library(mc_utils algorithms/mc_utils.cpp)

add_subdirectory(rules)
add_subdirectory(tests)
add_subdirectory(algorithms)
add_subdirectory(arena)
add_subdirectory(presentation)

set(COMMON_LIBRARIES mcts monte_carlo random_game_algorithm minimax alpha_beta zobrist_hasher transposition_table game_algorithm mc_utils splits)
set(GRADER_LIBRARIES simple_grader advanced_grader)

message(STATUS "common libraries " ${COMMON_LIBRARIES})
message(STATUS "graders " ${GRADER_LIBRARIES})

add_executable(sanity tests/sanity.cpp)
target_link_libraries(sanity ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

add_executable(sanity_with_constant_mallocing tests/sanity_with_constant_mallocing.cpp)
target_link_libraries(sanity_with_constant_mallocing ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

add_executable(alg_sanity tests/alg_sanity)
target_link_libraries(alg_sanity ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

add_executable(match arena/match.cpp)
target_link_libraries(match ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

add_executable(tournament arena/tournament.cpp)
target_link_libraries(tournament ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

add_executable(mcts_tournament arena/mcts_tournament.cpp)
target_link_libraries(mcts_tournament ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

add_executable(ab_tournament arena/ab_tournament.cpp)
target_link_libraries(ab_tournament ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})


add_executable(play_match presentation/play_match.cpp)
target_link_libraries(play_match ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

add_executable(alphabeta_speeds presentation/alphabeta_speeds.cpp)
target_link_libraries(alphabeta_speeds ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

# add_executable(transposition_table presentation/transposition_table.cpp)
# target_link_libraries(transposition_table ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

# add_executable(confidentiality_bound presentation/confidentiality_bound.cpp)
# target_link_libraries(confidentiality_bound ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

# add_executable(mcts_mc presentation/mcts_mc.cpp)
# target_link_libraries(mcts_mc ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

# add_executable(mcts_alphabeta presentation/mcts_alphabeta.cpp)
# target_link_libraries(mcts_alphabeta ${COMMON_LIBRARIES} ${GRADER_LIBRARIES})

add_definitions(-Wall)
