# idf_component_register(SRCS "tcp_client.c" "TempSensor.c" INCLUDE_DIRS ".")
file(GLOB_RECURSE MY_SOURCES *.c)
idf_component_register(SRCS ${MY_SOURCES}
                    INCLUDE_DIRS ".")