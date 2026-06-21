add_executable(test_point test_point.cpp ../src/point.cpp)
target_link_libraries(test_point PRIVATE GTest::gtest_main)
add_test(NAME test_point COMMAND test_point)

add_executable(test_rect test_rect.cpp ../src/rect.cpp)
target_link_libraries(test_rect PRIVATE GTest::gtest_main)
add_test(NAME test_rect COMMAND test_rect)

add_executable(test_extraText test_extraText.cpp ../src/extraText.cpp ../src/logger.cpp)
target_link_libraries(test_extraText PRIVATE GTest::gtest_main)
add_test(NAME test_extraText COMMAND test_extraText)

# Options death tests (redo find_package because scope from src/ doesn't propagate)
find_package(boost_program_options REQUIRED)
add_executable(test_options test_options.cpp ../src/options.cpp ../src/logger.cpp)
target_link_libraries(test_options PRIVATE GTest::gtest_main boost_program_options)
add_test(NAME test_options COMMAND test_options)

# Integration tests (requires full project dependencies)
find_package(qpdf REQUIRED)
find_package(ImageMagick REQUIRED COMPONENTS Magick++)

add_executable(test_integration
    test_integration.cpp
    ../src/imageProvider.cpp
    ../src/pdfProcessor.cpp
    ../src/options.cpp
    ../src/logger.cpp
    ../src/extraText.cpp
    ../src/rect.cpp
    ../src/point.cpp
)

target_compile_definitions(test_integration PRIVATE
    MAGICKCORE_QUANTUM_DEPTH=16
    MAGICKCORE_HDRI_ENABLE=0
    POINTERHOLDER_TRANSITION=4
    TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/tests/data"
    QPDF_IMAGE_EMBED_BINARY="${CMAKE_BINARY_DIR}/src/qpdfImageEmbed"
)

target_include_directories(test_integration PRIVATE
    "${ImageMagick_Magick++_INCLUDE_DIRS}"
)

target_link_libraries(test_integration PRIVATE
    GTest::gtest
    qpdf::libqpdf
    boost_program_options
    "${ImageMagick_Magick++_LIBRARY}"
    qrencode
)

add_test(NAME test_integration COMMAND test_integration WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
