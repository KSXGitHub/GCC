#### This script is meant to be sourced by ltconfig.

# ltcf-cxx.sh - Create a C++ compiler specific configuration
#
# Copyright (C) 1996-1999, 2000, 2001, 2003 Free Software Foundation, Inc.
# Originally by Gordon Matzigkeit <gord@gnu.ai.mit.edu>, 1996
#
# Original C++ support by:Gary V. Vaughan <gvv@techie.com>
#    Alexandre Oliva <oliva@lsd.ic.unicamp.br>
#    Ossama Othman <ossama@debian.org>
#    Thomas Thanner <tanner@gmx.de>
#
# This file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# As a special exception to the GNU General Public License, if you
# distribute this file as part of a program that contains a
# configuration script generated by Autoconf, you may include it under
# the same distribution terms that you use for the rest of that program.


# Source file extension for C++ test sources.
ac_ext=cc

# Object file extension for compiled C++ test sources.
objext=o

# Code to be used in simple compile tests
lt_simple_compile_test_code="int some_variable = 0;"

# Code to be used in simple link tests
lt_simple_link_test_code='int main(int, char *[]) { return (0); }'

# C++ compiler
CXX=${CXX-c++}

# ltmain only uses $CC for tagged configurations so make sure $CC is set.
CC=${CC-"$CXX"}
CFLAGS=${CFLAGS-"$CXXFLAGS"}

# Allow CC to be a program name with arguments.
set dummy $CC
compiler=$2
cc_basename=`$echo X"$compiler" | $Xsed -e 's%^.*/%%'`

# Check if we are using GNU gcc  (taken/adapted from configure script)
# We need to check here since "--with-gcc" is set at configure time,
# not ltconfig time!
cat > conftest.$ac_ext <<EOF
#ifdef __GNUC__
  yes;
