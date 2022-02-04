# nRF24L01-on-C8051
nRF24L01 communication implementation on C8051F920

Call nRF_begin() for initialization \
Call 	nRF_setWritingPipe() and nRF_setReadingPipe() for setting reading and writing pipes \
In reading, call nRF_read() contiguously in superloop for retrieve data from the module and store to array, call nRF_read_pipe() to read data in a pipe(which was retrived and stored before by nRF_read()) \
In writing, call nRF_write() to send data to another module
