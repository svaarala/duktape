#!/bin/sh
#
# http://naleid.com/blog/2012/01/17/finding-and-purging-big-files-from-git-history

rm -f /tmp/duk-all-file-shas.txt /tmp/duk-all-files.txt \
      /tmp/duk-big-objects.txt /tmp/duk-big-to-small.txt

git rev-list --objects --all | sort -k 2 > /tmp/duk-all-file-shas.txt

git rev-list --objects --all | sort -k 2 | cut -f 2 -d\  | uniq > /tmp/duk-all-files.txt

git gc && git verify-pack -v .git/objects/pack/pack-*.idx | egrep "^\w+ blob\W+[0-9]+ [0-9]+ [0-9]+$" | sort -k 3 -n -r > /tmp/duk-big-objects.txt

for SHA in `cut -f 1 -d\  < /tmp/duk-big-objects.txt`; do
	echo $(grep $SHA /tmp/duk-big-objects.txt) $(grep $SHA /tmp/duk-all-file-shas.txt) | awk '{print $1,$3,$7}' >> /tmp/duk-big-to-small.txt
done

echo "Result: cat /tmp/duk-big-to-small.txt"