#endif
EOF
if { ac_try='${CC-c++} -E conftest.$ac_ext'; { (eval echo \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }; } | egrep yes >/dev/null 2>&1; then
  with_gcc=yes

  # Set up default GNU C++ configuration

  # Check if GNU C++ uses GNU ld as the underlying linker, since the
  # archiving commands below assume that GNU ld is being used.
  if eval "`$CC -print-prog-name=ld` --version 2>&1" | \
      egrep 'GNU ld' > /dev/null; then
    with_gnu_ld=yes

    archive_cmds='$CC -shared -nostdlib $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-soname $wl$soname -o $lib'
    archive_expsym_cmds='$CC -shared -nostdlib $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-soname $wl$soname ${wl}-retain-symbols-file $wl$export_symbols -o $lib'

    hardcode_libdir_flag_spec='${wl}--rpath ${wl}$libdir'
    export_dynamic_flag_spec='${wl}--export-dynamic'

    # If archive_cmds runs LD, not CC, wlarc should be empty
    # XXX I think wlarc can be eliminated in ltcf-cxx, but I need to
    #     investigate it a little bit more. (MM)
    wlarc='${wl}'

    # ancient GNU ld didn't support --whole-archive et. al.
    if eval "`$CC -print-prog-name=ld` --help 2>&1" | \
        egrep 'no-whole-archive' > /dev/null; then
      whole_archive_flag_spec="$wlarc"'--whole-archive$convenience '"$wlarc"'--no-whole-archive'
    else
      whole_archive_flag_spec=
    fi
  else
    with_gnu_ld=no
    wlarc=

    # A generic and very simple default shared library creation
    # command for GNU C++ for the case where it uses the native
    # linker, instead of GNU ld.  If possible, this setting should
    # overridden to take advantage of the native linker features on
    # the platform it is being used on.
    archive_cmds='$CC -shared $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags -o $lib'
  fi

  # Commands to make compiler produce verbose output that lists
  # what "hidden" libraries, object files and flags are used when
  # linking a shared library.
  output_verbose_link_cmd='$CC -shared $CFLAGS -v conftest.$objext 2>&1 | egrep "\-L"'

else
  with_gcc=no
  with_gnu_ld=no
  wlarc=
fi

# PORTME: fill in a description of your system's C++ link characteristics
case $host_os in
  aix3*)
    # FIXME: insert proper C++ library support
    ld_shlibs=no
    ;;
  aix4* | aix5*)
    archive_cmds=''
    hardcode_direct=yes
    hardcode_libdir_separator=':'
    link_all_deplibs=yes
    # When large executables or shared objects are built, AIX ld can
    # have problems creating the table of contents.  If linking a library
    # or program results in "error TOC overflow" add -mminimal-toc to
    # CXXFLAGS/CFLAGS for g++/gcc.  In the cases where that is not
    # enough to fix the problem, add -Wl,-bbigtoc to LDFLAGS.
    if test "$with_gcc" = yes; then
      case $host_os in aix4.[012]|aix4.[012].*)
      # We only want to do this on AIX 4.2 and lower, the check
      # below for broken collect2 doesn't work under 4.3+
        collect2name=`${CC} -print-prog-name=collect2`
        if test -f "$collect2name" && \
	   strings "$collect2name" | grep resolve_lib_name >/dev/null
        then
	  # We have reworked collect2
	  hardcode_direct=yes
        else
	  # We have old collect2
	  hardcode_direct=unsupported
	  # It fails to find uninstalled libraries when the uninstalled
	  # path is not listed in the libpath.  Setting hardcode_minus_L
	  # to unsupported forces relinking
	  hardcode_minus_L=yes
	  hardcode_libdir_flag_spec='-L$libdir'
	  hardcode_libdir_separator=
        fi
      esac
      shared_flag='-shared'
    else
      # not using gcc
      if test "$host_cpu" = ia64; then
        shared_flag='${wl}-G'
      else
        shared_flag='${wl}-bM:SRE'
      fi
    fi

    if test "$host_cpu" = ia64; then
      # On IA64, the linker does run time linking by default, so we don't
      # have to do anything special.
      aix_use_runtimelinking=no
      if test $with_gnu_ld = no; then
        exp_sym_flag='-Bexport'
        no_entry_flag=""
      fi
    else
      # Test if we are trying to use run time linking, or normal AIX style linking.
      # If -brtl is somewhere in LDFLAGS, we need to do run time linking.
      aix_use_runtimelinking=no
      for ld_flag in $LDFLAGS; do
        if (test $ld_flag = "-brtl" || test $ld_flag = "-Wl,-brtl" ); then
          aix_use_runtimelinking=yes
          break
        fi
      done
      exp_sym_flag='-bexport'
      no_entry_flag='-bnoentry'
    fi
    # It seems that -bexpall does not export symbols beginning with
    # underscore (_), so it is better to generate a list of symbols to export.
    always_export_symbols=yes
    if test "$aix_use_runtimelinking" = yes; then
      hardcode_libdir_flag_spec='${wl}-blibpath:$libdir:/usr/lib:/lib'
      allow_undefined_flag=' -Wl,-G'
      archive_expsym_cmds="\$CC $shared_flag"' -o $output_objdir/$soname $libobjs $deplibs $compiler_flags ${allow_undefined_flag} '"\${wl}$no_entry_flag \${wl}-brtl \${wl}$exp_sym_flag:\$export_symbols"
     else
      if test "$host_cpu" = ia64; then
        if test $with_gnu_ld = no; then
          hardcode_libdir_flag_spec='${wl}-R $libdir:/usr/lib:/lib'
          allow_undefined_flag="-z nodefs"
          archive_expsym_cmds="\$CC $shared_flag"' -o $output_objdir/$soname $libobjs $deplibs $compiler_flags ${wl}${allow_undefined_flag} '"\${wl}$no_entry_flag \${wl}$exp_sym_flag:\$export_symbols"
        fi
      else
        hardcode_libdir_flag_spec='${wl}-blibpath:$libdir:/usr/lib:/lib'
        # Warning - without using the other run time loading flags, -berok will
        #           link without error, but may produce a broken library.
        no_undefined_flag=' ${wl}-bnoerok'
        allow_undefined_flag=' ${wl}-berok'
        # -bexpall does not export symbols beginning with underscore (_)
        always_export_symbols=yes
        # Exported symbols can be pulled into shared objects from archives
        whole_archive_flag_spec=' '
        build_libtool_need_lc=yes
        # This is similar to how AIX traditionally builds it's shared libraries.
        archive_expsym_cmds="\$CC $shared_flag"' -o $output_objdir/$soname $libobjs $deplibs $compiler_flags ${wl}-bE:$export_symbols ${wl}-bnoentry${allow_undefined_flag}~$AR $AR_FLAGS $output_objdir/$libname$release.a $output_objdir/$soname'
      fi
    fi
    ;;
  chorus*)
    case $cc_basename in
      *)
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
    esac
    ;;
  dgux*)
    case $cc_basename in
      ec++)
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
      ghcx)
        # Green Hills C++ Compiler
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
      *)
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
    esac
    ;;
  freebsd[12]*)
    # C++ shared libraries reported to be fairly broken before switch to ELF
    ld_shlibs=no
    ;;
  freebsd*)
    # FreeBSD 3 and later use GNU C++ and GNU ld with standard ELF
    # conventions
    ld_shlibs=yes
    ;;
  gnu*)
    ;;
  hpux*)
    if test $with_gnu_ld = no; then
      case "$host_cpu" in
	ia64*)
	  hardcode_libdir_flag_spec='-L$libdir'
	  hardcode_shlibpath_var=no ;;
	*)
	  hardcode_libdir_flag_spec='${wl}+b ${wl}$libdir' ;;
      esac
      hardcode_direct=yes
      hardcode_libdir_separator=:
      export_dynamic_flag_spec='${wl}-E'
    fi
    hardcode_minus_L=yes # Not in the search PATH, but as the default
			 # location of the library.

    case $cc_basename in
      CC)
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
      aCC)
	case $host_os in
	hpux9*) archive_cmds='$rm $output_objdir/$soname~$CC -b ${wl}+b ${wl}$install_libdir -o $output_objdir/$soname $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags~test $output_objdir/$soname = $lib || mv $output_objdir/$soname $lib' ;;
	*) archive_cmds='$CC -b ${wl}+h ${wl}$soname ${wl}+b ${wl}$install_libdir -o $lib $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags' ;;
	esac
        # Commands to make compiler produce verbose output that lists
        # what "hidden" libraries, object files and flags are used when
        # linking a shared library.
        #
        # There doesn't appear to be a way to prevent this compiler from
        # explicitly linking system object files so we need to strip them
        # from the output so that they don't get included in the library
        # dependencies.
        output_verbose_link_cmd='templist=`($CC -b $CFLAGS -v conftest.$objext 2>&1) | egrep "\-L"`; list=""; for z in $templist; do case $z in conftest.$objext) list="$list $z";; *.$objext);; *) list="$list $z";;esac; done; echo $list'
        ;;
      *)
        if test $with_gcc = yes; then
	  if test $with_gnu_ld = no; then
	    case "$host_os" in
	    hpux9*) archive_cmds='$rm $output_objdir/$soname~$CC -shared -nostdlib -fPIC ${wl}+b ${wl}$install_libdir -o $output_objdir/$soname $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags~test $output_objdir/$soname = $lib || mv $output_objdir/$soname $lib' ;;
	    *)
	      case "$host_cpu" in
		ia64*)
		  archive_cmds='$LD -b +h $soname -o $lib $predep_objects $libobjs $deplibs $postdep_objects $linker_flags' ;;
		*)
		  archive_cmds='$CC -shared -nostdlib -fPIC ${wl}+h ${wl}$soname ${wl}+b ${wl}$install_libdir -o $lib $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags' ;;
	      esac
	      ;;
	    esac
	  fi
	else
	  # FIXME: insert proper C++ library support
          ld_shlibs=no
	fi
        ;;
    esac
    ;;
  irix5* | irix6*)
    case $cc_basename in
      CC)
        # SGI C++
        archive_cmds='$CC -shared -all -multigot $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags -soname $soname `test -n "$verstring" && echo -set_version $verstring` -update_registry ${objdir}/so_locations -o $lib'

	# Archives containing C++ object files must be created using
	# "CC -ar", where "CC" is the IRIX C++ compiler.  This is
	# necessary to make sure instantiated templates are included
	# in the archive.
	old_archive_cmds='$CC -ar -WR,-u -o $oldlib $oldobjs'
	;;
      *)
        if test "$with_gcc" = yes; then
          if test "$with_gnu_ld" = no; then
            archive_cmds='$CC -shared -nostdlib $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-soname ${wl}$soname `test -n "$verstring" && echo ${wl}-set_version ${wl}$verstring` ${wl}-update_registry ${wl}${objdir}/so_locations -o $lib'
          else
            archive_cmds='$CC -shared -nostdlib $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags -soname $soname `test -n "$verstring" && echo -set_version $verstring` -o $lib'
          fi
        fi
        hardcode_libdir_flag_spec='${wl}-rpath ${wl}$libdir'
        hardcode_libdir_separator=:
        link_all_deplibs=yes
        ;;
    esac
    ;;
  linux*)
    case $cc_basename in
      KCC)
        # Kuck and Associates, Inc. (KAI) C++ Compiler

        # KCC will only create a shared library if the output file
        # ends with ".so" (or ".sl" for HP-UX), so rename the library
        # to its proper name (with version) after linking.
        archive_cmds='templib=`echo $lib | sed -e "s/\.so\..*/\.so/"`; $CC $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags --soname $soname -o \$templib; mv \$templib $lib'
        archive_expsym_cmds='templib=`echo $lib | sed -e "s/\.so\..*/\.so/"`; $CC $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags --soname $soname -o \$templib ${wl}-retain-symbols-file,$export_symbols; mv \$templib $lib'

        # Commands to make compiler produce verbose output that lists
        # what "hidden" libraries, object files and flags are used when
        # linking a shared library.
        #
        # There doesn't appear to be a way to prevent this compiler from
        # explicitly linking system object files so we need to strip them
        # from the output so that they don't get included in the library
        # dependencies.
        output_verbose_link_cmd='templist=`$CC $CFLAGS -v conftest.$objext -o libconftest.so 2>&1 | egrep "ld"`; rm -f libconftest.so; list=""; for z in $templist; do case $z in conftest.$objext) list="$list $z";; *.$objext);; *) list="$list $z";;esac; done; echo $list'

        hardcode_libdir_flag_spec='${wl}--rpath,$libdir'
        export_dynamic_flag_spec='${wl}--export-dynamic'

	# Archives containing C++ object files must be created using
	# "CC -Bstatic", where "CC" is the KAI C++ compiler.
        old_archive_cmds='$CC -Bstatic -o $oldlib $oldobjs'
        ;;
      cxx)
        # Compaq C++
        archive_cmds='$CC -shared $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-soname $wl$soname -o $lib'
        archive_expsym_cmds='$CC -shared $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-soname $wl$soname  -o $lib ${wl}-retain-symbols-file $wl$export_symbols'

        runpath_var=LD_RUN_PATH
        hardcode_libdir_flag_spec='-rpath $libdir'
        hardcode_libdir_separator=:

        # Commands to make compiler produce verbose output that lists
        # what "hidden" libraries, object files and flags are used when
        # linking a shared library.
        #
        # There doesn't appear to be a way to prevent this compiler from
        # explicitly linking system object files so we need to strip them
        # from the output so that they don't get included in the library
        # dependencies.
        output_verbose_link_cmd='templist=`$CC -shared $CFLAGS -v conftest.$objext 2>&1 | grep "ld"`; templist=`echo $templist | sed "s/\(^.*ld.*\)\( .*ld .*$\)/\1/"`; list=""; for z in $templist; do case $z in conftest.$objext) list="$list $z";; *.$objext);; *) list="$list $z";;esac; done; echo $list'
        ;;
    esac
    ;;
  lynxos*)
    # FIXME: insert proper C++ library support
    ld_shlibs=no
    ;;
  m88k*)
    # FIXME: insert proper C++ library support
    ld_shlibs=no
    ;;
  mvs*)
    case $cc_basename in
      cxx)
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
      *)
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
    esac
    ;;
  netbsd*)
    # NetBSD uses g++ - do we need to do anything?
    ;;
  osf3*)
    case $cc_basename in
      KCC)
        # Kuck and Associates, Inc. (KAI) C++ Compiler

        # KCC will only create a shared library if the output file
        # ends with ".so" (or ".sl" for HP-UX), so rename the library
        # to its proper name (with version) after linking.
        archive_cmds='templib=`echo $lib | sed -e "s/\.so\..*/\.so/"`; $CC $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags --soname $soname -o \$templib; mv \$templib $lib'

        hardcode_libdir_flag_spec='${wl}-rpath,$libdir'
        hardcode_libdir_separator=:

	# Archives containing C++ object files must be created using
	# "CC -Bstatic", where "CC" is the KAI C++ compiler.
        old_archive_cmds='$CC -Bstatic -o $oldlib $oldobjs'

        ;;
      RCC)
        # Rational C++ 2.4.1
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
      cxx)
        allow_undefined_flag=' ${wl}-expect_unresolved ${wl}\*'
        archive_cmds='$CC -shared${allow_undefined_flag} $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-soname $soname `test -n "$verstring" && echo ${wl}-set_version $verstring` -update_registry ${objdir}/so_locations -o $lib'

        hardcode_libdir_flag_spec='${wl}-rpath ${wl}$libdir'
        hardcode_libdir_separator=:

        # Commands to make compiler produce verbose output that lists
        # what "hidden" libraries, object files and flags are used when
        # linking a shared library.
        #
        # There doesn't appear to be a way to prevent this compiler from
        # explicitly linking system object files so we need to strip them
        # from the output so that they don't get included in the library
        # dependencies.
        output_verbose_link_cmd='templist=`$CC -shared $CFLAGS -v conftest.$objext 2>&1 | grep "ld" | grep -v "ld:"`; templist=`echo $templist | sed "s/\(^.*ld.*\)\( .*ld.*$\)/\1/"`; list=""; for z in $templist; do case $z in conftest.$objext) list="$list $z";; *.$objext);; *) list="$list $z";;esac; done; echo $list'
        ;;
      *)
	if test "$with_gcc" = yes && test "$with_gnu_ld" = no; then
	  allow_undefined_flag=' ${wl}-expect_unresolved ${wl}\*'
	  archive_cmds='$CC -shared -nostdlib ${allow_undefined_flag} $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-soname ${wl}$soname `test -n "$verstring" && echo ${wl}-set_version ${wl}$verstring` ${wl}-update_registry ${wl}${objdir}/so_locations -o $lib'

	  hardcode_libdir_flag_spec='${wl}-rpath ${wl}$libdir'
	  hardcode_libdir_separator=:

	  # Commands to make compiler produce verbose output that lists
	  # what "hidden" libraries, object files and flags are used when
	  # linking a shared library.
	  output_verbose_link_cmd='$CC -shared $CFLAGS -v conftest.$objext 2>&1 | egrep "\-L"'
    else
          # FIXME: insert proper C++ library support
          ld_shlibs=no
    fi
        ;;
    esac
    ;;
  osf4* | osf5*)
    case $cc_basename in
      KCC)
        # Kuck and Associates, Inc. (KAI) C++ Compiler

        # KCC will only create a shared library if the output file
        # ends with ".so" (or ".sl" for HP-UX), so rename the library
        # to its proper name (with version) after linking.
        archive_cmds='templib=`echo $lib | sed -e "s/\.so\..*/\.so/"`; $CC $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags --soname $soname -o \$templib; mv \$templib $lib'

        hardcode_libdir_flag_spec='${wl}-rpath,$libdir'
        hardcode_libdir_separator=:

	# Archives containing C++ object files must be created using
	# the KAI C++ compiler.
        old_archive_cmds='$CC -o $oldlib $oldobjs'
        ;;
      RCC)
        # Rational C++ 2.4.1
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
      cxx)
        allow_undefined_flag=' -expect_unresolved \*'
        archive_cmds='$CC -shared${allow_undefined_flag} $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags -msym -soname $soname `test -n "$verstring" && echo -set_version $verstring` -update_registry ${objdir}/so_locations -o $lib'
	archive_expsym_cmds='for i in `cat $export_symbols`; do printf "-exported_symbol " >> $lib.exp; echo "\$i" >> $lib.exp; done~
	  echo "-hidden">> $lib.exp~
	  $CC -shared$allow_undefined_flag $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags -msym -soname $soname -Wl,-input -Wl,$lib.exp  `test -n "$verstring" && echo -set_version	$verstring` -update_registry $objdir/so_locations -o $lib~
	  $rm $lib.exp'

        hardcode_libdir_flag_spec='-rpath $libdir'
        hardcode_libdir_separator=:

        # Commands to make compiler produce verbose output that lists
        # what "hidden" libraries, object files and flags are used when
        # linking a shared library.
        #
        # There doesn't appear to be a way to prevent this compiler from
        # explicitly linking system object files so we need to strip them
        # from the output so that they don't get included in the library
        # dependencies.
        output_verbose_link_cmd='templist=`$CC -shared $CFLAGS -v conftest.$objext 2>&1 | grep "ld" | grep -v "ld:"`; templist=`echo $templist | sed "s/\(^.*ld.*\)\( .*ld.*$\)/\1/"`; list=""; for z in $templist; do case $z in conftest.$objext) list="$list $z";; *.$objext);; *) list="$list $z";;esac; done; echo $list'
        ;;
      *)
	if test "$with_gcc" = yes && test "$with_gnu_ld" = no; then
	  allow_undefined_flag=' ${wl}-expect_unresolved ${wl}\*'
	  archive_cmds='$CC -shared -nostdlib ${allow_undefined_flag} $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-msym ${wl}-soname ${wl}$soname `test -n "$verstring" && echo ${wl}-set_version ${wl}$verstring` ${wl}-update_registry ${wl}${objdir}/so_locations -o $lib'
	  
	  hardcode_libdir_flag_spec='${wl}-rpath ${wl}$libdir'
	  hardcode_libdir_separator=:
	  
	  # Commands to make compiler produce verbose output that lists
	  # what "hidden" libraries, object files and flags are used when
	  # linking a shared library.
	  output_verbose_link_cmd='$CC -shared $CFLAGS -v conftest.$objext 2>&1 | egrep "\-L"'
    else
	  # FIXME: insert proper C++ library support
	  ld_shlibs=no
    fi
        ;;
    esac
    ;;
  psos*)
    # FIXME: insert proper C++ library support
    ld_shlibs=no
    ;;
  sco*)
    case $cc_basename in
      CC)
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
      *)
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
    esac
    ;;
  sunos4*)
    case $cc_basename in
      CC)
        # Sun C++ 4.x
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
      lcc)
        # Lucid
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
      *)
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
    esac
    ;;
  solaris*)
    case $cc_basename in
      CC)
	# Sun C++ 4.2, 5.x and Centerline C++
        no_undefined_flag=' -zdefs'
        archive_cmds='$CC -G${allow_undefined_flag} -nolib -h$soname -o $lib $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags'
        archive_expsym_cmds='$echo "{ global:" > $lib.exp~cat $export_symbols | sed -e "s/\(.*\)/\1;/" >> $lib.exp~$echo "local: *; };" >> $lib.exp~
        $CC -G${allow_undefined_flag} -nolib ${wl}-M ${wl}$lib.exp -h$soname -o $lib $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags~$rm $lib.exp'

        hardcode_libdir_flag_spec='-R$libdir'
        hardcode_shlibpath_var=no
        case $host_os in
          solaris2.[0-5] | solaris2.[0-5].*) ;;
          *)
            # The C++ compiler is used as linker so we must use $wl
            # flag to pass the commands to the underlying system
            # linker.
            # Supported since Solaris 2.6 (maybe 2.5.1?)
            whole_archive_flag_spec='${wl}-z ${wl}allextract$convenience ${wl}-z ${wl}defaultextract'
            ;;
        esac
        link_all_deplibs=yes

        # Commands to make compiler produce verbose output that lists
        # what "hidden" libraries, object files and flags are used when
        # linking a shared library.
        #
        # There doesn't appear to be a way to prevent this compiler from
        # explicitly linking system object files so we need to strip them
        # from the output so that they don't get included in the library
        # dependencies.
        output_verbose_link_cmd='templist=`$CC -G $CFLAGS -v conftest.$objext 2>&1 | egrep "\-R|\-L"`; list=""; for z in $templist; do case $z in conftest.$objext) list="$list $z";; *.$objext);; *) list="$list $z";;esac; done; echo $list'

	# Archives containing C++ object files must be created using
	# "CC -xar", where "CC" is the Sun C++ compiler.  This is
        # necessary to make sure instantiated templates are included
        # in the archive.
        old_archive_cmds='$CC -xar -o $oldlib $oldobjs'
        ;;
      gcx)
        # Green Hills C++ Compiler
        archive_cmds='$CC -shared $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-h $wl$soname -o $lib'

        # The C++ compiler must be used to create the archive.
        old_archive_cmds='$CC $LDFLAGS -archive -o $oldlib $oldobjs'
        ;;
      *)
        # GNU C++ compiler with Solaris linker
        if test "$with_gcc" = yes && test "$with_gnu_ld" = no; then
	  no_undefined_flag=' ${wl}-z ${wl}defs'
          if $CC --version | egrep -v '^2\.7' > /dev/null; then
            archive_cmds='$CC -shared -nostdlib $LDFLAGS $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-h $wl$soname -o $lib'
            archive_expsym_cmds='$echo "{ global:" > $lib.exp~cat $export_symbols | sed -e "s/\(.*\)/\1;/" >> $lib.exp~$echo "local: *; };" >> $lib.exp~
		$CC -shared -nostdlib ${wl}-M $wl$lib.exp -o $lib $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags~$rm $lib.exp'

            # Commands to make compiler produce verbose output that lists
            # what "hidden" libraries, object files and flags are used when
            # linking a shared library.
            output_verbose_link_cmd="$CC -shared $CFLAGS -v conftest.$objext 2>&1 | egrep \"\-L\""
          else
            # g++ 2.7 appears to require `-G' NOT `-shared' on this
            # platform.
            archive_cmds='$CC -G -nostdlib $LDFLAGS $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags ${wl}-h $wl$soname -o $lib'
            archive_expsym_cmds='$echo "{ global:" > $lib.exp~cat $export_symbols | sed -e "s/\(.*\)/\1;/" >> $lib.exp~$echo "local: *; };" >> $lib.exp~
		$CC -G -nostdlib ${wl}-M $wl$lib.exp -o $lib $predep_objects $libobjs $deplibs $postdep_objects $compiler_flags~$rm $lib.exp'

            # Commands to make compiler produce verbose output that lists
            # what "hidden" libraries, object files and flags are used when
            # linking a shared library.
            output_verbose_link_cmd="$CC -G $CFLAGS -v conftest.$objext 2>&1 | egrep \"\-L\""
          fi

          hardcode_libdir_flag_spec='${wl}-R $wl$libdir'
        fi
        ;;
    esac
    ;;
  tandem*)
    case $cc_basename in
      NCC)
        # NonStop-UX NCC 3.20
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
      *)
        # FIXME: insert proper C++ library support
        ld_shlibs=no
        ;;
    esac
    ;;
  unixware*)
    # FIXME: insert proper C++ library support
    ld_shlibs=no
    ;;
  vxworks*)
    # FIXME: insert proper C++ library support
    ld_shlibs=no
    ;;
  *)
    # FIXME: insert proper C++ library support
    ld_shlibs=no
    ;;
