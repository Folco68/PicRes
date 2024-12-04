cmake_minimum_required(VERSION 3.16)

project(PicRes VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    # Root
    BeforeRelease.hpp
    Global.hpp
    main.cpp

    # Core
    Core/DropThread.cpp
    Core/DropThread.hpp
    Core/ResizeThread.cpp
    Core/ResizeThread.hpp

    # Docs
    Docs/Docs.qrc
    Docs/Changelog.txt
    Docs/License.txt
    Docs/Readme.txt
    Docs/Todo.txt

    #Resources

    # UI
    UI/DlgErrorList.cpp
    UI/DlgErrorList.hpp
    UI/DlgErrorList.ui
    UI/DlgHelp.cpp
    UI/DlgHelp.hpp
    UI/DlgHelp.ui
    UI/Dropbox.cpp
    UI/Dropbox.hpp
    UI/MainWindow.cpp
    UI/MainWindow.hpp
    UI/MainWindow.ui
    UI/TableItem.cpp
    UI/TableItem.hpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PicRes
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Resources/Icon.qrc
        Resources/Icon.rc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PicRes APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(PicRes SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PicRes
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(PicRes PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PicRes)
endif()
set_target_properties(PicRes PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PicRes
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PicRes)
endif()
