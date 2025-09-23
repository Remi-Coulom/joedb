Logging
=======

 - Android:

   - https://developer.android.com/ndk/reference/group/logging
   - ``adb logcat -s log_tag -v color``

 - MacOS:

   - https://developer.apple.com/documentation/os/logger
   - https://developer.apple.com/documentation/os/1643744-os_log_create
   - https://developer.apple.com/documentation/os/os_log_with_type
   - ``log stream --level info --predicate 'category=="log_tag"'``
   - ``log stream --level info --predicate 'subsystem=="org.joedb"'``

 - Posix syslog:

   - https://man7.org/linux/man-pages/man3/syslog.3.html
   - ``sudo tail -f /var/log/syslog | grep log_tag``

 - systemd

   - https://www.freedesktop.org/software/systemd/man/latest/sd_journal_send.html
   - https://www.loggly.com/ultimate-guide/linux-logging-with-systemd/
