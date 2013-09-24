#!/bin/sh
#
#  Resize touch icon starting from touch_icon_1024x1024.png.
#

for sz in 57 60 72 114 120 144 152; do
	convert touch_icon_1024x1024.png -resize ${sz}x${sz} touch_icon_${sz}x${sz}.png
	file touch_icon_${sz}x${sz}.png
done

