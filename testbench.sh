#!bin/bash
<< 'MULTILINE-COMMENT'
      Image convolution test bench
      Written by David Cantu
      November 2020
      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.
 
      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.
 
      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <https://www.gnu.org/licenses/>.
MULTILINE-COMMENT

echo "Total arguments passed: " $#
echo "All arguments: " $@

if [ $# -ne 7 ]; then
	echo "Incorrect number of arguments"
	exit 0
fi

echo "Number of iterations used for validation: " $1
echo "Base name for image: " $2
echo "Image count: " $3
echo "Filter base name: " $4
echo "Program name: " $5
echo "Resulting images basename: " $6
echo "Max number of threads tested: " $7
iter=$1
img_basename=$2
imgcount=$3
filt=$4
prog=$5
resimg=$6
maxthreads=$7

filename_1="${iter}_${img_basename}_${imgcount}_${filt}_${resimg}.csv"
filename_2=`date "+%F-%T"`
filename="$filename_2$filename_1"
echo "Filename: ${filename}"

echo "Date, Imgpath, Filterpath, Filtsize, Threadcount, Time" > $filename

# Need to define, not as arguments

itot=1
((ibig=maxthreads*imgcount*iter*3))
# Iterations for thread count
for ((o = 2; o <= $maxthreads; o++))
do
	# Iterations for filter size
	for ((k = 3; k <= 7; k+= 2))
	do
		# Iterations for images
		for ((j = 1; j <= $imgcount; j++))
		do
			# Iterations for validation
			for ((i = 1; i <= $iter; i++))
			do
				imagename="${img_basename}_${j}.png"
				filtername="${filt}_${k}.txt"
				resname="${resimg}_${filt}${i}.jpg"
				res=`/usr/lib64/openmpi/1.4-gcc/bin/mpirun -np $o -mca btl ^openib $prog $imagename $filtername $resname $k`
				echo "Executing iteration (${itot}/${ibig}) ${i} for image ${j}, filter size ${k}, thead count ${o}"
				echo ${res} >> $filename 
				((itot=itot+1))
			done 
		done
	done
done
