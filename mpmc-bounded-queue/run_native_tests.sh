TASKNAME=`basename "$PWD"`
TASKNAMEUND=`echo $TASKNAME | tr - _`


cd ../build && ../run_linter.sh $TASKNAME


echo; echo "----------------------------- SIMPLE RUN -----------------------------"
cd ../build
make test_$TASKNAMEUND
make bench_$TASKNAMEUND
./test_$TASKNAMEUND
./bench_$TASKNAMEUND

#echo; echo "----------------------------- ASAN RUN -----------------------------"
#cd ../build-Asan
#make test_$TASKNAMEUND
#make bench_$TASKNAMEUND
#./test_$TASKNAMEUND
#./bench_$TASKNAMEUND

echo; echo "----------------------------- TSAN RUN -----------------------------"
cd ../build-Tsan
make test_$TASKNAMEUND
make bench_$TASKNAMEUND
./test_$TASKNAMEUND
./bench_$TASKNAMEUND
