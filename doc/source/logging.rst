Logging
=======

Specializations of the :joedb:`Logger` class can write logs to files, but also
use various logging APIs. This page collects reference documentation of those
APIs, as well as example command lines to read those logs. The
:joedb:`System_Logger` typedef will be defined to be one of those classes
depending on the OS detected at compile time.

 - :joedb:`Android_Logger`:

   - https://developer.android.com/ndk/reference/group/logging
   - ``adb logcat -s log_tag -v color``

 - :joedb:`Apple_Logger`:

   - https://developer.apple.com/documentation/os/logger
   - https://developer.apple.com/documentation/os/1643744-os_log_create
   - https://developer.apple.com/documentation/os/os_log_with_type
   - ``log stream --level info --predicate 'category=="log_tag"'``
   - ``log stream --level info --predicate 'subsystem=="org.joedb"'``

 - :joedb:`Posix_Logger`:

   - https://man7.org/linux/man-pages/man3/syslog.3.html
   - ``sudo tail -f /var/log/syslog | grep log_tag``

 - systemd (not yet supported by joedb)

   - https://www.freedesktop.org/software/systemd/man/latest/sd_journal_send.html
   - https://www.loggly.com/ultimate-guide/linux-logging-with-systemd/
