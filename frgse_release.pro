######################################################################
# Automatically generated by qmake (2.01a) Fri Sep 2 15:02:49 2011
######################################################################

QT+= core gui opengl
TEMPLATE = app
TARGET = frgse_release
DEPENDPATH += . \
              source/data \
              source/graphics \
              source/data/base \
              source/data/nodes \
              source/data/undo \
              source/graphics/base \
              source/graphics/nodes
INCLUDEPATH += . \
               source/data \
               source/graphics/nodes \
               source/data/nodes \
               source/graphics \
               source/graphics/base \
               source/data/base \
               source/data/undo

# Input
HEADERS += source/data/callbacks.h \
           source/data/shaderwriter.h \
           source/graphics/newnodeeditor.h \
           source/graphics/nodelib.h \
           source/graphics/nodelink.h \
           source/graphics/scenewidgetcontainer.h \
           source/graphics/shader_view.h \
           source/graphics/shaderpreview.h \
           source/data/base/dnspace.h \
           source/data/base/frg.h \
           source/data/base/frg_shader_author.h \
           source/data/base/project.h \
           source/data/nodes/buildin_nodes.h \
           source/data/nodes/data_node.h \
           source/data/nodes/data_node_socket.h \
           source/data/undo/frg_generic_undo.h \
           source/graphics/base/vnspace.h \
           source/graphics/nodes/graphics_node.h \
           source/graphics/nodes/graphics_node_socket.h
SOURCES += source/data/callbacks.cpp \
           source/data/shaderwriter.cpp \
           source/graphics/newnodeeditor.cpp \
           source/graphics/nodelib.cpp \
           source/graphics/nodelink.cpp \
           source/graphics/scenewidgetcontainer.cpp \
           source/graphics/shader_view.cpp \
           source/graphics/shaderpreview.cpp \
           source/data/base/dnspace.cpp \
           source/data/base/frg.cpp \
           source/data/base/frg_shader_author.cpp \
           source/data/base/main.cpp \
           source/data/base/project.cpp \
           source/data/nodes/buildin_nodes.cpp \
           source/data/nodes/data_node.cpp \
           source/data/nodes/data_node_socket.cpp \
           source/data/undo/frg_generic_undo.cpp \
           source/graphics/base/vnspace.cpp \
           source/graphics/nodes/graphics_node.cpp \
           source/graphics/nodes/graphics_node_socket.cpp
