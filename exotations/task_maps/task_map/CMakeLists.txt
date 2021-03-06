cmake_minimum_required(VERSION 2.8.3)
project(task_map)

add_definitions(-DEXOTICA_DEBUG_MODE)

find_package(catkin REQUIRED COMPONENTS
  exotica
  geometry_msgs
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS exotica geometry_msgs
  DEPENDS #system_lib
)

AddInitializer(CoM IMesh EffPosition JointLimit SweepFlux Distance Identity SphereCollision Sphere)
GenInitializers()

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} src/CoM.cpp
                            src/IMesh.cpp
                            src/EffPosition.cpp
                            src/JointLimit.cpp
                            src/SweepFlux.cpp
                            src/Distance.cpp
                            src/Identity.cpp
                            src/SphereCollision.cpp
           )


target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES})

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_initializers)

## Install
install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
install(DIRECTORY include/ DESTINATION include)
