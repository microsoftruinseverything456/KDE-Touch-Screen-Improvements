# CMake generated Testfile for 
# Source directory: /home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements
# Build directory: /home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test("appstreamtest" "/usr/bin/cmake" "-DAPPSTREAMCLI=/usr/bin/appstreamcli" "-DINSTALL_FILES=/home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements/build/install_manifest.txt" "-P" "/usr/share/ECM/kde-modules/appstreamtest.cmake")
set_tests_properties("appstreamtest" PROPERTIES  _BACKTRACE_TRIPLES "/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;177;add_test;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;195;appstreamtest;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;0;;/home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements/CMakeLists.txt;13;include;/home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements/CMakeLists.txt;0;")
subdirs("src")
