#------------------------------------------------------------------------------
# common
#------------------------------------------------------------------------------
include (../g/g.pri)

#------------------------------------------------------------------------------
# k_NAME
#------------------------------------------------------------------------------
K_NAME = pqc-vpn
CONFIG(qt): contains(QT, gui) K_NAME = $${K_NAME}-gui
CONFIG(debug, debug|release) K_NAME = $${K_NAME}-d
android: K_NAME = $${K_NAME}-android

#------------------------------------------------------------------------------
# K_DIR
#------------------------------------------------------------------------------
K_DIR = $${PWD}
INCLUDEPATH *= $${K_DIR}/src
!CONFIG(K_BUILD) {
        PRE_TARGETDEPS *= $${K_DIR}/bin/lib$${K_NAME}.a
        LIBS *= -L$${K_DIR}/bin -l$${K_NAME}
}
