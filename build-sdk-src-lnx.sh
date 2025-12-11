#!/usr/bin/bash

#default orign and target file path
src=./
dst=../bin/lnx/sdk/src

#rm -rf $dst

######################################## customer config area begin ###############################################
# (1) origin and target file path, should use '/' but not '\'
src=./
dst=../bin/lnx/sdk/src

mkdir -p $dst

# (2) copy file type list
copyFileList=(
	'h'
	'hpp'
)
copyFileListSize=${#copyFileList[*]}

# (3) ignore catalog list
ignoreDirList=(
        '3rd'
        'Resources'
        'examples'
	'lib_x64'
	'lnx'
	'bin'
	'projects'
	'DynEditor'
	'deprecated'
	'debug'
	'release'
	'x86'
	'x64'
	'CMakeModules'
	'*tlog'
	'rapidjson-1.1.0'
	'EquinoxPortal'
	'EquinoxRequestDemo'
	'EquinoxServer'
	'Frama'
	'InstanceBook'
	'InstanceStage'
	'LicenseManager'
	'Mantle'
	'Mantler'
	'npc'
	'npcbook'
	'NpcStage'
	'profilor'
	'test-*'
)
ignoreDirListSize=${#ignoreDirList[*]}

# if the script file is double cliecked to execute, the path is automatically splited joint '/'
for (( i = 0; i < ignoreDirListSize; i++))
do
	ignoreDirList+=(${ignoreDirList[i]}'/')
done

ignoreDirList+=('.')
ignoreDirList+=('..')
ignoreDirListSize=${#ignoreDirList[*]}

######################################### customer config area end ################################################

traverse_dir()
{
	local filepath=$1
	local validDirName=false
	local count=0

	for file in `ls $filepath`
	do
		if [ -d $filepath/$file ] # if $filepath/$file is a directory
		then
			count=0
            for (( i = 0; i < ignoreDirListSize; i++ ))
			do
				if [[ $file != ${ignoreDirList[i]} ]] # charge by Regular expressions
				then
					let count++
				fi
			done

			if [[ $count = $ignoreDirListSize && $filepath/$file != $dst ]]
			then
				echo 'copy files in directory '$file
				traverse_dir $filepath/$file
			fi
			count=0
		else # if $filepath/$file is a file
			check_suffix $filepath/$file
		fi
	done
}

check_suffix()
{
	local file=$1
	local validFileType=false;

	for (( i = 0; i < copyFileListSize; i++ ))
	do
		if [ "${file##*.}"x = ${copyFileList[i]}x ]
		then
			validFileType=true;
		fi
	done

	if [ $validFileType = true ]
	then
		# Dest directory, keep the relative path, use regex expression to replace string
		local tmpDst=${file/$src/$dst}
		# Create the path when it doesn't exists
		create_dir $tmpDst
		# Copy file to dest directory
		cp $file $tmpDst
		#echo "copy file: "$file
	fi
}

create_dir()
{
	local dir=$1

	if [ -d $dir ]
	then
		echo 'mkdir -p:' $dir
		mkdir -p $dir
	else
		# If `dir` is not a directory, then create its parent directory
		local tmpDir=`dirname $dir`
		#echo 'mkdir', $tmpDir
		mkdir -p $tmpDir
	fi
}

echo '************ start copy files *************'
echo 'src filePath: '$src
echo 'dst filePath: '$dst
# Source directory
traverse_dir $src
echo '************* copy finished ***************'
