Malti-Veiw is a minuture QT browser used to launch iPython Notebook.
To use Malti-View on the Raspberry Pi you must QT 5.5 from build from source.

The following script from: http://tomsik.eu/node/428 . Has been updated and tweaked
but is still a work in progress. Some links are broken. 


#!/bin/bash
#This script will download, set up, compile QT5, and set up the SDCard image ready to use.
#Pass -h to use https for git

OPT=~/opt
CC=$OPT/gcc-4.7-linaro-rpi-gnueabihf
CCT=$OPT/cross-compile-tools
MOUNT=/mnt/rasp-pi-rootfs

RASPBIAN_HTTP=http://ftp.snt.utwente.nl/pub/software/rpi/images/raspbian/2012-12-16-wheezy-raspbian/2012-12-16-wheezy-raspbian.zip
RASPBIAN_TORRENT=http://downloads.raspberrypi.org/images/raspbian/2012-12-16-wheezy-raspbian/2012-12-16-wheezy-raspbian.zip.torrent
RASPBIAN_FILE=
#RASPBIAN_FILE=esit_plus_libs4qt

CUSTOM_RASPBIAN=""

CC_GIT="gitorious.org/cross-compile-tools/cross-compile-tools.git"
QT_GIT="gitorious.org/qt/qt5.git"
GIT=GIT
INITREPOARGS="-f"

CORES=`cat /proc/cpuinfo | grep "cpu cores" -m 1 | awk '{print $4}'`
if [ ! `echo $CORES | awk '$1+0==$1'` ]; then
	CORES = 1
fi

#Parse arguments
while test $# -gt 0
do
	case $1 in
		# Normal option processing
		-h | --help)
			# usage and help
			echo "Usage:"
			echo "		./bakeqtpi.bash [options]"
			echo "Options:"
			echo "		--http			Tells git and init-repository to use http(s)"
			echo "		--httppi 		Tells the script to download the Raspbian image using http/wget"
			echo "		--torrentpi		Tells the script to download the Raspbian image using torrent/ctorrent"
			echo "		--raspbian <path>	Use custom raspbian"
			echo "		-v, --version		Version Info"
			echo "		-h, --help		Help and usage info"
			exit
			;;
		-v | --version)
			# version info
			echo "Version 0.1"
			exit
			;;
		--http)
			GIT=HTTPS
			;;
		--httppi)
			HTTPPI=1
			;;
		--torrentpi)
			TORRENT=1
			;;
		--raspbian)
			shift
			CUSTOM_RASPBIAN=$1
			;;
	
		# Special cases
		--)
			break
			;;
		--*)
			# error unknown (long) option $1
			;;
		-?)
			# error unknown (short) option $1
			;;
	
		# FUN STUFF HERE:
		# Split apart combined short options
		-*)
			split=$1
			shift
			set -- $(echo "$split" | cut -c 2- | sed 's/./-& /g') "$@"
			continue
			;;
	
		# Done with options
		*)
			break
			;;
	esac

	shift
done

echo "Using Raspbian image at $CUSTOM_RASPBIAN"

if [ "$GIT" == "HTTPS" ]; then
	CC_GIT="https://git."$CC_GIT
	QT_GIT="https://git."$QT_GIT
	INITREPOARGS="$INITREPOARGS --http"
else
	CC_GIT="git://"$CC_GIT
	QT_GIT="git://"$QT_GIT
fi


function error {
	case "$1" in

		1) echo "Error making directories"
		;;
		2) echo "Error downloading raspbian image"
		;;
		3) echo "Error mounting raspbian image"
		;;
		4) echo "Error downloading cross compilation tools"
		;;
		5) echo "Error extracting cross compilation tools"
		;;
		6) echo "Error cloning qt5 repo"
		;;
		7) echo "Error initialising qt5 repo"
		;;
		8) echo "Error running fixQualifiedLibraryPaths"
		;;
		9) echo "Configuring QT Failed"
		;;
		10) echo "Make failed for QTBase"
		;;
		*) echo "Unknown error"
		;;

	esac
	exit -1
}

