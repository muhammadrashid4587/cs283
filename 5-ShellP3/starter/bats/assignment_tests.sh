#!/usr/bin/env bats

############################ DO NOT EDIT THIS FILE #####################################
# File: assignment_tests.sh
#
# DO NOT EDIT THIS FILE
#
# Add/Edit Student tests in student_tests.sh
#
# All tests in this file must pass - it is used as part of grading!
########################################################################################

@test "Which which ... which?" {
    run "./dsh" <<EOF                
which which
EOF

    # Strip all whitespace from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/usr/bin/whichdsh3>dsh3>cmdloopreturned0"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$stripped_output" = "$expected_output" ]
}

@test "It handles quoted spaces" {
    run "./dsh" <<EOF                
echo " hello     world     " 
EOF

    # Strip all whitespace from the output
    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')
    expected_output=" hello     world     dsh3>dsh3>cmdloopreturned0"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$stripped_output" = "$expected_output" ]
}
