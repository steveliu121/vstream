add_definitions(-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -ftree-vectorize -pipe -Wno-psabi")
include_directories(${SDKSTAGE}/opt/vc/include/ ${SDKSTAGE}/opt/vc/include/interface/vcos/pthreads ${SDKSTAGE}/opt/vc/include/interface/vmcs_host/linux ${SDKSTAGE}/opt/vc/src/hello_pi/libs/ilclient ${SDKSTAGE}/opt/vc/src/hello_pi/libs/vgfont ${SDKSTAGE}/opt/vc/src/hello_pi/libs/revision)
link_directories(${SDKSTAGE}/opt/vc/lib/ ${SDKSTAGE}/opt/vc/src/hello_pi/libs/ilclient ${SDKSTAGE}/opt/vc/src/hello_pi/libs/vgfont ${SDKSTAGE}/opt/vc/src/hello_pi/libs/revision)
set(depend_lib ${depend_lib} ilclient brcmGLESv2 brcmEGL openmaxil bcm_host vcos vchiq_arm pthread rt m)

