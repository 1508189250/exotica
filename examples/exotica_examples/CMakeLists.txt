cmake_minimum_required(VERSION 2.8.3)
project(exotica_examples)

find_package(catkin REQUIRED COMPONENTS
  exotica
  sensor_msgs
  ik_solver
  task_map
)

catkin_package(

)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_executable(ExoticaCore src/core.cpp)

target_link_libraries(ExoticaCore
  ${catkin_LIBRARIES}
)

add_executable(ManualInitialization src/manual.cpp)
target_link_libraries(ManualInitialization ${catkin_LIBRARIES})

add_executable(GenericInitialization src/generic.cpp)
target_link_libraries(GenericInitialization ${catkin_LIBRARIES})

add_executable(XMLInitialization src/xml.cpp)
target_link_libraries(XMLInitialization ${catkin_LIBRARIES})

add_executable(PlannerDemo src/planner.cpp)
target_link_libraries(PlannerDemo ${catkin_LIBRARIES})
