#!/bin/bash

../../../fsutil/u6-fsutil   -n -s256000 u6.dsk
../../../fsutil/u6-fsutil -a u6.dsk lsx

../../../fsutil.lsx/lsx-util -n -s256000 lsx.dsk
../../../fsutil.lsx/lsx-util -a lsx.dsk lsx


