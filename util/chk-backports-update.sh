#!/bin/bash
#
# Copyright (C) 2016
#
# Authors:
#   Jason Graham <jgraha8@gmail.com>
#
# Simple script to check packages in the BACKPORTS list file for updates 
# between the backports git tree and the upstream git tree.
#

function cleanup()
{
    rm -f .updated
}

if (( $# != 3 )); then
    echo "Usage: `basename $0` backports-file backports-git-tree upstream-git-tree"
    exit 1
fi

backports_file=$1
backports_git_tree=$2
upstream_git_tree=$3

echo $backports_git_tree $upstream_git_tree

trap "cleanup; exit 1" SIGINT SIGTERM

while read p
do
    for d in `find $backports_git_tree -type d -name $p`
    do
	group_path=${d%/*}
	group=${group_path##*/}
	p_info="${group}/${p}/${p}.info"

	diff -u "${backports_git_tree}/${p_info}" "${upstream_git_tree}/${p_info}" > .updated

	if [[ -s .updated ]]; then
	    version_old=$(grep -w VERSION .updated | head -n 1 | cut -d= -f2 | sed 's/"//g')
	    version_new=$(grep -w VERSION .updated | tail -n 1 | cut -d= -f2 | sed 's/"//g')
	    if [[ $version_old < $version_new ]]; then
		echo "${group}/${p} updated: ${version_old} --> ${version_new}"
	    fi
	fi
    done
done<${backports_file}

cleanup
