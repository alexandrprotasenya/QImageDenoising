## SYSTEM_TYPE - compiling for 32 or 64 bit architecture
SYSTEM_TYPE = 64

## CUDA_COMPUTE_ARCH - This will enable nvcc to compiler appropriate architecture specific code for different compute versions.
## Multiple architectures can be requested by using a space to seperate. example:
## CUDA_COMPUTE_ARCH = 10 20 30 35
CUDA_COMPUTE_ARCH = 30

## CUDA_DEFINES - The seperate defines needed for the cuda device and host methods
CUDA_DEFINES +=

## CUDA_DIR - the directory of cuda such that CUDA\<version-number\ contains the bin, lib, src and include folders
CUDA_DIR= "c:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v6.5"

## CUDA_SOURCES - the source (generally .cu) files for nvcc. No spaces in path names
CUDA_SOURCES+=nlm_classic.cu

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


  CUDA_LIBS_DIR = "$$CUDA_DIR/lib/x64"
  QMAKE_LFLAGS += /LIBPATH:\"$$CUDA_DIR/lib/x64\"
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
