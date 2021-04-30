# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
set(EXTRA_COMPONENT_DIRS "components/peripheral"
			 "components/user_driver"
			 "components/task"
			 "components/json_parser"
			 )
#set(COMPONENTS bootloader esptool_py 
#			main 
#			peripheral 
#			user_driver
#			task
#			json_parser
#			)  # This reduces the number of redundant components included

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Anywave)
