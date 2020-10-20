# sudo apt-get install gawk
#timeout 5s ./get_mem_usage.sh test_ram

if [ "$1" != "" ]; then
    echo "Filename: " $1 
else
    usage
    echo ""
    echo "Please introduce the name of the file on which to save the mem usage"
    exit 1
fi

while true; do
    free -m | grep "Mem" | awk '{print strftime("%F %T") "\t" $0}' >> $1.txt
    sleep 1
done
