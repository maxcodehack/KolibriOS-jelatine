


#find2 ./ -iname '*.java' >files_java.lst
#sed 's-\./--g'< files_java.lst  >files_java_mod2.lst

function delete_class_files(){
	find2 ./ -iname '*.class' >files_class.lst
	cat files_class.lst
	rm -f `cat files_class.lst`
	#ls ./*/*.class
	#rm -f ./*/*.class
}

#delete_class_files
#exit
JAVA_FILE_LIST="files_java_mod2.lst"
function compile_src(){
	ix="0"
	J_FILES=""
	JAVAC="javac -Xlint:unchecked "
	#JAVAC="jikes -target 1.5"
	#JCFLAGS="-source 1.5 -target 1.4"
	JCFLAGS="-source 1.5 -target 1.5"
	#FORCE_RECOMPILE="1"
	#rm -f java/io/BufferedWriter.class
	for n in `cat $JAVA_FILE_LIST`;
	do
		class_name="${n%%.java}";
		#echo $class_name
		sub_annotation="${class_name##java/lang/annotation/}"
		if [ $sub_annotation != $class_name  ];
		then
			void=""
			continue
		fi
		sub_tire="${class_name##-}"
		if [ $sub_tire != $class_name  ];
		then
			#skip
			continue
		fi
		if [ -e $class_name.class ] &&  [ -z $FORCE_RECOMPILE ];
		then
			echo exists:$class_name.class
		else
			ix=$(expr $ix + 1)
			J_FILES="$J_FILES $n"
		fi
		#echo $ix
		if [ 30 == $ix ];
		then
			$JAVAC $JCFLAGS  -bootclasspath . -cp . -sourcepath . $J_FILES || exit 1
			ix=0;
			J_FILES=""
			#break
		fi
		#break;
	done
	if  [ $ix != 0 ];
	then
		$JAVAC $JCFLAGS   -bootclasspath . -cp . -sourcepath . $J_FILES \
		|| exit 1
		ix=0;
		J_FILES=""
	fi
}

function compile_jelatine_pack(){
	#find2 ./jelatine -iname '*.java' | \
	#	sed 's-\./--g' >files_jel_java.lst
	JAVA_FILE_LIST="files_jel_java.lst"
	compile_src
}

compile_src

#compile_jelatine_pack

#find2 ./ -iname '*.class' >files_class2.lst
#cat files_class.lst
jar cvf C:/Programs/jelatine/classpath_nxj.jar `cat files_class2.lst`
