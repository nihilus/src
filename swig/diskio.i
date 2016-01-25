// TODO: These could be wrapped
%ignore enumerate_files;
%rename (enumerate_files) py_enumerate_files;
%ignore enumerate_system_files;
%ignore enumerate_sorted_files;
%ignore ioport_bit_t;
%ignore ioport_bits_t;
%ignore ioport_t;
%ignore read_ioports;
%ignore choose_ioport_device;
%ignore find_ioport;
%ignore find_ioport_bit;
%ignore free_ioports;
%ignore lread;
%ignore qlread;
%ignore efilelength;
%ignore qlgets;
%ignore qlgetc;
%ignore lreadbytes;
%ignore lread2bytes;
%ignore lread2bytes;
%ignore lread4bytes;
%ignore lread4bytes;
%ignore lread8bytes;
%ignore lread8bytes;
%ignore qlsize;
%ignore qlseek;
%ignore qltell;
%ignore qlfile;
%ignore make_linput;
%ignore unmake_linput;
%ignore create_remote_linput;
%ignore make_filehandle_linput;
%ignore get_default_userdir;

// FIXME: These should be wrapped for completeness
%ignore eread;
%ignore ewrite;

// Ignore kernel-only & unexported symbols
%ignore get_thread_priority;
%ignore set_thread_priority;
%ignore checkdspace;
%ignore lowdiskgo;
%ignore ida_argv;
%ignore exename;

%include "diskio.hpp"

%{
//<code(py_diskio)>
//</code(py_diskio)>
%}

%inline %{
//<inline(py_diskio)>
//</inline(py_diskio)>
%}

%pythoncode %{
#<pycode(py_diskio)>
#</pycode(py_diskio)>
%}
