#!/usr/bin/env bash


me=`realpath $0`
my_dir=`dirname $me`

export LD_LIBRARY_PATH="$my_dir/../HVAC_LIB/bin:$LD_LIBRARY_PATH"
cd $my_dir
$my_dir/bin/LOGIC_CORE $@
