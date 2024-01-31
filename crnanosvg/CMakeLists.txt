cmake_minimum_required(VERSION 2.8.12)
project(crnanosvg)

add_library(crnanosvg STATIC
    nanosvg.h
)

#encoding_check(semver)
