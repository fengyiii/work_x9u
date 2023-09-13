#! /bin/bash

sudo systemctl start docker
#newgrp docker 
docker ps -a | grep tztek_docker

if [ $? == 1 ];then
    docker run --privileged -it  \
        -v /home/y/work/code/:/workspace \
        -v /tool:/tool\
	-v /dev:/dev\
	-v /home/y/tools/:/home/fengyi/tools/ \
        --name d82_ptg411 tztek_docker  /bin/bash
fi
DOCKER_ID=`docker ps -a |grep d82_ptg411 |awk '{print $1}'`

docker start ${DOCKER_ID}
docker exec -it ${DOCKER_ID} /bin/bash

#docker run --privileged -it  -v /work/user/fengyi/code/:/workspace  -v /work/user/fengyi/tools:/tool  --user 1012  --name fengyi_docker x9u_docker_image  /bin/bash
