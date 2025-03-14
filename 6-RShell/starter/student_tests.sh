#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}


#!/usr/bin/env bats

# -----------------------------------------------------------------------------
# 1) Local Shell Tests
# -----------------------------------------------------------------------------

@test "Local: Built-in cd then pwd" {
  # We feed commands to ./dsh (local mode) via stdin.
  # Use a heredoc to send multiple commands, then 'exit' to terminate.
  run bash -c 'cat <<EOF | ./dsh
pwd
cd ..
pwd
exit
EOF'
  
  # Check exit status
  [ "$status" -eq 0 ]

  # Check output for expected behavior.
  # For example, after the first `pwd`, it should show the current dir
  # after the second, a parent directory. This is a minimal check:
  [[ "$output" =~ "/" ]]  # we see at least a slash from 'pwd'
}



@test "Local: Simple pipeline" {
  run bash -c 'cat <<EOF | ./dsh
ls | grep dsh
exit
EOF'
  [ "$status" -eq 0 ]
  # Check that we see at least "dsh_cli.c" or something that matches 'dsh'
  [[ "$output" =~ "dsh_" ]] 
}


# -----------------------------------------------------------------------------
# 2) Optional Remote Shell Tests
# -----------------------------------------------------------------------------
# These only work if you can launch the server in one process
# and connect to it in another. This is more advanced because
# you typically need two shells or background the server. 
#
# Below is a minimal example where we:
#   1) Start the server in the background on port 9999.
#   2) Sleep briefly to ensure it's up.
#   3) Connect as a client, run commands, exit.
#   4) Kill the server process (if still running).
#
# Adjust as needed for your environment. This is just a demonstration.
# -----------------------------------------------------------------------------

@test "Remote: Single command (ls)" {
  # Start server in background on port 9999
  ./dsh -s -p 9999 &
  SVR_PID=$!
  sleep 1  # wait a moment for the server to bind

  # Now run a client that does `ls` then `exit`
  run bash -c 'cat <<EOF | ./dsh -c -p 9999
ls
exit
EOF'
  # The client might show the server's directory listing
  [ "$status" -eq 0 ]
  [[ "$output" =~ "dsh_cli.c" || "$output" =~ "rsh_server.c" ]]

  # Kill server just in case it's still running (or use "stop-server" if you implemented it)
  kill $SVR_PID 2>/dev/null
}


