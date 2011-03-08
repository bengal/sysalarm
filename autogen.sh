echo "Regenerating autotools files"
autoreconf --install || exit 1
echo "Now run ./configure, make, and make install."
