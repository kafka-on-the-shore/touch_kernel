# Comments of seq_file interface

Initially a aux tool for create /proc file which is larger then a page. It mainly simplify the work for dealing with the reader's postion. A summary of its features:
* Provides a iterator interface which lets virtual file implementation step through the object it's presenting.
* Some utility functions for formatting objects for output
* A set of canned file_operations which implemented most operations on the virtual file.


> Reference
* https://www.cnblogs.com/hoys/archive/2011/04/10/2011261.html
* https://lwn.net/Articles/22355/
