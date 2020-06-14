#!/usr/bin/env bash


me=`realpath $0`
my_dir=`dirname $me`
me=`basename $me`
my_parms="$@"

$my_dir/LOGIC_CORE.py $my_parms

