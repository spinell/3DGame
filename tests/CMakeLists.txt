find_package(GTest 1.15.2 EXACT CONFIG REQUIRED)

add_executable(Test1 Test1.cpp)
target_link_libraries(
    Test1
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
)
add_test(NAME Test1 COMMAND Test1)

add_executable(Test2 Test2.cpp)
target_link_libraries(
    Test2
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
)
add_test(NAME Test2 COMMAND Test2)

add_executable(Test3 Test3.cpp)
target_link_libraries(
    Test3
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
)
add_test(NAME Test3_1 COMMAND Test3)
add_test(NAME Test3_2 COMMAND Test3)