esac


## Compiler Characteristics: PIC flags, static flags, etc

# We don't use cached values here since only the C compiler
# characteristics should be cached.
ac_cv_prog_cc_pic=
ac_cv_prog_cc_shlib=
ac_cv_prog_cc_wl=
ac_cv_prog_cc_static=
ac_cv_prog_cc_no_builtin=
ac_cv_prog_cc_can_build_shared=$can_build_shared

ac_cv_prog_cc_pic_works=
ac_cv_prog_cc_static_works=

if test "$with_gcc" = yes; then
  ac_cv_prog_cc_wl='-Wl,'
  ac_cv_prog_cc_static='-static'

  case $host_os in
  aix*)
    # All AIX code is PIC.
    if test "$host_cpu" = ia64; then
      # AIX 5 now supports IA64 processor
      lt_cv_prog_cc_static='-Bstatic'
    else
      lt_cv_prog_cc_static='-bnso -bI:/lib/syscalls.exp'
    fi
    ;;
  amigaos*)
    # FIXME: we need at least 68020 code to build shared libraries, but
    # adding the `-m68020' flag to GCC prevents building anything better,
    # like `-m68040'.
    ac_cv_prog_cc_pic='-m68020 -resident32 -malways-restore-a4'
    ;;
  beos* | irix5* | irix6* | osf3* | osf4* | osf5*)
    # PIC is the default for these OSes.
    ;;
  cygwin* | mingw* | os2*)
    # This hack is so that the source file can tell whether it is being
    # built for inclusion in a dll (and should export symbols for example).
    ac_cv_prog_cc_pic='-DDLL_EXPORT'
    ;;
  darwin* | rhapsody*)
    # PIC is the default on this platform
    # Common symbols not allowed in MH_DYLIB files
    lt_cv_prog_cc_pic='-fno-common'
    ;;
  *djgpp*)
    # DJGPP does not support shared libraries at all
    ac_cv_prog_cc_pic=
    ;;
  sysv4*MP*)
    if test -d /usr/nec; then
      ac_cv_prog_cc_pic=-Kconform_pic
    fi
    ;;
  *)
    ac_cv_prog_cc_pic='-fPIC'
    ;;
  esac
