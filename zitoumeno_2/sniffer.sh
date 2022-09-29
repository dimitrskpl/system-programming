# !/bin/sh

args=("$@")
CNT_ARGS=${#args[@]} 

if [ "$CNT_ARGS" -eq 0 ]
then 
    echo "Too few arguments"
    exit 1
fi

start=0 #pos in args where tlds start
path="" #defeult path
if [ "$CNT_ARGS" -gt 1 ] && [ "$1" == "-p" ] #parametre -p and path are given 
then
    if [ "$CNT_ARGS" -lt 3 ] #if no tld is given
    then 
        echo "No tld is given in arguments"
        exit 1
    else
        path=$2
        start=2
    fi
fi

FILES="$path*.out" #check for files in path that end with '.out'
for ((i=start; i < CNT_ARGS; i++)) do #for each tld
    tld=${args[${i}]}
    result=$(grep "$tld "  $FILES) #save lines that containt tld in the end of location
    if [ "$?" -ne 0 ]
    then
        echo "Number of appearance of tld $tld: 0"
    else
        cnt_tokens=0 #pos in result for current string
        cnt_tld_appear=0 #no of appearance of tld
        for value in $result
        do
            check=$(($cnt_tokens%2)) #1 if its the location's frequency
            if [ "$check" -eq 1 ]
            then
                let cnt_tld_appear=cnt_tld_appear+$value
            fi
            let cnt_tokens=cnt_tokens+1
        done
        echo "Number of appearance of tld $tld: $cnt_tld_appear"
    fi
done
