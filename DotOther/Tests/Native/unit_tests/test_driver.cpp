/**
 * \file Tests/Native/test_driver
 **/
#include <gtest/gtest.h>

#ifdef DOTOTHER_WINDOWS
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  ::testing::InitGoogleTest(&nCmdShow, __argv);
#else
int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
#endif
  return RUN_ALL_TESTS();
}