else
  case $host_os in
    aix4* | aix5*)
      # All AIX code is PIC.
      if test "$host_cpu" = ia64; then
        # AIX 5 now supports IA64 processor
        lt_cv_prog_cc_static='-Bstatic'
      else
        lt_cv_prog_cc_static='-bnso -bI:/lib/syscalls.exp'
      fi
      ;;
    chorus*)
      case $cc_basename in
      cxch68)
        # Green Hills C++ Compiler
        # ac_cv_prog_cc_static="--no_auto_instantiation -u __main -u __premain -u _abort -r $COOL_DIR/lib/libOrb.a $MVME_DIR/lib/CC/libC.a $MVME_DIR/lib/classix/libcx.s.a"
        ;;
      esac
      ;;
    dgux*)
      case $cc_basename in
        ec++)
          ac_cv_prog_cc_pic='-KPIC'
          ;;
        ghcx)
          # Green Hills C++ Compiler
          ac_cv_prog_cc_pic='-pic'
          ;;
        *)
          ;;
      esac
      ;;
    freebsd*)
      # FreeBSD uses GNU C++
      ;;
    gnu*)
      ;;
    hpux9* | hpux10* | hpux11*)
      case $cc_basename in
        CC)
          ac_cv_prog_cc_wl='-Wl,'
          ac_cv_prog_cc_static="${ac_cv_prog_cc_wl}-a ${ac_cv_prog_cc_wl}archive"
          ac_cv_prog_cc_pic='+Z'
          ;;
        aCC)
          ac_cv_prog_cc_wl='-Wl,'
          ac_cv_prog_cc_static="${ac_cv_prog_cc_wl}-a ${ac_cv_prog_cc_wl}archive"
          ac_cv_prog_cc_pic='+Z'
          ;;
        *)
          ;;
      esac
      ;;
    irix5* | irix6*)
      case $cc_basename in
        CC)
          ac_cv_prog_cc_wl='-Wl,'
          ac_cv_prog_cc_static='-non_shared'
          ac_cv_prog_cc_pic='-KPIC'
          ;;
        *)
          ;;
      esac
      ;;
    linux*)
      case $cc_basename in
        KCC)
          # KAI C++ Compiler
          ac_cv_prog_cc_wl='--backend -Wl,'
          ac_cv_prog_cc_pic='-fPIC'
          ;;
        cxx)
          # Compaq C++
          # Make sure the PIC flag is empty.  It appears that all Alpha
          # Linux and Compaq Tru64 Unix objects are PIC.
          ac_cv_prog_cc_pic=
          ac_cv_prog_cc_static='-non_shared'
          ;;
        *)
          ;;
      esac
      ;;
    lynxos*)
      ;;
    m88k*)
      ;;
    mvs*)
      case $cc_basename in
        cxx)
          ac_cv_prog_cc_pic='-W c,exportall'
          ;;
        *)
          ;;
      esac
      ;;
    netbsd*)
      ;;
    osf3* | osf4* | osf5*)
      case $cc_basename in
        KCC)
          ac_cv_prog_cc_wl='--backend -Wl,'
          ;;
        RCC)
          # Rational C++ 2.4.1
          ac_cv_prog_cc_pic='-pic'
          ;;
        cxx)
          # Digital/Compaq C++
          ac_cv_prog_cc_wl='-Wl,'
          # Make sure the PIC flag is empty.  It appears that all Alpha
          # Linux and Compaq Tru64 Unix objects are PIC.
          ac_cv_prog_cc_pic=
          ac_cv_prog_cc_static='-non_shared'
          ;;
        *)
          ;;
      esac
      ;;
    psos*)
      ;;
    sco*)
      case $cc_basename in
        CC)
          ac_cv_prog_cc_pic='-fPIC'
          ;;
        *)
          ;;
      esac
      ;;
    solaris*)
      case $cc_basename in
        CC)
          # Sun C++ 4.2, 5.x and Centerline C++
          ac_cv_prog_cc_pic='-KPIC'
          ac_cv_prog_cc_static='-Bstatic'
          ac_cv_prog_cc_wl='-Qoption ld '
          ;;
        gcx)
          # Green Hills C++ Compiler
          ac_cv_prog_cc_pic='-PIC'
          ;;
        *)
          ;;
      esac
      ;;
    sunos4*)
      case $cc_basename in
        CC)
          # Sun C++ 4.x
          ac_cv_prog_cc_pic='-pic'
          ac_cv_prog_cc_static='-Bstatic'
          ;;
        lcc)
          # Lucid
          ac_cv_prog_cc_pic='-pic'
          ;;
        *)
          ;;
      esac
      ;;
    tandem*)
      case $cc_basename in
        NCC)
          # NonStop-UX NCC 3.20
          ac_cv_prog_cc_pic='-KPIC'
          ;;
        *)
          ;;
      esac
      ;;
    unixware*)
      ;;
    vxworks*)
      ;;
    *)
      ac_cv_prog_cc_can_build_shared=no
      ;;
  esac
