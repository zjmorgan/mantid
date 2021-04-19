# CMake generated Testfile for 
# Source directory: /SNS/users/zgf/.git/mantid/Framework/PythonInterface/test/python/mantid/_plugins
# Build directory: /SNS/users/zgf/.git/mantid/Framework/PythonInterface/test/python/mantid/_plugins
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(python._plugins.ProductFunctionTest.ProductFunctionTest "/usr/bin/cmake3" "-E" "chdir" "/SNS/users/zgf/.git/mantid/bin/Testing" "/SNS/users/zgf/.git/mantid/bin/mantidpython" "--classic" "/SNS/users/zgf/.git/mantid/Framework/PythonInterface/test/testhelpers/testrunner.py" "/SNS/users/zgf/.git/mantid/Framework/PythonInterface/test/python/mantid/_plugins/ProductFunctionTest.py")
set_tests_properties(python._plugins.ProductFunctionTest.ProductFunctionTest PROPERTIES  ENVIRONMENT "PYTHONPATH=/SNS/users/zgf/.git/mantid/Framework/PythonInterface/test/python/mantid/_plugins::" TIMEOUT "300" _BACKTRACE_TRIPLES "/SNS/users/zgf/.git/mantid/buildconfig/CMake/PyUnitTest.cmake;53;add_test;/SNS/users/zgf/.git/mantid/Framework/PythonInterface/test/python/mantid/_plugins/CMakeLists.txt;9;pyunittest_add_test;/SNS/users/zgf/.git/mantid/Framework/PythonInterface/test/python/mantid/_plugins/CMakeLists.txt;0;")
