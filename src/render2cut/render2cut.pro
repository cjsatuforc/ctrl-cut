TARGET = render2cut
#CONFIG += raster
CONFIG += gsapi
CONFIG += boost
CONFIG += cups
CONFIG += rsvg
CONFIG += cairo-ps
CONFIG += gio
CONFIG += libxml++
CONFIG += magick++
CONFIG += libpng
CONFIG += libctrl-cut

include(common.pri)
include(cups.pri)

DEFINES += ETLOG DEBUG=4

HEADERS += ./CupsGetOpt.hpp \
			./CupsOptions.hpp

SOURCES += ./Render2Cut.cpp \
			./CupsGetOpt.cpp \ 
			./CupsOptions.cpp
