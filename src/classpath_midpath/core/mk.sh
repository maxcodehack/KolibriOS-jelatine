


#find2 ./ -iname '*.java'  | sed 's-\./--g'  >files_java_mod2.lst

function delete_class_files(){
	find2 ./ -iname '*.class' >files_class.lst
	cat files_class.lst
	rm -f `cat files_class.lst`
	#ls ./*/*.class
	#rm -f ./*/*.class
}
function delete_java_class_files(){
	#find2 ./java -iname '*.class' >files_class.lst
	#cat files_class.lst
	rm -f `cat files_class.lst`
	#ls ./*/*.class
	#rm -f ./*/*.class
}
function delete_custom_class_files(){
	if [ -z $1 ];
	then
		echo delete_custom_class_files need param
		echo example delete_custom_class_files ./java
		exit 1
	fi
	find2 $1 -iname '*.class' >files_class.lst
	cat files_class.lst
	rm -f `cat files_class.lst`
	#ls ./*/*.class
	#rm -f ./*/*.class
}

#delete_class_files
#delete_java_class_files
#delete_custom_class_files ./java
#delete_custom_class_files ./org/thenesis/midpath
#delete_custom_class_files ./org/thenesis/microbackend
#delete_custom_class_files  ./javax
#exit
JAVA_FILE_LIST="files_java_mod2.lst"
FAILED_FILES=""
#BREAK_ON_ERRORS="1"
#FORCE_DELETE_CLASS_FILE="1"
FORCE_RECOMPILE="1"
COMPILE_NUM_FILES="10"
#COMPILE_NUM_FILES="3"
#COMPILE_NUM_FILES="1"
function report_fails(){
	if ! [ -z $FAILED_FILES ];
	then
		echo Failed compile:
		for n in $FAILED_FILES;
		do
			echo $n
		done
	else
		echo No errors detected!
	fi
}
function compile_src(){
	ix="0"
	J_FILES=""
	FAILED_FILES=""
	JAVAC="javac -Xlint:unchecked -source 1.5 -target 1.5"
	#JAVAC="jikes -target 1.5"
	#JCFLAGS="-source 1.5 -target 1.4"
	#JCFLAGS="-source 1.4 -target 1.5"
	HAS_ERROR=""
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
		
		if ! [ -z $FORCE_DELETE_CLASS_FILE ];
		then
			rm -f $class_name.class 2>/dev/null
		fi
		if [ -e $class_name.class ] &&  [ -z $FORCE_RECOMPILE ];
		then
			echo exists:$class_name.class
		else
			ix=$(expr $ix + 1)
			J_FILES="$J_FILES $n"
		fi
		#echo $ix
		if [ $COMPILE_NUM_FILES == $ix ];
		then
			cmdline="$JAVAC $JCFLAGS  -bootclasspath . -cp . -sourcepath . $J_FILES"
			echo "try $cmdline"
			$cmdline 
			err="$?"
			if [ 0 == $err ];
			then
				echo "ok compiled: $J_FILES"
			else
				echo "failed compile: $J_FILES"
				FAILED_FILES="$FAILED_FILES $J_FILES"
				if ! [ -z $BREAK_ON_ERRORS ];
				then
					ix="0";
					HAS_ERROR="1"
					break;
					#return 1
				fi
			fi
			
			
			ix=0;
			J_FILES=""
			#break
		fi
		#break;
	done
	if  [ $ix != 0 ];
	then
		#$JAVAC $JCFLAGS   -bootclasspath . -cp . -sourcepath . $J_FILES \
		#|| return 1
		cmdline="$JAVAC $JCFLAGS  -bootclasspath . -cp . -sourcepath . $J_FILES"
		echo "try $cmdline"
		$cmdline 
		err="$?"
		if [ 0 == $err ];
		then
			echo "ok compiled: $J_FILES"
		else
			echo "failed compile: $J_FILES"
			FAILED_FILES="$FAILED_FILES $J_FILES"
			ix="0";
			HAS_ERROR="1"
			#break;
				#return 1
			
			#return 1
		fi
		ix=0;
		J_FILES=""
	fi
	report_fails
	! [ -z $HAS_ERROR ] && return 1
}
BUILD_DIR="../build_dir"
function preverify_src(){
	ix="0"
	CL_FILES=""
	FAILED_FILES=""
	PREVERIFY="J:/algoritm/S60_3rd_fp1/S60_3rd_MIDP_SDK_FP1/bin/preverify.exe"
	PREV_FLAGS="-classpath . -d $BUILD_DIR"
	#JAVAC="javac -Xlint:unchecked -source 1.5 -target 1.5"
	#JAVAC="jikes -target 1.5"
	#JCFLAGS="-source 1.5 -target 1.4"
	#JCFLAGS="-source 1.4 -target 1.5"
	
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
		
		if ! [ -z $FORCE_DELETE_CLASS_FILE ];
		then
			rm -f $BUILD_DIR/$class_name.class 2>/dev/null
		fi
		if [ -e $BUILD_DIR/$class_name.class ] &&  [ -z $FORCE_RECOMPILE ];
		then
			echo exists:$BUILD_DIR/$class_name.class
		else
			ix=$(expr $ix + 1)
			CL_FILES="$CL_FILES $class_name"
		fi
		#echo $ix
		if [ $COMPILE_NUM_FILES == $ix ];
		then
			cmdline="$PREVERIFY $PREV_FLAGS $CL_FILES"
			echo "try $cmdline"
			$cmdline 
			err="$?"
			if [ 0 == $err ];
			then
				echo "ok preverify: $CL_FILES"
			else
				echo "failed preverify: $CL_FILES"
				FAILED_FILES="$FAILED_FILES $CL_FILES"
				if ! [ -z $BREAK_ON_ERRORS ];
				then
					ix="0";
					HAS_ERROR="1"
					break;
					#return 1
				fi
			fi
			
			
			ix=0;
			CL_FILES=""
			#break
		fi
		#break;
	done
	if  [ $ix != 0 ];
	then
		#$JAVAC $JCFLAGS   -bootclasspath . -cp . -sourcepath . $J_FILES \
		#|| return 1
		#cmdline="$JAVAC $JCFLAGS  -bootclasspath . -cp . -sourcepath . $J_FILES"
		cmdline="$PREVERIFY $PREV_FLAGS $CL_FILES"
		echo "try $cmdline"
		$cmdline 
		err="$?"
		if [ 0 == $err ];
		then
			echo "ok preverify: $CL_FILES"
		else
			echo "failed preverify: $CL_FILES"
			FAILED_FILES="$FAILED_FILES $CL_FILES"
			ix="0";
			HAS_ERROR="1"
			#return 1
		fi
		ix=0;
		CL_FILES=""
	fi
	report_fails
	! [ -z $HAS_ERROR ] && return 1
}
function preverify_custom(){
	JAVA_FILE_LIST="$1"
	preverify_src
}

