source ./vars

export SSH_KEY_PATH=${SSH_KEY_PATH}
export SSH_USERNAME=${SSH_USERNAME}

python3 ./run.py $@


# ./run_remote.sh 192.168.80.129 "bash ~/overseer/runner.sh $@"