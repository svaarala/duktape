#!/bin/sh

MAINT=`pwd`/maint

rm -rf $MAINT
mkdir $MAINT

echo "FIXME" > $MAINT/FIXME
