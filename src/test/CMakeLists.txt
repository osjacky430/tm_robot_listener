include(AddCompileTimeTest)

# cmake-format: off
test_ext_script_syntax(TEST_CASE "CMD_MUST_END"
                       FAIL_EXPR "TMSCT << ID{\"1\"} << QueueTag(1, 1)"
                       PASS_EXPR "TMSCT << ID{\"1\"} << QueueTag(1, 1) << End()"
                       ERR_MSG_REGEX "error: could not convert")

test_ext_script_syntax(TEST_CASE "NO_CMD_AFTER_END"
                       FAIL_EXPR "TMSCT << ID{\"1\"} << QueueTag(1, 1) << End() << QueueTag(1, 1)"
                       PASS_EXPR "TMSCT << ID{\"1\"} << QueueTag(1, 1) << End()"
                       ERR_MSG_REGEX "error: no match for ‘operator<<’")

test_ext_script_syntax(TEST_CASE "NO_CMD_AFTER_SCRIPT_EXIT"
                       FAIL_EXPR "TMSCT << ID{\"1\"} << QueueTag(1, 1) << ScriptExit() << QueueTag(1, 1)"
                       PASS_EXPR "TMSCT << ID{\"1\"} << QueueTag(1, 1) << ScriptExit()"
                       ERR_MSG_REGEX "error: no match for ‘operator<<’")

test_ext_script_syntax(TEST_CASE "TMSCT_ID_MUST_BE_FIRST"
                       FAIL_EXPR "TMSCT << QueueTag(1, 1) << ScriptExit()"
                       PASS_EXPR "TMSCT << ID{\"1\"} << QueueTag(1, 1) << ScriptExit()")

test_ext_script_syntax(TEST_CASE "ID_IS_TMSCT_ONLY"
                       FAIL_EXPR "TMSTA << ID{\"1\"} << End()"
                       PASS_EXPR "TMSCT << ID{\"1\"} << End()")

test_ext_script_syntax(TEST_CASE "TMSTA_NO_TMSCT_CMD"
                       FAIL_EXPR "TMSTA << QueueTag(1, 1) << End()"
                       PASS_EXPR "TMSTA << QueueTagDone(1) << End()")

test_ext_script_syntax(TEST_CASE "SCRIPT_EXIT_IS_TMSCT_ONLY"
                       FAIL_EXPR "TMSTA << QueueTagDone(1) << ScriptExit()"
                       PASS_EXPR "TMSTA << QueueTagDone(1) << End()")

# test_ext_script_syntax(TEST_CASE "VAR_DECLARATION_IS_TMSCT_ONLY"
#                        FAIL_EXPR "TMSTA << var_test << End()"
#                        PASS_EXPR "TMSCT << ID{\"1\"} << var_test << End()")

test_ext_script_syntax(TEST_CASE "TMSTA_ONLY_1_CMD_PER_SCRIPT"
                       FAIL_EXPR "TMSTA << QueueTagDone(1) << QueueTagDone(1) << End()"
                       PASS_EXPR "TMSTA << QueueTagDone(1) << End()")

# cmake-format: on

# stupid catkin_add_gtest function, catkin just sucks really hard, I must disable warning as error
# since I have no control over adding gtest dependency, at least I do it myself
catkin_add_gtest(tmr_msg_gen tmr_msg_gen_test.cpp)
target_include_directories(tmr_msg_gen PRIVATE ${CMAKE_SOURCE_DIR}/include)

catkin_add_gtest(tmr_msg_parse tmr_msg_parse_test.cpp)
target_link_libraries(tmr_msg_parse tm_robot_listener)
target_include_directories(tmr_msg_parse PRIVATE ${CMAKE_SOURCE_DIR}/include)
