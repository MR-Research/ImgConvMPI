# Imgconv MPI

Parallel image convolution tool written in C using the Message Passing Interface (MPI) library.

## Features

 - Wide image format compatibility.
 - Arbitrary image and filter size.
 - Low computational complexity.
 - Grayscale conversion.
 - No external dependencies.
 - Parallel execution.
 - Distributed memory architecutre.
 - Multi-computing node support.
 - Dynamic memory management.

## Installation

 - Clone the repository.
```bash
git clone https://github.com/MrRobot-oss/ImgConv_OMP.git
```
 - Compile using MPI compiler, math library and openmp tags.
```bash
mpicc -o imgconv imgconv.c -O2 -m64 -lm -lc -fopenmp
```
 - Copy the binary to your standard binary path.

## Usage

```bash
mpirun -np process_count -mca btl ^openib imgconv image_path filter_path resulting_image_path filter_size
```
## Parameters
 1. process_count: Amount of workers (processess) used for execution
 2. image_path: The source image, supported formats are the same as stb image (JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC).
 3. filter_path: Filter file path, should be a plain text file, where each element is separated by a space, and each row is separated by newline (\n).
 4. resulting_image_path:  Output image file path, the image should be in jpg format.
 5. filter_size: Size of the filter, should be an odd number.

## Limitations
The following are the current limitations of the program:
 - The program by default crops the resulting image, to account for the data loss due to the convolution process.
 - Grayscale conversion is performed by default.
 - Supported color channels is 3.
 - Output image is jpg.
 - Only odd number square filters are supported at the moment.

## Known issues
No known issues have been reported so far. If you encounter an issue, please post a new issue so it can be verified and fixed.

## Automated test bench
In order to test the execution of the algorithm by using different parameters, execute the script called ```bash testbench.sh niter img imcount filt prog resimg maxth```, the input parameters are as follows:
 - niter: Number of iterations used for validation
 - img: Base name for image
 - imcount: Image count
 - filt: Filter base name
 - prog: Program name
 - resimg: Resulting images basename
 - maxth: Max number of threads tested

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.