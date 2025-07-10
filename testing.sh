#!/bin/bash

# Detect the OS type
OS_TYPE=$(uname)

if [[ "$OS_TYPE" == "Linux" ]]; then
    cd shtest
elif [[ "$OS_TYPE" == "Darwin" ]]; then
    cd shtest_mac
else
    echo "Unsupported OS: $OS_TYPE"
    exit 1
fi

# Compile the timeout helper if needed
gcc helpers/timeout.c -o helpers/timeout

# Make sure it’s executable
chmod +x helpers/timeout

# Compile your minishell and run the tests
gcc ../testat_Agha_Aslam.c -o ../minishell && ./shtest ../minishell
