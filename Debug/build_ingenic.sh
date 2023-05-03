rm -rf libmp4recorder.a
export CC=/opt/mips-gcc472-glibc216-64bit/bin/mips-linux-uclibc-gnu-gcc
make clean
make all
ar rcs libmp4recorder.a mp4recorder.o g711.o
cp libmp4recorder.a /home/phongdv/eclipse-workspace/camerafirmware-v2-Ingenic-S3/libs/
