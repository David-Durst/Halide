#! /usr/bin/awk -f

/kernel_/ && $0 !~ /GPU activities/ { print $2 }
/kernel_/ && /GPU activities/ { print $4 }
