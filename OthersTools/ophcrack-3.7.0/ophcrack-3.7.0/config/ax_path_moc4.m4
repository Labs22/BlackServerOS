##### 
#
# SYNOPSIS
#
#   AX_PATH_MOC4
#
# DESCRIPTION
#
#   Check for Qt4 version of moc.
#
#   $MOC4 is set to absolute name of the executable if found.
#
# LAST MODIFICATION
#
#   2008-03-31
#
# COPYLEFT
#
#   Copyright (c) 2007 YAMAMOTO Kengo <yamaken AT bp.iij4u.or.jp>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.

AC_DEFUN([AX_PATH_MOC4], [
  ax_guessed_qt4_dirs="/usr/lib/qt4/bin:/usr/local/lib/qt4/bin:/usr/qt4/bin:/usr/local/qt4/bin:${QT4DIR}/bin:${QTDIR}/bin"
  AC_PROG_EGREP
  AC_PATH_PROGS(_MOC4, [moc-qt4 moc4], [], ["$with_qt/bin:$PATH:$ax_guessed_qt4_dirs"])
  AC_PATH_PROGS(_MOC, [moc], [], ["$with_qt/bin:$PATH:$ax_guessed_qt4_dirs"])

  AC_CACHE_CHECK([for Qt4 version of moc], ax_cv_path_MOC4, [
    ax_cv_path_MOC4=no
    for moc4 in ${_MOC4} ${_MOC}; do
      if ($moc4 -v 2>&1 | $EGREP -q 'Qt 4'); then
        ax_cv_path_MOC4="$moc4"
	      break
      fi
    done])


  if test "x$ax_cv_path_MOC4" = "xno"; then
    AC_MSG_ERROR([
		  ophcrack requires Qt toolkit version 4 or later.
		  Please disable the GUI via '--disable-gui',
		  or see http://www.trolltech.com/ to obtain it.])
    else
      MOC4="$ax_cv_path_MOC4"
      AC_SUBST([MOC4])
    fi])
