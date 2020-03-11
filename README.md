# Parallel calculation of the 2-variable integral
![alt text](http://www.geatbx.com/docu/fcnindex-msh_f6_5-14.gif) The program is written with C++ using std::thread API for parallel computing. It is created to provide a way for calculation of 2-variable real function integral in the given domain (rectangular area) and with a given precision. The boost for productivity is obtained by using parallel computing techniques, more precisely - threading.
To run C++ program write in terminal:
`$ ./lab02_self path_to_config_file`
Program requires config file in the specific format:
* absolute error of integral estimation
* relative error of integral estimation
* amount of threads program is to use
* x coordinates for the funciton domain
* y coordinates for the funciton domain
