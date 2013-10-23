#!/bin/sh
PND_SCRIPTS_DIR=/usr/local/angstrom/arm/scripts
PND_TMP_DIR=tmp
PND_DIST_FILE=mooboy.pnd
PND_MAKE=$PND_SCRIPTS_DIR/pnd_make

rm -fr $PND_TMP_DIR/

mkdir $PND_TMP_DIR
mkdir $PND_TMP_DIR/data

cp mooboy $PND_TMP_DIR/
cp ../data/Xolonium.ttf $PND_TMP_DIR/data/
cp ../data/back.png $PND_TMP_DIR/data/
cp ../icon.png $PND_TMP_DIR/
cp PXML.xml $PND_TMP_DIR/
cp mooboy.sh $PND_TMP_DIR/
cp -r preview $PND_TMP_DIR/

$PND_MAKE -p mooboy.mrz.pnd -d $PND_TMP_DIR -x $PND_TMP_DIR/PXML.xml -i $PND_TMP_DIR/icon.png

#rm -fr $PND_TMP_DIR/
