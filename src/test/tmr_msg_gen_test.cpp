#include <gtest/gtest.h>

#include "tmr_listener_handle/tmr_motion_function.hpp"

TEST(ChecksumTest, ChecksumStringMatch) {
  using namespace tm_robot_listener::motion_function;

  EXPECT_EQ(calculate_checksum("$TMSTA,10,01,08,true,"), "6D");
  EXPECT_EQ(calculate_checksum("$TMSTA,5,01,15,"), "6F");
  EXPECT_EQ(calculate_checksum("$TMSCT,5,10,OK,"), "6D");
  EXPECT_EQ(calculate_checksum("$TMSCT,4,1,OK,"), "5C");
  EXPECT_EQ(calculate_checksum("$TMSCT,8,2,OK;2;3,"), "52");
  EXPECT_EQ(calculate_checksum("$TMSCT,13,3,ERROR;1;2;3,"), "3F");
  EXPECT_EQ(calculate_checksum("$TMSCT,25,1,ChangeBase(\"RobotBase\"),"), "08");
  EXPECT_EQ(calculate_checksum("$TMSCT,64,2,ChangeBase(\"RobotBase\")\r\nChangeTCP(\"NOTOOL\")\r\nChangeLoad(10.1),"),
            "68");
  EXPECT_EQ(calculate_checksum("$TMSCT,40,3,int var_i = 100\r\nvar_i = 1000\r\nvar_i++,"), "5A");
  EXPECT_EQ(calculate_checksum("$TMSCT,42,4,int var_i = 100\r\nvar_i = 1000\r\nvar_i++\r\n,"), "58");
  EXPECT_EQ(calculate_checksum("$TMSCT,9,4,ERROR;1,"), "02");
  EXPECT_EQ(calculate_checksum("$TMSTA,9,00,false,,"), "37");
  EXPECT_EQ(calculate_checksum("$TMSTA,15,00,true,Listen1,"), "79");
  EXPECT_EQ(calculate_checksum("$TMSTA,2,00,"), "41");
  EXPECT_EQ(calculate_checksum("$TMSTA,10,01,15,none,"), "7D");
  EXPECT_EQ(calculate_checksum("$TMSTA,5,01,88,"), "6B");
  EXPECT_EQ(calculate_checksum("$TMSTA,10,01,88,none,"), "79");
  EXPECT_EQ(calculate_checksum("$TMSCT,-100,1,ChangeBase(\"RobotBase\"),"), "13");
  EXPECT_EQ(calculate_checksum("$CPERR,2,01,"), "49");
  EXPECT_EQ(calculate_checksum("$CPERR,2,02,"), "4A");
  EXPECT_EQ(calculate_checksum("$TMsct,25,1,ChangeBase(\"RobotBase\"),"), "28");
  EXPECT_EQ(calculate_checksum("$CPERR,2,03,"), "4B");
  EXPECT_EQ(calculate_checksum("$TMSCT,23,ChangeBase(\"RobotBase\"),"), "13");
  EXPECT_EQ(calculate_checksum("$CPERR,2,04,"), "4C");
  EXPECT_EQ(calculate_checksum("$TMSTA,4,XXXX,"), "47");
  EXPECT_EQ(calculate_checksum("$CPERR,2,F1,"), "3F");

  EXPECT_EQ(calculate_checksum("$TMSCT,172,2,float[] targetP1= {0,0,90,0,90,0}\r\n"
                               "PTP(”JPP”,targetP1,10,200,0,false)\r\nQueueTag(1)\r\n"
                               "float[] targetP2 = { 0, 90, 0, 90, 0, 0 }\r\n"
                               "PTP(”JPP”, targetP2, 10, 200, 10, false)\r\n"
                               "QueueTag(2)\r\n,"),
            "49");
}

