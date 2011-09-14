
NAME=sylfm
TARGET=$NAME.dll
OBJS="$NAME.o"
OBJS="$NAME.o version.o"
PKG=sylpheed-$NAME
LIBSYLPH=./lib/libsylph-0-1.a
LIBSYLPHEED=./lib/libsylpheed-plugin-0-1.a
LIBSYLFILTER=./lib/libsylfilter.a
#LIBS=" -lglib-2.0-0  -lintl"
LIBS=" `pkg-config --libs glib-2.0 gobject-2.0 gtk+-2.0 gthread-2.0`"
INC=" -I. -I../../ -I../../libsylph -I../../src -I/mingw/local `pkg-config --cflags glib-2.0 cairo gdk-2.0 gtk+-2.0 gthread-2.0`"

USE_AQUESTALK=1
DEF=" -DHAVE_CONFIG_H -DUNICODE -D_UNICODE "
DEBUG=0
LIB_AQUESTALK=""
if [ "$USE_AQUESTALK" -eq 1 ]; then
    DEF="$DEF -DUSE_AQUESTALK -DMULTI_STR_CODE"
    LIB_AQUESTALK=" -lAquesTalk2 -lAquesTalk2Da -lAqKanji2Koe AquesTalk2Da.dll AqKanji2Koe.dll"
fi

function compile ()
{
    if [ ! -f "private_build.h" ]; then
        echo "1" > .compile
        echo "#define PRIVATE_BUILD 1" > private_build.h
    else
        ret=`cat .compile | gawk '{print $i+1}'`
        echo $ret | tee .compile
        echo "#define PRIVATE_BUILD \"build $ret\\0\"" > private_build.h
    fi
    com="windres -i version.rc -o version.o"
    echo $com
    eval $com

    com="gcc -Wall -c $DEF $INC $NAME.c"
    echo $com
    eval $com
    if [ $? != 0 ]; then
        echo "compile error"
        exit
    fi
    com="gcc -shared -o $TARGET $OBJS -L./lib $LIBSYLPH $LIBSYLPHEED $LIBSYLFILTER $LIBS -lssleay32 -leay32 -lws2_32 -liconv -lonig"
    echo $com
    eval $com
    if [ $? != 0 ]; then
        echo "done"
    else
        DEST="/C/Users/$LOGNAME/AppData/Roaming/Sylpheed/plugins"
        if [ -d "$DEST" ]; then
            com="cp $TARGET $DEST/$NAME.dll"
            echo $com
            eval $com
        else
            DEST="/C/Documents and Settings/$LOGNAME/Application Data/Sylpheed/plugins"
            if [ -d "$DEST" ]; then
                com="cp $TARGET \"$DEST/$NAME.dll\""
                echo $com
                eval $com
            fi
        fi
    fi

}

if [ -z "$1" ]; then
    compile
else
    while [  $# -ne 0 ]; do
        case "$1" in
            -debug|--debug)
                DEF=" -ggdb $DEF -DDEBUG"
                shift
                ;;
            pot)
                mkdir -p po
                com="xgettext $NAME.h $NAME.c -k_ -kN_ -o po/$NAME.pot"
                echo $com
                eval $com
                shift
                ;;
            po)
                com="msgmerge po/ja.po po/$NAME.pot -o po/ja.po"
                echo $com
                eval $com
                shift
                ;;
            mo)
                com="msgfmt po/ja.po -o po/$NAME.mo"
                echo $com
                eval $com
                DEST="/C/apps/Sylpheed/lib/locale/ja/LC_MESSAGES"
                if [ -d "$DEST" ]; then
                    com="cp po/$NAME.mo $DEST/$NAME.mo"
                    echo $com
                    eval $com
                fi
                exit
                ;;
            res)
                com="windres -i version.rc -o version.o"
                echo $com
                eval $com
                shift
                ;;
            -r|release)
                shift
                if [ ! -z "$1" ]; then
                    r=$1
                    shift
                    zip sylpheed-$NAME-$r.zip $NAME.dll
                    zip -r sylpheed-$NAME-$r.zip README.ja.txt
                    #zip -r sylpheed-$NAME-$r.zip $NAME.c
                    zip -r sylpheed-$NAME-$r.zip po/$NAME.mo
                    zip -r sylpheed-$NAME-$r.zip *.xpm
                fi
                ;;
            -c|-compile)
                shift
                if [ ! -z "$1" ]; then
                    if [ "$1" = "stable" ]; then
                        DEF="$DEF -DSTABLE_RELEASE";
                        shift
                    fi
                fi
                compile
                ;;
            def)
                shift
                PKG=libsylph-0-1
                com="(cd lib;pexports $PKG.dll > $PKG.dll.def)"
                echo $com
                eval $com
                com="(cd lib;dlltool --dllname $PKG.dll --input-def $PKG.dll.def --output-lib $PKG.a)"
                echo $com
                eval $com
                PKG=libsylpheed-plugin-0-1
                com="(cd lib;pexports $PKG.dll > $PKG.dll.def)"
                echo $com
                eval $com
                com="(cd lib;dlltool --dllname $PKG.dll --input-def $PKG.dll.def --output-lib $PKG.a)"
                echo $com
                eval $com
                exit
                ;;
            *)
                shift
                ;;
        esac
    done
fi