function downloadAndMountPi {
	cd $OPT

		RASPBIAN_IMG=$RASPBIAN_FILE.img

	if [ ! -d $MOUNT ]; then
		sudo mkdir $MOUNT || error 3
	else
		sudo umount $MOUNT
	fi
	sudo mount -o loop,offset=62914560 $RASPBIAN_IMG $MOUNT || error 3
}

#Download and extract cross compiler and tools
function dlcc {
	cd $OPT
	#wget https://www.dropbox.com/s/sl919ly0q79m1e6/gcc-4.7-linaro-rpi-gnueabihf.tbz || error 4
	#tar -xf gcc-4.7-linaro-rpi-gnueabihf.tbz || error 5
	#if [ ! -d $CCT/.git ]; then
	#	git clone https://gitorious.org/cross-compile-tools/cross-compile-tools.git || error 4
	#else
	#	cd $CCT && git pull && cd $OPT
	#fi
}

function dlqt {
	cd $OPT
	if [ ! -d $OPT/qt5/.git ]; then
		git clone git://code.qt.io/qt/qt5.git || error 6
	else
		cd $OPT/qt5/ && git pull && cd ..
	fi
	cd qt5
	while [ ! -e $OPT/qt5/.initialised ]
	do
		./init-repository $INITREPOARGS && touch $OPT/qt5/.initialised
	done || error 7
	#cd $OPT/qt5/qtjsbackend
	#git fetch https://codereview.qt-project.org/p/qt/qtjsbackend refs/changes/56/27256/4 && git cherry-pick FETCH_HEAD
}

function prepcctools {
	cd $CCT
	./fixQualifiedLibraryPaths $MOUNT ~/opt/gcc-4.7-linaro-rpi-gnueabihf/bin/arm-linux-gnueabihf-gcc || error 8
	cd $OPT/qt5/qtbase
}

function configureandmakeqtbase {
	cd $OPT/qt5/qtbase
	if [ ! -e $OPT/qt5/qtbase/.CONFIGURED ]; then
		./configure -opengl es2 -tslib -device linux-rasp-pi-g++ -device-option CROSS_COMPILE=$CC/bin/arm-linux-gnueabihf- -sysroot $MOUNT -opensource -confirm-license -optimized-qmake -reduce-exports -release -make libs -prefix /usr/local/qt5pi -nomake examples -nomake tests -no-pch -qt-xcb && touch $OPT/qt5/qtbase/.CONFIGURED || error 9
	fi
	CORES=`cat /proc/cpuinfo | grep "cpu cores" -m 1 | awk '{print $4}'`
	make -j $CORES || error 10
}

function installqtbase {
	cd $OPT/qt5/qtbase
	sudo make install
}

function makemodules {
	for i in qtwebkit qtimageformats qtsvg qtscript qtxmlpatterns qtdeclarative qtsensors qt3d qtgraphicaleffects qtlocation qtquick1 qtsystems qtmultimedia
	do
		cd $OPT/qt5/$i && echo "Building $i" && sleep 3 && /usr/local/qt5pi/bin/qmake . && make -j $CORES && sudo make install && touch .COMPILED
		cd $OPT/qt5/
	done
	
	for i in qtwebkit qtimageformats qtsvg qtscript qtxmlpatterns qtdeclarative qtsensors qt3d qtgraphicaleffects qtlocation qtquick1 qtsystems qtmultimedia
	do
		if [ -e "$OPT/qt5/$i/.COMPILED" ]
		then
			echo "Compiled $i"
		else
			echo "Failed   $i"
		fi
	done
}
#Start of script

mkdir -p $OPT || error 1
cd $OPT || error 1

downloadAndMountPi
dlcc
dlqt
prepcctools
configureandmakeqtbase
installqtbase
makemodules
