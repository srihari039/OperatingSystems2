CS19BTECH11039 - SRI HARI

Executing the C source file:

	1. Given the C source code files are in the zip file, named as Asgn1_cs19btech11039_mth1.c and Asgn1_cs19btech11039_mth2.c
	2. now we need to compile it using compiler
	3. we can use gcc , g++ or cc compilers (as cc is also a compiler linked to gcc in Linux in addition with g++)
	4. as this is C file, we compile it in gcc(remaining also can be used).
	5. the compilation is to be done with "-o" and "-lpthread" flag as we are dealing with threads and with "-lm" as we deal with inbuilt power function in the program.
		gcc Asgn1_cs19btech11039_mth1.c -lm -o <executablefilename> -lpthread
		gcc Asgn1_cs19btech11039_mth2.c -lm -o <executablefilename> -lpthread
	6. now the compilation is done and the executable file is created with the given name.
	7. now running the executable file
		as the input file is named "inp.txt" it is hardcoded in the code to take input from the file inp.txt, so no need of passing any arguments.
		let exec be the executable file name, then
		./exec gives you the output.txt file
	8. finally the output appears in output.txt
		It contains an array before sorting, the very same array after sorting and the time elapsed while the program sorts the array.
