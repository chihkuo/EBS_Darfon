function copy() {
	echo "run function copy()"
	if [ ! -d "update" ]; then
		mkdir update
	fi
	cp ../datalog-h5000/dlg320.exe ./update/
	cp ../DataProgram/DataProgram.exe ./update/
	cp ../SWupdate/SWupdate.exe ./update/
	cp ../script/dldevice.lua ./update/
	cp ../script/dllist ./update/
	cp ../script/dllist.lua ./update/
	cp ../script/dlsetting ./update/
	cp ../script/dlsetting.lua ./update/
	cp ../script/ModelList ./update/
	cp ../script/system.lua ./update/
	cp ../script/update.sh ./update/
	sync
}

function package() {
	echo "run function package()"
	tar -cvf update_$1.tar update
	sync
}

function clean() {
	echo "run function clean()"
	rm -rf update
}

function all() {
	echo "run function all()"
	copy;
	package $1;
	clean;
}

if [ $1 == "copy" ]; then
	copy;
fi

if [ $1 == "package" ]; then
	package $2;
fi

if [ $1 == "clean" ]; then
	clean;
fi

if [ $1 == "all" ]; then
	all $2;
fi
