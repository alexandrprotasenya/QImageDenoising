#-------------------------------------------------
#
# Project created by QtCreator 2014-06-28T19:16:51
#
#-------------------------------------------------

QT       += core

QT       += gui

TARGET = QImageDenoising
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

#QMAKE_LIBS+=-static -fopenmp
#QMAKE_CXXFLAGS+= -std=c++11
QMAKE_CXXFLAGS += /openmp

SOURCES += main.cpp \
    awgn.cpp \
    noise.cpp \
    median_filter.cpp \
    nlm_filter.cpp \
    parrot_test.cpp \
    utils.cpp \
    nlm_filter_gray.cpp \
    nlm_filter_gray2.cpp \
    nlm_filter_gray3.cpp \
#    nlmdenoise.cpp \
    china_denoise.cpp \
    test_nlm.cpp \
    diffimages.cpp \
    nlm_filter_cuda.cpp \
    nlm_random_cuda.cpp

HEADERS += \
    awgn.h \
    noise.h \
    median_filter.h \
    nlm_filter.h \
    parrot_test.h \
    utils.h \
    nlm_filter_gray.h \
    nlm_filter_gray2.h \
    nlm_filter_gray3.h \
 #   nlmdenoise.h \
    china_denoise.h \
    diffimages.h \
    nlm_filter_cuda.h \
    nlm_random_cuda.h

IMAGE_FILES += \
    Lenna.png \
    Parrot_ideal.png \
    Parrot_ideal1.png \
    Parrot_WGN_stdev_25.png

## SYSTEM_TYPE - compiling for 32 or 64 bit architecture
SYSTEM_TYPE = 32

## CUDA_COMPUTE_ARCH - This will enable nvcc to compiler appropriate architecture specific code for different compute versions.
## Multiple architectures can be requested by using a space to seperate. example:
## CUDA_COMPUTE_ARCH = 10 20 30 35
CUDA_COMPUTE_ARCH = 30

## CUDA_DEFINES - The seperate defines needed for the cuda device and host methods
CUDA_DEFINES +=

## CUDA_DIR - the directory of cuda such that CUDA\<version-number\ contains the bin, lib, src and include folders
CUDA_DIR= "c:/CUDA/SDK_7.5"

## CUDA_SOURCES - the source (generally .cu) files for nvcc. No spaces in path names
CUDA_SOURCES += nlm_classic.cu \
                nlm_random.cu

OTHER_FILES += nlm_classic.cu

## CUDA_LIBS - the libraries to link
CUDA_LIBS= -lcuda -lcudart

## CUDA_INC - all incldues needed by the cuda files (such as CUDA\<version-number\include)
CUDA_INC+= $$CUDA_DIR/include

## NVCC_OPTIONS - any further options for the compiler
NVCC_OPTIONS += --use_fast_math --ptxas-options=-v

win32:{
  INCLUDEPATH += $$CUDA_DIR/include

  #Debug and Release flags necessary for compilation and linking
  QMAKE_CFLAGS_DEBUG += /MTd
  QMAKE_CXXFLAGS_DEBUG += /MTd
  QMAKE_CFLAGS_RELEASE += /MT
  QMAKE_CXXFLAGS_RELEASE += /MT
  # The following library conflicts with something in Cuda
  QMAKE_LFLAGS_RELEASE += /NODEFAULTLIB:msvcrt.lib
  QMAKE_LFLAGS_DEBUG += /NODEFAULTLIB:msvcrtd.lib


  CUDA_LIBS_DIR = "$$CUDA_DIR/lib/Win32"
  QMAKE_LFLAGS += /LIBPATH:\"$$CUDA_DIR/lib/Win32\"
  LIBS += -lcuda -lcudart

  win32-msvc2012:contains(QMAKE_TARGET.arch, x86_64):{
       #Can also set SYSTEM_TYPE here
       CONFIG(debug, debug|release) {
            #Debug settings
            message("Using x64 Debug arch config MSVC2012 for build")
            #read as: --compiler-options options,... + ISO-standard C++ exception handling
            # + speed over size, + create debug symbols, + code generation multi-threaded debug
            NVCC_OPTIONS += -Xcompiler /EHsc,/O2,/Zi,/MTd -g
        }
        else {
            #Release settings
            message("Using x64 Release arch config MSVC2012 for build")
            #read as: --compiler-options options,... + ISO-standard C++ exception handling
            # + speed over size, + code generation multi-threaded
            NVCC_OPTIONS += -Xcompiler /EHsc,/O2,/MT
        }
}
}

## correctly formats CUDA_COMPUTE_ARCH to CUDA_ARCH with code gen flags
## resulting format example: -gencode arch=compute_20,code=sm_20
for(_a, CUDA_COMPUTE_ARCH):{
    formatted_arch =$$join(_a,'',' -gencode arch=compute_',',code=sm_$$_a')
    CUDA_ARCH += $$formatted_arch
}

## correctly formats CUDA_DEFINES for nvcc
for(_defines, CUDA_DEFINES):{
    formatted_defines += -D$$_defines
}
CUDA_DEFINES = $$formatted_defines

#nvcc config
CONFIG(debug, debug|release) {
        #Debug settings
        CUDA_OBJECTS_DIR = cudaobj/$$SYSTEM_NAME/Debug
        cuda_d.input = CUDA_SOURCES
        cuda_d.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
        cuda_d.commands = $$CUDA_DIR/bin/nvcc -D_DEBUG $$CUDA_DEFINES $$NVCC_OPTIONS -I\"$$CUDA_INC\" $$CUDA_LIBS --machine $$SYSTEM_TYPE $$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
        cuda_d.dependency_type = TYPE_C
        QMAKE_EXTRA_COMPILERS += cuda_d
}
else {
        # Release settings
        CUDA_OBJECTS_DIR = cudaobj/$$SYSTEM_NAME/Release
        cuda.input = CUDA_SOURCES
        cuda.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
        cuda.commands = $$CUDA_DIR/bin/nvcc $$CUDA_DEFINES $$NVCC_OPTIONS -I\"$$CUDA_INC\" $$CUDA_LIBS --machine $$SYSTEM_TYPE $$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
        cuda.dependency_type = TYPE_C
        QMAKE_EXTRA_COMPILERS += cuda
}

win32 {
    DESTDIR_WIN = $${DESTDIR}
    DESTDIR_WIN ~= s,/,\\,g
    PWD_WIN = $${PWD}
    PWD_WIN ~= s,/,\\,g
    for(FILE, IMAGE_FILES){
        QMAKE_POST_LINK += $$quote(cmd /c copy /y $${PWD_WIN}\\$${FILE} $${DESTDIR_WIN}$$escape_expand(\\n\\t))
    }
}
#unix {
#    for(FILE, IMAGE_FILES){
#        QMAKE_POST_LINK += $$quote(cp $${PWD}/$${FILE} $${DESTDIR}$$escape_expand(\\n\\t))
#    }
#

DISTFILES += \
    nlm_random.cu
