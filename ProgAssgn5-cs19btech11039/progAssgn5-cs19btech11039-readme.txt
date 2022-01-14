CS19BTECH11039 - SRI HARI

Executing the C++ source file:

	1. Given the C++ source code files are in the zip file
	2. now we need to compile it using compiler
	3. we need to use g++ for compiling
	4. The compilation is to be done with "-o" and "-lpthread" for getting executable with desired name.
		g++ SrcAssgn5_coarse_CS19BTECH11039.cpp -lpthread -o <executable>
		g++ SrcAssgn5_fine_CS19BTECH11039.cpp -lpthread -o <executable>
	5. now the compilation is done and the executable file is created with the given name.
	6. now running the executable file
		as the input file is named "input_params.txt", it is hardcoded in the code to take input from the file input_parameters.txt, so no need of passing any arguments.
		let exec be the executable file name, then
		./exec gives you all the files containing required data corresponding to the problem
	7. input format is as followed in the pdf
		2 5
		  1 2 3 4 5
		1 0 1 1 0 0
		2 1 0 1 1 0
		3 1 1 0 1 0
		4 0 1 1 0 1
		5 0 0 0 1 0
	8. if there is no source for producing input in this format, use the generator file attatched in the zip.
