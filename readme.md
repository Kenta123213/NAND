# PROJECT 1: NAND Gates
This project is an implementation of dynamically loadable C library for creating and managing Boolean circuits composed of NAND gates.

## Original task
[Original task in Polish](./task_description.pdf)

## Full task description

The task is to implement a dynamically loadable library in C that handles combinational Boolean circuits composed of NAND gates. A NAND gate has a non-negative integer number of inputs and one output. A NAND gate with no inputs always outputs a false signal. A NAND gate with one input is a negation. For a positive 
𝑛
n, the inputs of an 
𝑛
n-input gate are numbered from 0 to 
𝑛
−
1
n−1. A Boolean signal that can be either false or true can be input to the gate. The gate outputs a false signal if all its inputs are true, and true otherwise. The output signal from the gate can be connected to multiple inputs of other gates. Only one signal source can be connected to an input of a gate.

### Library Interface
The library interface is provided in the attached file nand.h, which contains the following declarations. Additional details about the library's operation can be inferred from the attached file nand_example.c, which is an integral part of the specification.

```c
typedef struct nand nand_t;
This is the name of the structure type representing a NAND gate. This type needs to be defined (implemented) as part of this task.
```

```c
nand_t * nand_new(unsigned n);
```
The function nand_new creates a new NAND gate with 
𝑛
n inputs. The result of the function is:

- a pointer to the structure representing the NAND gate;
- NULL if there was a memory allocation error; in this case, the function sets errno to ENOMEM.
```c
void nand_delete(nand_t *g);
```
The function nand_delete disconnects the input and output signals of the specified gate and then removes the specified gate, freeing all the memory it was using. It does nothing if called with a NULL pointer. After executing this function, the passed pointer becomes invalid.

Function parameter:

- g – a pointer to the structure representing the NAND gate.
```c
int nand_connect_nand(nand_t *g_out, nand_t *g_in, unsigned k);
```
The function nand_connect_nand connects the output of gate g_out to the 
𝑘
k-th input of gate g_in, possibly disconnecting the signal that was previously connected to that input.

Function parameters:

- g_out – a pointer to the structure representing the NAND gate;
- g_in – a pointer to the structure representing the NAND gate;
- k – the index of the input of gate g_in.
Result of the function:

- 0 if the operation was successful;
- -1 if any pointer is NULL, parameter 
𝑘 has an invalid value, or there was a memory allocation error; in this case, the function sets errno to either EINVAL or ENOMEM.
```c
int nand_connect_signal(bool const *s, nand_t *g, unsigned k);
```
The function nand_connect_signal connects the Boolean signal s to the 
𝑘
k-th input of gate g, possibly disconnecting the signal that was previously connected to that input.

Function parameters:

- s – a pointer to a variable of type bool;
- g – a pointer to the structure representing the NAND gate;
- k – the index of the input of gate g.
Result of the function:

- 0 if the operation was successful;
- -1 if any pointer is NULL, parameter 
𝑘 has an invalid value, or there was a memory allocation error; in this case, the function sets errno to either EINVAL or ENOMEM.
```c
ssize_t nand_evaluate(nand_t **g, bool *s, size_t m);
```
The function nand_evaluate determines the output signal values of the given gates and calculates the length of the critical path.

The length of the critical path for a Boolean signal and for a gate with no inputs is zero. The length of the critical path at the output of a gate is 
1
+
max
⁡
(
𝑆
0
,
𝑆
1
,
𝑆
2
,
…
,
𝑆
𝑛
−
1
)
1+max(S 
0
​
 ,S 
1
​
 ,S 
2
​
 ,…,S 
n−1
​
 ), where 
𝑆
𝑖
S 
i
​
  is the length of the critical path at its 
𝑖
i-th input. The length of the critical path of the gate circuit is the maximum of the lengths of the critical paths at the outputs of all the provided gates.

Function parameters:

