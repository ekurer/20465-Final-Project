This is the final task in the C course of the Open University, by @RoyGonen (https://tinyurl.com/3d3umkmf) any myself.

This project is based on the double-pass assembler model.  

The task is to build an assembler that will:
1. receive assembly files (.as)
2. interpret the files according to given rules
3. output the interpretation, in three files: 


      ob. - memory image and instructions (hex)


      ent. - symbol table


      ext. - list of external values


The project was coded and compiled using Ubuntu, but it may run on all Linux versions.

Use makefile to compile the project like this:
```
>   make
```
After preparing assembly files **with an `.as` extension**, open *terminal* and pass file names as arguments (without the file extensions) as following:

As for the files x.as, y.as, hello.as we will run:
```
>   assembler x y hello
```
The assembler will generate output files with the same filenames and the following extensions:  
- `.ob` - Object file
- `.ent` - Entries file
- `.ext` - Externals file

Examples of input and output files can be found under the 'tests' folder.
