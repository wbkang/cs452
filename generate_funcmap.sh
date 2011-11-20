 #!/bin/sh

FUNCTION_COUNT=`find . -name '*.S' -o -name '*.s' | xargs grep .global | awk '{print $3}' | grep -v '^$' | grep -v '^__' | sort | uniq | wc -l`

FUNCTIONS=`find . -name '*.S' -o -name '*.s' | xargs grep .global | awk '{print $3}' | egrep -v '(^$|^__|PLT|GOT|,)' | sort | uniq`

 OUTFILE=./main/funcmap.c

 echo > $OUTFILE

cd include
for header in `find . -name '*.h' | grep -v '\._'`; do
	header=$header
	echo "#include <${header}>" >> ../$OUTFILE
done
cd ..
cd main/include
for header in `find . -name '*.h' | grep -v '\._'`; do
	header=$header
	echo "#include <${header}>" >> ../../$OUTFILE
done
cd ../../
cd kernel/include
for header in `find . -name '*.h' | grep -v '\._'`; do
	header=$header
	echo "#include <${header}>" >> ../../$OUTFILE
done
cd ../../

#ctags -x  --c-kinds=p --format=1 `find . -name '*.h'` | sed 's/^.*\.h /extern /g' | grep -v inline>> $OUTFILE



 echo "static funcinfo __funclist[$(($FUNCTION_COUNT+1))];" >> $OUTFILE

 echo "void __init_funclist() {" >> $OUTFILE
 echo "int i = 0;" >> $OUTFILE
 for f in $FUNCTIONS; do
	echo "__funclist[i].fn=(unsigned int)$f;__funclist[i++].name=\"$f\";" >> $OUTFILE
 done
 echo "__funclist[i].fn=(unsigned int)REDBOOT_ENTRYPOINT;__funclist[i++].name=\"REDBOOT_ENTRYPOINT\";" >> $OUTFILE
 echo "__funclist[i].fn=0;" >> $OUTFILE
 echo "}" >> $OUTFILE

 echo "funcinfo* __getfunclist() { return __funclist; }" >> $OUTFILE
