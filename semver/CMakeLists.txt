cmake_minimum_required(VERSION 2.8.12)
project(semver)

__add_real_target(semver lib SOURCE
    semver.c
    semver.h
	FOLDER slice
)

#encoding_check(semver)
