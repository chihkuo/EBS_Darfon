mkdir update

cp ../datalog-h5000/dlg320.exe ./update/
cp ../DataProgram/DataProgram.exe ./update/
cp ../script/dldevice.lua ./update/
cp ../script/dllist ./update/
cp ../script/dllist.lua ./update/
cp ../script/dlsetting ./update/
cp ../script/dlsetting.lua ./update/
cp ../script/ModelList ./update/
cp ../script/system.lua ./update/
cp ../script/update.sh ./update/

tar -cvf update_$1.tar update

sync

rm -rf update

