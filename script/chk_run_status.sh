#/bin/sh

running=`sacct | grep -iE 'running|pending' | grep -v "hpg2-dev"`; while [ "$running" != "" ]; do echo "RUNNING"; sleep 120; running=`sacct | grep -iE 'running|pending' | grep -v "hpg2-dev"`; done;
