### cpp-transport-catalogue
# Транспортный справочник / роутер
Система хранения транспортных маршрутов и обработки запросов к ней:
- Входные данные и ответ в JSON-формате.
- Выходной JSON-файл может содержать визуализацию карты маршрута(ов) в формате SVG-файла.  
- Поиск кратчайшего маршрута. 
- Сериализация базы данных и настроек справочника при помощи Google Protobuf. 
- Объекты JSON поддерживают цепочки вызовов (method chaining) при конструировании, превращая ошибки применения данных формата JSON в ошибки компиляции.

### Использованные идеомы, технологии и элементы языка
- OOP: inheritance, abstract interfaces, final classes
- Unordered map/set
- STL smart pointers
- std::variant and std:optional
- JSON load / output
- SVG image format embedded inside XML output
- Curiously Recurring Template Pattern (CRTP)
- Method chaining
- Facade design pattern
- C++17 with С++20 Ranges emulation
- Directed Weighted Graph data structure for Router module
- Google Protocol Buffers for data serialization
- Static libraries .LIB/.A
- CMake generated project and dependency files

## Сборка проекта CMake
```
cmake_minimum_required(VERSION 3.10)

project(PhoneBook CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto transport_router.proto)

set(TRANSPORT_CATALOGUE_FILES main.cpp domain.h geo.cpp geo.h graph.h json.cpp json.h json_builder.cpp json_builder.h
        json_reader.cpp json_reader.h map_renderer.cpp map_renderer.h ranges.h request_handler.cpp request_handler.h
        router.h svg.cpp svg.h transport_catalogue.cpp transport_catalogue.h transport_router.cpp transport_router.h
        serialization.cpp serialization.h)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOGUE_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
```

## Запуск программы

Приложение транспортного справочника спроектировано для работы в 2 режимах: режиме создания базы данных и режиме запросов к базе данных.

Для создания базы данных транспортного справочника с последующей ее сериализацией в файл необходимо запустить программу с параметром make_base. Входные данные поступают из stdin, поэтому можно переопределить источник данных, например, указав входной JSON-файл, из которого будет взята информация для наполнения базы данных вместо stdin. Пример:
transport_catalogue.exe make_base <input_data.json

Для обработки запросов к созданной базе данных (сама база данных десериализуется из ранее созданного файла) необходимо запустить программу с параметром process_requests, указав входной JSON-файл, содержащий запрос(ы) к БД и выходной файл, который будет содержать ответы на запросы, также в формате JSON.
Пример:
transport_catalogue.exe process_requests <requests.json >output.txt

## Формат входных данных

Входные данные принимаются из stdin в JSON формате. Структура верхнего уровня имеет следующий вид:

{
  "base_requests": [ ... ],
  "render_settings": { ... },
  "routing_settings": { ... },
  "serialization_settings": { ... },
  "stat_requests": [ ... ]
}

Каждый элемент является словарем, содержащим следующие данный:
base_requests — описание автобусных маршрутов и остановок.
stat_requests — запросы к транспортному справочнику.
render_settings — настройки рендеринга карты в формате .SVG.
routing_settings — настройки роутера для поиска кратчайших маршрутов.
serialization_settings — настройки сериализации/десериализации данных.
Примеры входного файла и файла с запросом к справочнику прилагаются.
