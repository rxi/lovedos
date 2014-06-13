# Building LoveDOS
Although LoveDOS provides precompiled binaries on the
[releases page](https://github.com/rxi/lovedos/releases), you may still wish to
build the project's source if you want to make changes to the project.


## Requirements
LoveDOS depends on the following being installed before building:
* **[Python2.7](https://www.python.org/)** is required by build.py, the build
  script
* **[DJGPP cross compiler](https://github.com/andrewwutw/build-djgpp)** 
  is required to compile the source files


## Building
To compile you should clone the git repository or
[download the .zip](https://github.com/rxi/lovedos/archive/master.zip) of it.
Once this is done you should open the build.py file in an editor and check to
make sure the COMPILER variable is set to the correct command as to run DJGPP's
gcc executable; change the COMPILER variable's value if it is not set to the
correct value.

Assuming the COMPILER variable is correctly set the script should be run:
```
./build.py
```
 The script will output the following line when it starts:
```
compiling...
```
Within a minute the script should finish and display the following line:
```
done
```
There should now be a file named "love.exe" in the "bin/" directory
