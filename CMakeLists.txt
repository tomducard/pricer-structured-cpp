cmake_minimum_required(VERSION 3.15)
project(pricer-structured LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Widgets Charts REQUIRED)

add_executable(pricer_gui
        main/main.cpp
        src/MarketData.cpp
        src/AutocallBase.cpp
        src/AirbagAutocall.cpp
        src/SimpleAutocall.cpp
        src/MemoryPhoenixAutocall.cpp
        src/StepDownAutocall.cpp
        src/PhoenixAutocall.cpp
        src/CliquetBase.cpp
        src/CliquetMaxReturn.cpp
        src/CliquetCappedCoupons.cpp
        src/BlackScholesMC.cpp
        src/HestonMC.cpp
        src/InputUtils.cpp
        src/PricerRunner.cpp
)

target_include_directories(pricer_gui PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(pricer_gui PRIVATE Qt6::Widgets Qt6::Charts)