function compile_jelatine_pack(){
	find2 ./jelatine -iname '*.java' | \
		sed 's-\./--g' >files_jel_java.lst
	JAVA_FILE_LIST="files_jel_java.lst"
	compile_src
}
function compile_com_pack(){
	#find2 ./com -iname '*.java' | \
	#	sed 's-\./--g' >files_com_java.lst
	JAVA_FILE_LIST="files_com_java.lst"
	compile_src
}
function compile_midpath_pack(){
	
	#find2 ./org/thenesis/midpath -iname '*.java' | \
	#	sed 's-\./--g' >files_midpath_java.lst
	JAVA_FILE_LIST="files_midpath_java.lst"
	compile_src
}


function compile_microbackend_pack(){
	#find2 ./org/thenesis/microbackend -iname '*.java' | \
	#	sed 's-\./--g' >files_microbackend_java.lst
	JAVA_FILE_LIST="files_microbackend_java.lst"
	compile_src
}
function compile_javax_pack(){
	#find2 ./javax -iname '*.java' | \
	#	sed 's-\./--g' >files_javax_java.lst
	JAVA_FILE_LIST="files_javax_java.lst"
	compile_src
}
function compile_java_pack(){
	#find2 ./java -iname '*.java' | \
	#	sed 's-\./--g' >files_java2_java.lst
	JAVA_FILE_LIST="files_java2_java.lst"
	#JAVA_FILE_LIST="files_java_java.lst"
	compile_src
}
function compile_org_kxml(){ 
	#find2 ./org/kxml2 -iname '*.class' | \
	#	sed 's-\./--g' >files_org_xml_java.lst
	#find2 ./org/xmlpull -iname '*.class' | \
	#	sed 's-\./--g' >>files_org_xml_java.lst
	JAVA_FILE_LIST="files_org_xml_java.lst"
	preverify_src
}
function mk_natives_com(){
	fgrep 'native ' -r ./com >..\natives_com.lst
	fgrep 'native ' -r ./org >..\natives_org.lst
	fgrep 'native ' -r ./javax >..\natives_javax.lst
	fgrep 'native ' -r ./sdljava >..\natives_sdl.lst
	
}

#compile_src
#compile_jelatine_pack
#preverify_src

#compile_com_pack 
#preverify_src
#preverify_custom files_com_java.lst

mk_natives_com

#compile_midpath_pack
#preverify_src
#compile_microbackend_pack
#preverify_src
#compile_javax_pack
#preverify_src
#compile_java_pack
#preverify_src

#compile_org_kxml 
