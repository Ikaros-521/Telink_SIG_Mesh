#########################################################################################
# As a new project, you should  the following variables		
#########################################################################################

# Select Chip  
TEL_CHIP := -DCHIP_TYPE=CHIP_TYPE_8258 -D__PROJECT_MESH__=1

# Select link lib
LIBS := -llt_8258_mesh -lsoft-fp -lsig_mesh_825x

##########################################################################################
#Set SDK Path , if you copy this project other dir, please set TEL_PATH absolute path
#设置 SDK Path，目前使用的是相对路径，如果你将此工程复制到别的文件夹，请将TEL_PATH设为绝对路径
# 比如�?TEL_PATH := /home/ospanic/Telink_825X_SDK
#########################################################################################
TEL_PATH := ../..

# Set Project Name
PROJECT_NAME := at_sig_mesh

# 设置下载固件的串口号
DOWNLOAD_PORT := COM96

#########################################################################################
# The following content need not be modified by the user!!!!!							#
#########################################################################################
PROJECT_PATH := .
OUT_PATH :=$(PROJECT_PATH)/out

ifneq ($(TEL_PATH)/make/makefile, $(wildcard $(TEL_PATH)/make/makefile))
$(error "Please check if the TEL_PATH is correct, ���� TEL_PATH �Ƿ�������ȷ������")
endif

#include the SDK makefile
-include $(TEL_PATH)/make/makefile