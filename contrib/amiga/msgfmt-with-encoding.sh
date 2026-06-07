#!/bin/sh
# Convert .po files to appropriate ISO encoding before compiling to .mo
# Usage: msgfmt-with-encoding.sh input.po output.mo

INPUT="$1"
OUTPUT="$2"

# Extract language code from filename (e.g., po/it.po -> it)
LANG=$(basename "$INPUT" .po)

# Determine target encoding based on language
# Note: Only convert languages where we're confident the encoding will work
case "$LANG" in
    # ISO-8859-15 (Western European) - only basic Latin languages
    de|fr|it|pt_PT|es|sv|nl|da)
        ENCODING="ISO-8859-15"
        ;;
    ru)
        ENCODING="ISO-8859-5"
        ;;
    el)
        ENCODING="ISO-8859-7"
        ;;
    pl|hu|cs|sk|sl|hr|ro|bg|mk|sr|bs|sq|fa)
        ENCODING="ISO-8859-2"
        ;;
    # ISO-8859-9 (Turkish)
    tr)
        ENCODING="ISO-8859-9"
        ;;
    # ISO-8859-8 (Hebrew)
    he)
        ENCODING="ISO-8859-8"
        ;;
    # ISO-8859-6 (Arabic)
    ar)
        ENCODING="ISO-8859-6"
        ;;
    # ISO-8859-4 (Baltic languages)
    lt|lv|et)
        ENCODING="ISO-8859-4"
        ;;
    # ISO-8859-3 (South European languages)
    el|mt)
        ENCODING="ISO-8859-3"
        ;;
    # ISO-8859-1 (Western European) - fallback for other Western languages
    fi|no|is)
        ENCODING="ISO-8859-1"
        ;;
    # Keep UTF-8 for all other languages to avoid conversion issues
    *)
        ENCODING="UTF-8"
        ;;
esac

if [ "$ENCODING" = "UTF-8" ]; then
    # No conversion needed, compile directly
    exec msgfmt -o "$OUTPUT" "$INPUT"
else
    # Convert encoding with error handling
    # Create temporary file for converted content
    TMPFILE="${OUTPUT}.tmp.po"
    
    # Try conversion, fall back to UTF-8 if it fails
    if sed "s/charset=UTF-8/charset=$ENCODING/g" "$INPUT" | \
       iconv -f UTF-8 -t "$ENCODING//TRANSLIT" 2>/dev/null > "$TMPFILE" && \
       msgfmt --no-convert -o "$OUTPUT" "$TMPFILE" 2>/dev/null; then
        # Conversion succeeded
        rm -f "$TMPFILE"
        exit 0
    else
        # Conversion failed, fall back to UTF-8
        rm -f "$TMPFILE"
        exec msgfmt -o "$OUTPUT" "$INPUT"
    fi
fi