- g – a pointer to an array of pointers to structures representing NAND gates;
- s – a pointer to an array where the determined output signal values should be placed;
- m – the size of the arrays pointed to by g and s.
Result of the function:

- the length of the critical path if the operation was successful; the array s then contains the determined output signal values of the gates; s[i] contains the value of the signal at the output of the gate pointed to by g[i];
- -1 if any pointer is NULL, 
𝑚 is zero, the operation failed, or memory allocation failed; in this case, the function sets errno to either EINVAL, ECANCELED, or ENOMEM, and the contents of the array s are undefined.
```c
ssize_t nand_fan_out(nand_t const *g);
```
The function nand_fan_out determines the number of gates connected to the output of a given gate.

Function parameter:

- g – a pointer to the structure representing the NAND gate.
Result of the function:

- the number of gates connected to the output of the given gate if the operation was successful;
- -1 if the pointer is NULL; in this case, the function sets errno to EINVAL.
```c
void* nand_input(nand_t const *g, unsigned k);
```
The function nand_input returns a pointer to the Boolean signal or gate connected to the 
𝑘-th input of the gate pointed to by g, or NULL if nothing is connected to that input.

Function parameters:

- g – a pointer to the structure representing the NAND gate;
- k – the index of the input.
Result of the function:

- a pointer of type bool* or nand_t*;
- NULL if nothing is connected to the given input; in this case, the function sets errno to 0;
- NULL if the pointer g is NULL or the value of 
𝑘 is invalid; in this case, the function sets errno to EINVAL.
```c
nand_t* nand_output(nand_t const *g, ssize_t k);
```
The function nand_output allows iterating over the gates connected to the output of the given gate. The result of this function is undefined if its parameters are invalid. If the output of gate g is connected to multiple inputs of the same gate, that gate appears multiple times when iterating.

Function parameters:

- g – a pointer to the structure representing the NAND gate;
- k – an index in the range from zero to one less than the value returned by the function nand_fan_out.
Result of the function:

- a pointer of type nand_t* to the gate connected to the output of gate g.
### Functional Requirements
Determining the output signal value and critical path length at the output of a gate requires recursively determining these values at its inputs (although a recursive implementation is not required), unless the gate has no inputs. Determining these values may fail if no signal is connected to one of the inputs, if the procedure loops (the gates do not form a combinational circuit), or if a memory allocation error occurs.

Care must be taken to ensure that the output signal value and critical path length are determined only once.

### Formal Requirements
As a solution to the task, you should submit a zip archive containing the file nand.c and optionally other *.h and *.c files with the library implementation, as well as a makefile or Makefile. The archive should not contain other files or subdirectories, and in particular, it should not contain binary files. The archive should be compressed using the zip, 7z, or rar programs, or a pair of tar and gzip programs. After extracting the archive, all files should be located in a common subdirectory.

The provided makefile or Makefile should include a target libnand.so, such that the command make libnand.so compiles the library and creates a file libnand.so in the current directory. This command should also compile and link the attached file memory_tests.c with the library. Dependencies between files should be described, and only files that have been modified or files that depend on them should be compiled. The invocation of make clean should remove all files created by the make command. The makefile or Makefile should include the .PHONY pseudotarget. It may also include other targets, such as a target for compiling and linking with the example usage library provided in the attached file nand_example.c, or a target for running tests.

The gcc compiler should be used for compilation. The library should compile in a computer lab under Linux. Files with the library implementation should be compiled with the following options:

```c
-Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2
```
Files with the library implementation should be linked with the following options:

```css
-shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl
```

## Attachments

The following files are attached to the task description:

- **memory_tests.c** – implementation of a library module for testing the reaction of the implementation to memory allocation failure;
- **memory_tests.h** – declaration of the interface of the library module for testing the reaction of the implementation to memory allocation failure;
- **nand_example.c** – example tests for the library;
- **nand.h** – declaration of the library interface.


## Testing
To run example tests:
```bash
export LD_LIBRARY_PATH=./
make
./test.sh
make clean
