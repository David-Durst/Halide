#! /usr/bin/awk -f

/Manually-tuned time:/ { print $3 }
/Auto-scheduled time:/ { print $3 }
