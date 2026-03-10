HOST=$1
#USER="ubuntu"
#KEY=$2
CMD=$2

command=(ssh ${SSH_USERNAME}@${HOST} -i ${SSH_KEY_PATH} "${CMD}")
result=$("${command[@]}")
echo $result
