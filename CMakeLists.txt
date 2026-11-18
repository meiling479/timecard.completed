cmake_minimum_required(VERSION 3.19)
project(TimeCard LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(TimeCard
    WIN32 MACOSX_BUNDLE
    main.cpp
    Mainwindow.cpp
    Mainwindow.h
    Mainwindow.ui
    plandialog.h plandialog.cpp plandialog.ui
    deleteplandialog.h deleteplandialog.cpp deleteplandialog.ui
    editrecorddialog.h editrecorddialog.cpp editrecorddialog.ui
)

target_link_libraries(TimeCard
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS TimeCard
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET TimeCard
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
