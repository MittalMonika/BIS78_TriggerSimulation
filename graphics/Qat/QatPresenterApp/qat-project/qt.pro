######################################################################
# Automatically generated by qmake (2.01a) Mon Feb 1 12:06:28 2010
######################################################################

TEMPLATE = app
TARGET = qat-project
DEPENDPATH += .
INCLUDEPATH += .

# Input
FORMS   += qat-project.ui
SOURCES += qat-project.cpp QatProjectForm.cpp
HEADERS += QatProjectForm.h

INSTALLS += target
target.path=/usr/local/bin

templates.path  = /usr/local/share/templates
templates.files = TEMPLATES/*.cpp TEMPLATES/*.pro
INSTALLS += templates
mac {
  CONFIG -= app_bundle
}

