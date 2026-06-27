#!/bin/bash

set -e

show_progress() 
{
    local current=$1
    local total=$2
    local percent=$(( current * 100 / total ))
    local filled=$(( percent * 20 / 100 ))
    local empty=$(( 20 - filled ))

    local bar_filled=$(printf "%${filled}s" | tr ' ' '#')
    local bar_empty=$(printf "%${empty}s" | tr ' ' '-')

    echo -e "\r[${bar_filled}${bar_empty}] ${percent}% ($3)"
}

TOTAL_STEPS=5

show_progress 1 $TOTAL_STEPS "[+] Compiling Evaluate.cpp..."
g++ -std=c++20 -O2 Evaluate.cpp -o evaluate_hash.out

show_progress 2 $TOTAL_STEPS "[+] Running script..."
./evaluate_hash.out

show_progress 3 $TOTAL_STEPS "[+] Showing data heads:"

ls -lh *.data

echo "-- New_Steps"
head -n 5 New_Steps.data
echo "..."
head -n 5 New_Steps.data

echo "-- Old_Steps"
head -n 5 Old_Steps.data
echo "..."
tail -n 5 Old_Steps.data

echo "-- New_Time"
head -n 5 New_Time.data
echo "..."
tail -n 5 New_Time.data

echo "-- Old_Time"
head -n 5 Old_Time.data
echo "..."
tail -n 5 Old_Time.data


show_progress 4 $TOTAL_STEPS "[+] Plotting results..."
gnuplot script.p

show_progress 5 $TOTAL_STEPS "[+] Cleaning up data..."

rm evaluate_hash.out
rm -rf New_Steps.data New_Time.data Old_Steps.data Old_Time.data