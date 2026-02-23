file(
  GLOB FASTOML_CPP_EXAMPLE_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

foreach(example_source IN LISTS FASTOML_CPP_EXAMPLE_SOURCES)
  get_filename_component(example_name "${example_source}" NAME_WE)
  add_executable("${example_name}" "${example_source}")
  target_link_libraries("${example_name}" PRIVATE fastoml-cpp)
  target_compile_features("${example_name}" PRIVATE cxx_std_23)
endforeach()
