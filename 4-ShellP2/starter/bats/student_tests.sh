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
    [[ "$output" =~ "Command not found" ]] || [[ "$output" =~ "execution failure" ]]
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

@test "Dragon command outputs art" {
    run ./dsh <<EOF
dragon
exit
EOF
    [ -n "$output" ]
}
