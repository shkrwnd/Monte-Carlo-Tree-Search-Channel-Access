
obj_file=(${1//./ })
#rm obj_file
if g++ --std=c++14 $1 -o $obj_file -lcrypto ; then 
    ./$obj_file
else 
    exit
fi