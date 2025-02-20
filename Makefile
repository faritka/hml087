all : flash

#Coder Id that is written on the front of a coder
#1377368, 1380873, 1385364, 1385468, 1394321
CODER_ID    = 1385468
EXTRA_CFLAGS:= -DCODER_ID=$(CODER_ID)

TARGET:=hml087
#get it from https://github.com/cnlohr/ch32v003fun
CH32V003FUN:=../ch32v003fun/ch32v003fun

include ../ch32v003fun/ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

