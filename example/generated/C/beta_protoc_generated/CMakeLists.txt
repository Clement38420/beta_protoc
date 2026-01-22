project(beta_protoc_generated LANGUAGES C)

add_library(beta_protoc_generated STATIC src/dispatcher.c src/SensorData.c src/Value.c )
target_include_directories(beta_protoc_generated PUBLIC include)