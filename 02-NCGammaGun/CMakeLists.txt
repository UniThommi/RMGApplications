cmake_minimum_required(VERSION 3.8)
# Set up from 01-FullCosmogenics: project and executable keep the name FullCosmogenics.
# Requires the remage fork, which provides MyTrackInfo.
project(FullCosmogenics)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(remage REQUIRED COMPONENTS GDML)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(
  FullCosmogenics
  main.cc
  src/CosmogenicPhysics.cc
  src/RNGCerenkov.cc
  src/RNGTrackingAction.cc
  src/MySteppingAction.cc
  src/MyPrimaryGammaUserInfo.cc
  src/MySingleNCGammaGenerator.cc
  src/MyMuonGammaGenerator.cc
  src/DebugVertexOutputScheme.cc)

target_link_libraries(FullCosmogenics PUBLIC RMG::remage)

target_include_directories(FullCosmogenics PUBLIC ${PROJECT_SOURCE_DIR}/include)
