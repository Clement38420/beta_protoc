cmake_minimum_required(VERSION 3.22)
project(beta_protoc LANGUAGES C)

add_library(beta_protoc STATIC src/beta_protoc.c)
target_include_directories(beta_protoc PUBLIC include)