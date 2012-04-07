#!/bin/bash
[ -n "$1" ] && cd $1
git rev-list HEAD | sort > config.git-hash
LOCALVER=`wc -l config.git-hash | awk '{print $1}'`
if [ $LOCALVER \> 1 ] ; then
    VER=`git rev-list origin/master | sort | join config.git-hash - | wc -l | awk '{print $1}'`
    VER_DIFF=$(($LOCALVER-$VER))
    echo "#define X265_REV $VER"
    echo "#define X265_REV_DIFF $VER_DIFF"
    if [ $VER_DIFF != 0 ] ; then
        VER="$VER+$VER_DIFF"
    fi
    if git status | grep -q "modified:" ; then
        VER="${VER}M"
    fi
    VER="$VER $(git rev-list HEAD -n 1 | cut -c 1-7)"
    echo "#define X265_VERSION \"r$VER\""
else
    echo "#define X265_VERSION \"\""
    VER="x"
fi
rm -f config.git-hash
