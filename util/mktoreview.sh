#!/bin/bash

if [[ ! -f PKGLIST ]]; then
    echo "PKGLIST not found"
    exit 1
fi

rm -f TO_REVIEW
while read p
do
    c=$(grep -x $p REVIEWED)
    [[ -z $c ]] && echo $p >> TO_REVIEW
done<PKGLIST
