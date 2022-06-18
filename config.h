/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* Path to the chroot /var/named directory */
#define CHROOT_PATH "/var/named"

/* Seconds to wait from starting named until the log tab is updated */
#define CMD_SLEEP_LOG_SECS "3"

/* Path to the dig binary */
#define DIG_BINARY "dig"

/* always defined to indicate that i18n is enabled */
#define ENABLE_NLS 1

/* needed for gettext... */
#define GETTEXT_PACKAGE "gadmin-bind"

/* The groupadd binary */
#define GROUPADD_BINARY "groupadd"

/* Path to group */
#define GROUP_FILE "/etc/group"

/* Path to gshadow */
#define GSHADOW_FILE "/etc/gshadow"

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
#define HAVE_BIND_TEXTDOMAIN_CODESET 1

/* Define to 1 if you have the `dcgettext' function. */
#define HAVE_DCGETTEXT 1

/* Define if the GNU gettext() function is already present or preinstalled. */
#define HAVE_GETTEXT 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define if your <locale.h> file defines LC_MESSAGES. */
#define HAVE_LC_MESSAGES 1

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* localstatedir */
#define LOCALSTATEDIR "/var"

/* Path to the named binary */
#define NAMED_BINARY "named"

/* The user which named runs as */
#define NAMED_USER "bind"

/* Name of package */
#define PACKAGE "gadmin-bind"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Path to passwd */
#define PASSWD_FILE "/etc/passwd"

/* Path to the proc filesystem */
#define PROC_PATH "/proc"

/* Path to resolv.conf */
#define RESOLV_PATH "/etc/resolv.conf"

/* Path to the rndc binary */
#define RNDC_BINARY "rndc"

/* Path to rndc.conf */
#define RNDC_CONF "/etc/bind/rndc.conf"

/* Configuration file for bind */
#define BIND_CONF "/etc/bind/named.conf"

/* Path to the rndc-confgen binary */
#define RNDC_CONFGEN_BINARY "rndc-confgen"

/* Path to the sed binary */
#define SED_BINARY "sed"

/* Path to settings directory etc/gadmin-bind */
#define SETTINGS_DIR "/etc/gadmin-bind"

/* Path to shadow */
#define SHADOW_FILE "/etc/shadow"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* sysconfdir */
#define SYSCONFDIR "/etc"

/* Path to the sysinit scripts directory */
#define SYSINIT_SCRIPTS_DIR "/etc/init.d"

/* Name of the sysinit script */
#define SYSINIT_SCRIPT_NAME "gadmin-bind"

/* Command for starting the server at boot */
#define SYSINIT_START_CMD "update-rc.d -f gadmin-bind defaults"

/* Command for not starting the server at boot */
#define SYSINIT_STOP_CMD "update-rc.d -f gadmin-bind remove"

/* Path to syslog messages */
#define SYSLOG_PATH "/var/log/messages"

/* The useradd binary */
#define USERADD_BINARY "useradd"

/* using aix macros... */
/* #undef USE_AIX */

/* using darwin macros... */
/* #undef USE_DARWIN */

/* using freebsd macros... */
/* #undef USE_FREEBSD */

/* using hpux macros... */
/* #undef USE_HPUX */

/* using Linux macros... */
#define USE_LINUX 1

/* using netbsd macros... */
/* #undef USE_NETBSD */

/* using openbsd macros... */
/* #undef USE_OPENBSD */

/* using sunos macros... */
/* #undef USE_SUNOS */

/* Version number of package */
#define VERSION "0.2.5"
