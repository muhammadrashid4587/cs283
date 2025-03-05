#!/usr/bin/env bats

@test "Check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF
    [ "$status" -eq 0 ]
}

@test "Invalid command returns error" {
    run ./dsh <<EOF
foobar
exit
EOF
    [[ "$output" =~ "CommandnotfoundinPATH" ]] || [[ "$output" =~ "executionfailure" ]]
}

@test "CD to valid directory and verify pwd" {
    start_dir=$(pwd)
    run ./dsh <<EOF
cd /tmp
pwd
exit
EOF
    [[ "$output" =~ "/tmp" ]]
    [ "$status" -eq 0 ]
}

@test "CD with no args does nothing" {
    current=$(pwd)
    run ./dsh <<EOF
cd
pwd
exit
EOF
    [[ "$output" =~ "$current" ]]
    [ "$status" -eq 0 ]
}

@test "RC command returns last exit code" {
    run ./dsh <<EOF
foobar
rc
exit
EOF
    [[ "$output" =~ "2" ]]  # assuming ENOENT returns 2 on your system
    [ "$status" -eq 0 ]
}

@test "Dragon command outputs art" {
    run ./dsh <<EOF
dragon
exit
EOF
    [ -n "$output" ]
}

@test "Piped commands work" {
    run ./dsh <<EOF
ls | grep ".c"
exit
EOF
    [[ "$output" =~ ".c" ]]
    [ "$status" -eq 0 ]
}

@test "Redirection overwrites output" {
    run ./dsh <<EOF
echo "hello, class" > out.txt
cat out.txt
exit
EOF
    [[ "$output" =~ "hello, class" ]]
    [ "$status" -eq 0 ]
}

@test "Redirection append works" {
    run ./dsh <<EOF
echo "line1" > out.txt
echo "line2" >> out.txt
cat out.txt
exit
EOF
    [[ "$output" =~ "line1" ]] && [[ "$output" =~ "line2" ]]
    [ "$status" -eq 0 ]
}

@test "Combined pipes and redirection" {
    run ./dsh <<EOF
echo "hello world" > temp.txt
cat temp.txt | grep "hello" > out.txt
cat out.txt
exit
EOF
    [[ "$output" =~ "hello world" ]]
    [ "$status" -eq 0 ]
}
