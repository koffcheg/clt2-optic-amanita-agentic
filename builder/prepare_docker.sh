BASE_DIR=$(dirname "$0")

docker build -f $BASE_DIR/Dockerfile -t clt2-optic-build:latest $BASE_DIR

