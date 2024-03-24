
SET(TOTAL_IMGUICHAT_FILES) # lv1engine �� �׳� �״�� ����� ;

SET(LAYER 
    Layer/ChatServerLayer.h
    Layer/ChatClientLayer.h
    Layer/EchoTCPClientLayer.h
    Layer/EchoTCPServerLayer.h

    Layer/ChatServerLayer.cpp
    Layer/ChatClientLayer.cpp
    Layer/EchoTCPClientLayer.cpp
    Layer/EchoTCPServerLayer.cpp
)
SOURCE_GROUP("layer" FILES ${LAYER})
LIST(APPEND TOTAL_IMGUICHAT_FILES ${LAYER})

SET(APP 
    mainApp.cpp
)
SOURCE_GROUP("app" FILES ${APP})
LIST(APPEND TOTAL_IMGUICHAT_FILES ${APP})

add_executable(
    ${EXE_IMGUIChat} 
    ${TOTAL_IMGUICHAT_FILES})

target_link_libraries(
    ${EXE_IMGUIChat} 
    PRIVATE
    ${ENGINE_NAME})

# Dependency Tree �� ����� �ش�.
# - � target �� build �� ��, �� ������ �ٸ� target �� build ����
add_dependencies(
    ${EXE_IMGUIChat} 
    ${ENGINE_NAME}
)
    
# Set the output directory of the dll, exe
set_target_properties(
    ${EXE_IMGUIChat} 
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY
    ${RUNTIME_OUTPUT_DIR}
)

