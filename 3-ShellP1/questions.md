1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  _`fgets()` is line-based and stops reading after a newline or EOF, which aligns well with reading one command per line. It also prevents buffer overflows by respecting the buffer size we provide. This makes it safer than older functions like `gets()` and more convenient than using lower-level functions to read arbitrary amounts of data._

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?å

    > **Answer**:  _We might want flexibility in how large commands can be, or we may not know in advance the size needed. A fixed-size array risks running out of space or wasting memory. Using `malloc()` lets us dynamically allocate (and potentially reallocate) memory to fit the actual size of the commands._


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  _Trimming removes accidental whitespace that can create empty or invalid tokens. If we don’t trim, we might store commands like `"   ls  "` with extra spaces, which could lead to parsing errors, empty commands, or issues matching the correct executable name._

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  - **`command > file`**: Redirects STDOUT of `command` to `file`, overwriting it.  
    > - **`command >> file`**: Redirects STDOUT of `command` to `file`, appending rather than overwriting.  
    > - **`command < file`**: Takes input from `file` instead of the keyboard.  
    >
    > Challenges include correctly manipulating file descriptors, verifying file write/read permissions, and handling any combination of input and output redirections together (e.g. `command < file1 > file2`).

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:   _Redirection attaches a command’s input/output to a file (or device). Piping (`|`) attaches the output of one command directly to the input of another command, with no file in between. Redirection is command-to-file, while piping is command-to-command._

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  _By separating error output (STDERR) from normal output (STDOUT), we let users and scripts handle errors differently. For example, we can redirect only errors to a log without interfering with normal output, or vice versa. This separation also avoids mixing error text with valid data._

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  _Typically, the shell prints STDERR messages as they come, separate from STDOUT. We should let users decide if they want to merge them (e.g., with `2>&1`). Internally, that means handling separate file descriptors. If a command fails, our shell should detect the non-zero exit status and potentially print an error message or store that status for later (like in `$?` in other shells)._