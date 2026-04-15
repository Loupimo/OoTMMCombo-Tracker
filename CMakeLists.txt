cmake_minimum_required(VERSION 3.16)
project(OoTMMCombo-Tracker)

# Trouver Qt6 (assure-toi que Qt6 est installé)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt6_wrap_ui(UI_HEADERS "${CMAKE_SOURCE_DIR}/OoTMMComboTracker.ui")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

# Définir les répertoires
set(RootDir ${CMAKE_SOURCE_DIR})
set(HeaderDir ${RootDir}/Headers)
set(SourceDir ${RootDir}/Sources)
set(ResFile "${RootDir}/OoTMMCombo-Tracker.rc")
set(Main "${RootDir}/main.cpp")


# Ajouter tous les fichiers .cpp et .h
file(GLOB_RECURSE HEADER_FILES ${HeaderDir}/*.h)
file(GLOB_RECURSE SOURCE_FILES ${SourceDir}/*.cpp)

# Créer l'exécutable avec tous les fichiers sources et headers
add_executable(OoTMMCombo-Tracker WIN32 ${SOURCE_FILES} ${Main} ${HEADER_FILES} ${UI_HEADERS} ${ResFile})

# Lier Qt6
target_link_libraries(OoTMMCombo-Tracker PRIVATE Qt6::Widgets)

# Ajouter les include directories
target_include_directories(OoTMMCombo-Tracker PRIVATE Headers ${CMAKE_BINARY_DIR})