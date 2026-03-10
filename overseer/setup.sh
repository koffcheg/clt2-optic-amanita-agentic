export SSH_KEY_PATH=~/.ssh/cp_rsa
export SSH_USERNAME="ubuntu"

./run_remote.sh 192.168.80.129 "rm -rf ~/overseer"
./copy_dir.sh 192.168.80.129 ./overseer ~/
./run_remote.sh 192.168.80.129 "chmod +x ~/overseer/*"
