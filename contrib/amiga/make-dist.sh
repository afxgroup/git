#!/bin/sh
#
# Builds the AmigaOS 4 distribution archive Git_Installer.lha from a
# finished build ("make amiga-dist" calls this after "make all"):
#
#   Git_Installer.info
#   Git_Installer/Install-Git(.info)        Installer script
#   Git_Installer/Installer-Files/          what Install-Git copies to Git/
#   Git_Installer/Installer-Files/LIBS/     clib4.library
#
# usage: make-dist.sh <version> <clib4.library> <ca-bundle> <output.lha>

set -e

if test $# -ne 4
then
	echo >&2 "usage: $0 <version> <clib4.library> <ca-bundle> <output.lha>"
	exit 1
fi

VERSION=$1
CLIB4=$2
CA_BUNDLE=$3
OUTPUT=$4
AMIGA=contrib/amiga
STAGE=.amiga-dist
FILES=$STAGE/Git_Installer/Installer-Files

for f in "$CLIB4" "$CA_BUNDLE" git git-remote-http git-shell git-submodule \
	 scalar templates/blt gitweb/gitweb.cgi
do
	if ! test -e "$f"
	then
		echo >&2 "make-dist.sh: missing $f"
		exit 1
	fi
done

rm -rf "$STAGE"
mkdir -p "$FILES/LIBS"

# Installer, with the version of this build
sed -e "s/^\(;\* \$VER: Install-Git \)[^ ]* (.*)/\1$VERSION ($(date +%d.%m.%Y))/" \
    -e "s/^(set #app-version \"[^\"]*\")/(set #app-version \"$VERSION\")/" \
    "$AMIGA/Install-Git" >"$STAGE/Git_Installer/Install-Git"
cp "$AMIGA/icons/Git_Installer.info" "$STAGE/"
cp "$AMIGA/icons/Install-Git.info" "$STAGE/Git_Installer/"

# Programs (the builtin commands are links to git made by Install-Git)
cp git git-remote-http git-shell git-submodule scalar "$FILES/"
cp "$CLIB4" "$FILES/LIBS/clib4.library"
cp "$CA_BUNDLE" "$FILES/curl-ca-bundle.crt"

# Documentation and support files
cp "$AMIGA/AMIGA-KNOWN-ISSUES.txt" "$AMIGA/OpenGuide.rexx" \
   "$AMIGA/git-commands.guide" "$AMIGA/icons/git-commands.guide.info" \
   LGPL-2.1 "$FILES/"

# Templates: the content of templates/blt is what GIT:templates must hold
mkdir -p "$FILES/templates"
cp -R templates/blt/. "$FILES/templates/"

# Translations (ISO encoded when USE_ISO_ENCODING is set)
if test -d po/build/locale
then
	cp -R po/build/locale "$FILES/locale"
fi

# gitweb
mkdir -p "$FILES/gitweb"
cp gitweb/gitweb.cgi "$FILES/gitweb/"
cp -R gitweb/static "$FILES/gitweb/static"

rm -f "$OUTPUT"
(cd "$STAGE" && lha aqo5 "../$OUTPUT" Git_Installer.info Git_Installer)
rm -rf "$STAGE"
echo "Created $OUTPUT"
