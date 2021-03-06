
######################################################################
# Automatically generated by qmake (2.01a) Sun Oct 19 02:42:55 2008
######################################################################

TEMPLATE = lib
TARGET = 
DEPENDPATH += .
INCLUDEPATH += . .. ../../QatPlotting ../../QatDataModelling ../../QatDataAnalysis
DESTDIR=../../../lib
FORMS=LabelEditorDialog.ui PVControl.ui PlotDirDialog.ui ParameterDialog.ui
CONFIG += qt debug 
QT     += svg 
LIBS = -lQatPlotting -lQatDataModeling -lQatDataAnalysis -lCLHEP
# Input
HEADERS += ../QatPlotWidgets/CustomRangeDivider.h \
           ../QatPlotWidgets/RangeDivision.h   \
           ../QatPlotWidgets/AbsRangeDivider.h \
           ../QatPlotWidgets/LinearRangeDivider.h \
           ../QatPlotWidgets/LogRangeDivider.h \
           ../QatPlotWidgets/PlotView.h \
           ../QatPlotWidgets/LabelEditorDialog.h \
           ../QatPlotWidgets/characterwidget.h \
           ../QatPlotWidgets/PVControl.h \
           ../QatPlotWidgets/PlotHist1DDialog.h \
           ../QatPlotWidgets/PlotHist2DDialog.h \
           ../QatPlotWidgets/PlotDirDialog.h \
           ../QatPlotWidgets/PlotFunction1DDialog.h \
           ../QatPlotWidgets/ParameterDialog.h \
           ../QatPlotWidgets/LinearSubdivider.h \
           ../QatPlotWidgets/LogSubdivider.h

SOURCES += CustomRangeDivider.cpp \
           RangeDivision.cpp \
           AbsRangeDivider.cpp \
           LinearRangeDivider.cpp \
           LogRangeDivider.cpp \
           PlotView.cpp \
           LabelEditorDialog.cpp \
           characterwidget.cpp \
           PVControl.cpp \
           PlotHist1DDialog.cpp \
           PlotHist2DDialog.cpp \
           PlotDirDialog.cpp \
           PlotFunction1DDialog.cpp \
           ParameterDialog.cpp \
           LinearSubdivider.cpp \
           LogSubdivider.cpp

target.path=/usr/local/lib
headers.path=/usr/local/include/QatPlotWidgets
headers.files= ../QatPlotWidgets/*.h ../QatPlotWidgets/*.icc ui_PlotDirDialog.h ui_LabelEditorDialog.h ui_PVControl.h ui_ParameterDialog.h

INSTALLS += target
INSTALLS += headers 
RESOURCES = PlotWidgets.qrc
