if which hg > /dev/null; then
	HG=hg
elif which /usr/bin/hg; then
	HG=/usr/bin/hg
else
	echo "Mercurial not found!"
	exit 1
fi

cd ..
FILES=$($HG status -A | grep -v "^I" | awk '{ print $2 }' | grep -v '\.o' | sed 's/\\/\//g')


echo $FILES | xargs md5sum | sed 's/\*//g'
echo -n "Total lines of code (c, h, S): "
wc -l `/bin/find . -name '*.[chS]' | egrep -v '(track_data.c|a0_track)'` | tail -1