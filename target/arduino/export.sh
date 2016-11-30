#!/bin/sh

SRC=$(cd "$(dirname "$0")"; pwd)

usage() {
	echo "$0 [--link] TARGET_DIR"
	echo "Create an Arduino-compatible library in the given directory, overwriting existing files. If --link is given, creates symbolic links for easy testing."
}


CMD="cp -r -v"
case "$1" in
	--help)
		usage
		exit 0
	;;
	--link)
		CMD="ln -s -v"
		shift;
	;;
	--*)
		echo "Unknown option: $1" 1>&2
		exit 1
	;;
esac

TARGET=$1

if [ -z "$TARGET" ]; then
	usage
	exit 1
fi

if ! [ -d "$(dirname "$TARGET")" ]; then
	echo "Parent of $TARGET should exist" 1>&2
	exit 1
fi

mkdir -p "$TARGET"/src

$CMD "$SRC"/library.properties "$TARGET"
$CMD "$SRC"/lmic.h "$TARGET"/src
$CMD "$SRC"/../../lmic "$TARGET"/src
$CMD "$SRC"/../../aes "$TARGET"/src
$CMD "$SRC"/hal "$TARGET"/src
$CMD "$SRC"/examples "$TARGET"
