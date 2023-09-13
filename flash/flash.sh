#!/bin/bash
pydownload="/home/y/tools/semidrive/SDToolBox_R2.23.1601/App/Linux/pydownload/bin/pydownload"
ImagePath="/home/y/work/code/410_d82/sdk/buildsystem/image_X9U_merge_pac"
tmpPath="/home/y/tools/semidrive/SDToolBox_R2.23.1601/App/Linux/pydownload/bin/ImageFiles"
rePath="/home/y/work/flash/IMAGE"
version="old"

replace() {
    sleep 0.5s
    for(( i=0; i<3 ; i= ${i} +1));do
        cp ${rePath}/$1/FDA.bin ${tmpPath}/ospi*/FDA.bin
        cp ${rePath}/$1/FDA.bin ${tmpPath}/emmc*/FDA.bin
        cp ${rePath}/$1/OPSIDA.bin ${tmpPath}/ospi*/OPSIDA.bin
        cp ${rePath}/$1/OPSIDA.bin ${tmpPath}/emmc*/OPSIDA.bin
        cp ${rePath}/$1/DLOADER.bin ${tmpPath}/ospi*/DLOADER.bin
        cp ${rePath}/$1/DLOADER.bin ${tmpPath}/emmc*/DLOADER.bin
        sleep 0.3s
    done
}

while getopts "v:i:" opt;do
    case ${opt} in
    v)
        version=${OPTARG}
        ;;
    i)
        ImagePath=${OPTARG}
        ;;
    \?)
        echo "无效的选项: -$OPTARG"
        ;;
    esac
done

replace ${version} &
sudo ${pydownload} --ospi1 ${ImagePath}/ospi2.pac --emmc1 ${ImagePath}/global2.pac 