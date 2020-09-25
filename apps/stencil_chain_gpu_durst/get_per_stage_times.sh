grep -r "kernel_" $1  | awk '{ print substr($4,1,length($4)-2) }' > $1.nums
