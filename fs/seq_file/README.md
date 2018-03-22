# Comments of seq_file interface

Initially a aux tool for create /proc file which is larger then a page. It mainly simplify the work for dealing with the reader's postion. A summary of its features:
* Provides a iterator interface which lets virtual file implementation step through the object it's presenting.
* Some utility functions for formatting objects for output
* A set of canned file_operations which implemented most operations on the virtual file.

## How seq_file works
Seq_file provides operaitons: 
```
struct seq_operations {
	void * (*start) (struct seq_file *m, loff_t *pos);
	void (*stop) (struct seq_file *m, void *v);
	void * (*next) (struct seq_file *m, void *v, loff_t *pos);
	int (*show) (struct seq_file *m, void *v);
};
```
This is how it interactive with fs:
```
/* tailed-code */
struct seq_file *m = file->private_data;
loff_t_t pos;
void *p;

p = m->op->start(m, &pos);
while (1) {
  m->op->show(m, p);
  ...
  if (!m->count) {
    p = m->op->next(m, p, &pos);
    m->index = pos;
    continue;
  }
  ...
  m->op->stop(m, p);
  ...
}
```
So:
* `pos` is used by seq_file to maitain the reading postion and should be controlled variable.
* `start` method return the first object and provides a hook to do something before formmaly read.
* `next` method recv input of previous object and return the next object.
* `show` method recv the current object.



> Reference
* https://www.cnblogs.com/hoys/archive/2011/04/10/2011261.html
* https://lwn.net/Articles/22355/
