export DB_HOST=192.168.0.112
export DB_PORT=6033
export DB_LOGIN=stud
export DB_PASSWORD=stud
export DB_DATABASE=archdb

export AUTH_SERVICE_PORT=8081
export USER_SERVICE_PORT=8082
export PRODUCT_SERVICE_PORT=8083

export JWT_KEY=bigsecretkey

export MAX_SHARD=1
export SEQUENCE_SHARD=2

export CREATE_TEST_USERS=1

cd ./build
make auth_service
./auth_service 
# ./user_service &