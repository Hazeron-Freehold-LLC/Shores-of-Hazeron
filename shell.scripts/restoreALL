#!/bin/bash
# Restores the Hazeron databases from a backup.
# The current user must have database 'superuser' privileges.
DIR=/sei/Common/PSQLBackup
if [ "$1" == "" ]; then
	echo $0
	echo ' Purpose: Restores Hazeron databases from backup files in '$DIR.
	echo ' Usage: '$0' date'
	echo '  date - Backup date to restore, in the format YYYY-Mmm-DD.'
	echo ' Example: '$0' 2008-Aug-01'
elif [ ! -f $DIR/auLogin.$1 ]; then
	echo Login database backup file not found: $DIR/auLogin.$1
elif [ ! -f $DIR/auMail.$1 ]; then
	echo Mail database backup file not found: $DIR/auMail.$1
elif [ ! -f $DIR/auUniverse.$1 ]; then
	echo Universe database backup file not found: $DIR/auUniverse.$1
else
	echo `date` Restoring Hazeron databases from backup files created $1
	# Note the pg_restore format was -Ft up until Oct 2012 then failed due to a file size error.
	pg_restore -c -d auLogin -Fc /sei/Common/PSQLBackup/auLogin.$1
	pg_restore -c -d auMail -Fc /sei/Common/PSQLBackup/auMail.$1
	pg_restore -c -d auUniverse -Fc /sei/Common/PSQLBackup/auUniverse.$1
	echo `date` Finished
fi
