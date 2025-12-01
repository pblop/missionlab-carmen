#
# Regular cron jobs for the missionlabcarmen package
#
0 4	* * *	root	[ -x /usr/bin/missionlabcarmen_maintenance ] && /usr/bin/missionlabcarmen_maintenance
