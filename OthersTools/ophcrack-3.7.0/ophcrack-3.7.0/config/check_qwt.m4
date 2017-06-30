#
# SYNOPSIS
#
#   CHECK_QWT
#
# DESCRIPTION
#
#   This macro will check various standard spots for Qwt including
#   a user-supplied directory. The user uses '--with-qwt' or
#   '--with-qwt=/path/to/qwt' as arguments to configure.
#
# LAST MODIFICATION
#
#   2009-05-27
#
# COPYLEFT
#
#   Copyright (c) 2009 Cedric Tissieres 
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.

AC_DEFUN([CHECK_QWT],
[
    for dir in $withval /usr/local/qwt /usr/local /usr; do
        qwtdir="$dir"
        if test -f "$dir/include/qwt-qt4/qwt.h"; then
            found_qwt="yes";
            QWT_INCLUDES="-I$qwtdir/include/qwt-qt4";
	          QWT_LIBS="-lqwt-qt4";
            break;
        fi
        if test -f "$dir/include/qwt/qwt.h"; then
            found_qwt="yes";
            QWT_INCLUDES="-I$qwtdir/include/qwt";
	          QWT_LIBS="-lqwt";
            break;
        fi
        if test -f "$dir/include/qwt.h"; then
            found_qwt="yes";
            QWT_INCLUDES="-I$qwtdir/include";
            QWT_LIBS="-lqwt";
            break;
        fi
    done
    if test x_$found_qwt != x_yes; then
    	AM_CONDITIONAL(HAVE_QWT, false)
	    AC_DEFINE_UNQUOTED([HAVE_QWT], 0, [QWT is disabled])
      AC_MSG_RESULT(no)
	    enable_graph="no";
    else
      printf "Qwt found in $qwtdir\n";
      QWT_LDFLAGS="-L$qwtdir/lib";
	    AC_DEFINE_UNQUOTED([HAVE_QWT], 1, [QWT is enabled])
	    AM_CONDITIONAL(HAVE_QWT, true)
      AC_SUBST([QWT_INCLUDES])
      AC_SUBST([QWT_LIBS])
      AC_SUBST([QWT_LDFLAGS])
    fi
])