fi

case "$host_os" in
    # Platforms which do not suport PIC and -DPIC is meaningless
    # on them:
    *djgpp*)
      ac_cv_prog_cc_pic=
      ;;
    *)
      ac_cv_prog_cc_pic="$ac_cv_prog_cc_pic -DPIC"
      ;;
esac


# Figure out "hidden" C++ library dependencies from verbose
# compiler output whening linking a shared library.
cat > conftest.$ac_ext <<EOF
class Foo
{
public:
  Foo (void) { a = 0; }
private:
  int a;
};
EOF


if (eval $ac_compile) 2>&5; then
  # Parse the compiler output and extract the necessary
  # objects, libraries and library flags.

  # Sentinel used to keep track of whether or not we are before
  # the conftest object file.
  pre_test_object_deps_done=no

  for p in `eval $output_verbose_link_cmd`; do

    case $p in

    -L* | -R* | -l*)
       # Some compilers place space between "-{L,R}" and the path.
       # Remove the space.
       if test $p = "-L" \
          || test $p = "-R"; then
         prev=$p
         continue
       else
         prev=
       fi

       if test "$pre_test_object_deps_done" = no; then
         case $p in
	 -L* | -R*)
	   # Internal compiler library paths should come after those
	   # provided the user.  The postdeps already come after the
	   # user supplied libs so there is no need to process them.
           if test -z "$compiler_lib_search_path"; then
             compiler_lib_search_path="${prev}${p}"
           else
             compiler_lib_search_path="${compiler_lib_search_path} ${prev}${p}"
           fi
           ;;
         # The "-l" case would never come before the object being
         # linked, so don't bother handling this case.
         esac
       else
         if test -z "$postdeps"; then
           postdeps="${prev}${p}"
         else
           postdeps="${postdeps} ${prev}${p}"
         fi
       fi
       ;;

    *.$objext)
       # This assumes that the test object file only shows up
       # once in the compiler output.
       if test "$p" = "conftest.$objext"; then
         pre_test_object_deps_done=yes
         continue
       fi

       if test "$pre_test_object_deps_done" = no; then
         if test -z "$predep_objects"; then
           predep_objects="$p"
         else
           predep_objects="$predep_objects $p"
         fi
       else
         if test -z "$postdep_objects"; then
           postdep_objects="$p"
         else
           postdep_objects="$postdep_objects $p"
         fi
       fi
       ;;

    *) ;; # Ignore the rest.

    esac
  done

  # Clean up.
  rm -f a.out
else
  echo "ltcf-cxx.sh: error: problem compiling test program"
fi

$rm -f confest.$objext

case " $postdeps " in
*" -lc "*) need_lc=no ;;
*) need_lc=yes ;;
esac
