rm -rf libmp4recorder.a
export CC=/opt/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-gcc
make clean
make all
ar rcs libmp4recorder.a mp4recorder.o g711.o
cp libmp4recorder.a /home/phongdv/eclipse-workspace/camerafirmware-v2-S3/libs/
