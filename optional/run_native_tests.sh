TASKNAME=`basename "$PWD"`
TASKNAMEUND=`echo $TASKNAME | tr - _`

cd ../build-asan
../run_linter.sh $TASKNAME
make test_$TASKNAMEUND
./test_$TASKNAMEUND
