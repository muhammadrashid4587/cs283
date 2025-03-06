1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

**Answer**:  
After creating each child process in the pipeline, the shell records the child’s PID. Once all the children have been forked, the shell calls `waitpid()` (or `wait()` in a loop) for each PID. This ensures that the shell blocks until each child has finished execution. If we neglected to call `waitpid()`, each child process would become a “zombie,” meaning it has terminated but still occupies a slot in the process table, using system resources and preventing the shell from cleaning up properly. Over time, this would lead to resource leaks, and eventually the system might run out of process slots.

---

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

**Answer**:  
When you `dup2()` one end of a pipe onto `STDIN_FILENO` or `STDOUT_FILENO`, you no longer need the original pipe file descriptors in that process. Leaving these descriptors open can cause confusion for the operating system about when the pipe is truly finished. For example, if the write-end of the pipe remains open in a process that doesn’t actually write, the reader might never see an EOF (end-of-file). This can lead to processes blocking indefinitely. Additionally, not closing unused pipes wastes file descriptors and can lead to a resource leak if many commands are processed.

---

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

**Answer**:  
`cd` needs to change the working directory of the current shell process itself. If `cd` were an external process run by `execvp()`, it would run in a separate child process, so any `chdir()` call would affect only that child’s working directory—not the parent shell’s. Upon returning to the shell, the working directory would remain unchanged. Implementing `cd` as a built-in allows the shell to call `chdir()` directly in its own process space, truly updating the shell’s working directory. If `cd` were external, you could never actually change the parent’s directory, making the command ineffective for its main purpose.

---

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

**Answer**:  
To allow an arbitrary number of piped commands, you could dynamically grow your array of commands instead of using a fixed-size array. For example, you could use a linked list or a resizable data structure (like `realloc()` on an array of command structs). The trade-off is the added complexity of managing dynamic memory: you’d need to carefully allocate, reallocate, and free memory when parsing or executing commands. Another consideration is performance overhead: while a dynamically growing structure can handle any number of commands, constantly resizing or scanning linked lists might degrade performance if you have extremely large pipelines. You’d balance simplicity (fixed-size arrays) against flexibility (dynamic memory) depending on your shell’s expected usage.