#define EXPECT(TYPE) Expression<TYPE>
#define VARIABLE_BINARY_OP_TEST(VAR_1, OP, VAR_2, RESULT_TYPE)                                        \
  {                                                                                                   \
    auto expr = VAR_1 OP VAR_2;                                                                       \
    static_assert(std::is_same<decltype(expr), RESULT_TYPE>::value, "Expression type doesn't match"); \
    EXPECT_EQ(expr(), "(" #VAR_1 #OP #VAR_2 ")");                                                     \
  }

TEST(VariableTest, BinaryOperator) {
  using namespace tm_robot_listener;

  Variable<int> other_int{"other_int"};
  Variable<int> int_var{"int_var"};
  Variable<float> float_var{"float_var"};

  EXPECT_EQ(declare(int_var, 0)(), "int int_var=0");

  VARIABLE_BINARY_OP_TEST(int_var, =, 1, EXPECT(int));
  VARIABLE_BINARY_OP_TEST(int_var, =, float_var, EXPECT(int));

  VARIABLE_BINARY_OP_TEST(int_var, +, 1, EXPECT(int));
  VARIABLE_BINARY_OP_TEST(1, +, int_var, EXPECT(int));

  VARIABLE_BINARY_OP_TEST(1.5, +, int_var, EXPECT(double));
  VARIABLE_BINARY_OP_TEST(int_var, +, 1.5, EXPECT(double));

  VARIABLE_BINARY_OP_TEST(int_var, +, other_int, EXPECT(int));

  VARIABLE_BINARY_OP_TEST(int_var, +, float_var, EXPECT(float));
}

TEST(VariableTest, UnaryOperator) {
  using namespace tm_robot_listener;

  Variable<int> int_var{"int_var"};
  Variable<bool> bool_var{"bool_var"};

  {
    auto expr = int_var++;
    static_assert(std::is_same<decltype(expr), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(expr(), "(int_var++)");
  }

  {
    auto expr = ++int_var;
    static_assert(std::is_same<decltype(expr), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(expr(), "(++int_var)");
  }

  {
    auto expr = !bool_var;
    static_assert(std::is_same<decltype(expr), Expression<bool>>::value, "Expression type doesn't match");
    EXPECT_EQ(expr(), "(!bool_var)");
  }

  {
    auto expr = ~int_var;
    static_assert(std::is_same<decltype(expr), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(expr(), "(~int_var)");
  }

  {
    auto expr = -int_var;
    static_assert(std::is_same<decltype(expr), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(expr(), "(-int_var)");
  }

  {
    auto expr = +int_var;
    static_assert(std::is_same<decltype(expr), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(expr(), "(+int_var)");
  }
}

TEST(ExpressionTest, BinaryOperator) {
  using namespace tm_robot_listener;

  Variable<int> int_var{"int_var"};
  Variable<int> other_int{"other_int"};
  Variable<float> float_var{"float_var"};

  Expression<int> int_expr       = int_var + other_int;
  Expression<int> other_int_expr = int_var + 1;
  Expression<float> float_expr   = int_var + float_var;

  {
    auto add_two_int_expr = int_expr + other_int_expr;
    static_assert(std::is_same<decltype(add_two_int_expr), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(add_two_int_expr(), "((int_var+other_int)+(int_var+1))");
  }

  {  // expression with different type
    auto add_int_to_float_expr = int_expr + float_expr;
    static_assert(std::is_same<decltype(add_int_to_float_expr), Expression<float>>::value,
                  "Expression type doesn't match");
    EXPECT_EQ(add_int_to_float_expr(), "((int_var+other_int)+(int_var+float_var))");
  }

  {  // expression + r-value
    auto add_int_expr = int_expr + 1;
    static_assert(std::is_same<decltype(add_int_expr), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(add_int_expr(), "((int_var+other_int)+1)");
  }

  {  // expression + variable
    auto add_int_expr = int_expr + int_var;
    static_assert(std::is_same<decltype(add_int_expr), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(add_int_expr(), "((int_var+other_int)+int_var)");
  }

  {  // variable + expression
    auto add_int_expr = int_var + int_expr;
    static_assert(std::is_same<decltype(add_int_expr), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(add_int_expr(), "(int_var+(int_var+other_int))");
  }

  {
    Variable<bool> bool_var{"bool_var"};

    auto tern_expr_3_var = ternary_expr<int>(bool_var, int_var, float_var);
    static_assert(std::is_same<decltype(tern_expr_3_var), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(tern_expr_3_var(), "(bool_var?int_var:float_var)");

    auto tern_expr_3_expr = ternary_expr<int>(int_var == 1, int_var + other_int, float_var + int_var);
    static_assert(std::is_same<decltype(tern_expr_3_expr), Expression<int>>::value, "Expression type doesn't match");
    EXPECT_EQ(tern_expr_3_expr(), "((int_var==1)?(int_var+other_int):(float_var+int_var))");
  }
}

TEST(TMMsgGen, StringMatch) {
  using namespace tm_robot_listener::motion_function;
  using namespace std::string_literals;

  {
    auto const command = TMSCT << ID{"1"} << ChangeBase("RobotBase"s) << End();
    EXPECT_EQ(command->to_str(), "$TMSCT,25,1,ChangeBase(\"RobotBase\"),*08\r\n");
  }

  {
    auto const command = TMSTA << QueueTagDone(88) << End();
    EXPECT_EQ(command->to_str(), "$TMSTA,5,01,88,*6B\r\n");
  }

  {
    using namespace tm_robot_listener;
    Variable<std::array<float, 6>> targetP1{"targetP1"};
    Variable<std::array<float, 6>> targetP2{"targetP2"};

    auto const command = TMSCT << ID{"2"} << declare(targetP1, std::array<float, 6>{205, -35, 125, 0, 90, 0})
                               << PTP("JPP"s, targetP1, 10, 200, 0, false) << QueueTag(1)
                               << declare(targetP2, std::array<float, 6>{90, -35, 125, 0, 90, 0})
                               << PTP("JPP"s, targetP2, 10, 200, 10, false) << QueueTag(2) << End();
    EXPECT_EQ(command->to_str(),
              "$TMSCT,176,2,float[] targetP1={205,-35,125,0,90,0}\r\n"
              "PTP(\"JPP\",targetP1,10,200,0,false)\r\n"
              "QueueTag(1)\r\n"
              "float[] targetP2={90,-35,125,0,90,0}\r\n"
              "PTP(\"JPP\",targetP2,10,200,10,false)\r\n"
              "QueueTag(2),*54\r\n");
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}