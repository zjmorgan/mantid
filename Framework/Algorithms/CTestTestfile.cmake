# CMake generated Testfile for 
# Source directory: /SNS/users/zgf/.git/mantid/Framework/Algorithms
# Build directory: /SNS/users/zgf/.git/mantid/Framework/Algorithms
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(python.NormaliseToUnityTest.NormaliseToUnityTest "/usr/bin/cmake3" "-E" "chdir" "/SNS/users/zgf/.git/mantid/bin/Testing" "/SNS/users/zgf/.git/mantid/bin/mantidpython" "--classic" "/SNS/users/zgf/.git/mantid/Framework/PythonInterface/test/testhelpers/testrunner.py" "/SNS/users/zgf/.git/mantid/Framework/Algorithms/test/NormaliseToUnityTest.py")
set_tests_properties(python.NormaliseToUnityTest.NormaliseToUnityTest PROPERTIES  ENVIRONMENT "PYTHONPATH=/SNS/users/zgf/.git/mantid/Framework/Algorithms::" TIMEOUT "300" _BACKTRACE_TRIPLES "/SNS/users/zgf/.git/mantid/buildconfig/CMake/PyUnitTest.cmake;53;add_test;/SNS/users/zgf/.git/mantid/Framework/Algorithms/CMakeLists.txt;1003;pyunittest_add_test;/SNS/users/zgf/.git/mantid/Framework/Algorithms/CMakeLists.txt;0;")
subdirs("test")
