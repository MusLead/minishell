#!/usr/bin/env expect
#
# This script runs the specified shell in a temporary directory, and
# executes the .t test specifications.
#
# The implementation of getting expected values back from the shell is
# not terribly robust, but was a lot simpler, as a first stab, to fit
# into expect's way of doing things.  I thought about either having a
# ptrace'ing wrapper around the shell that watches for forks and so
# on, or more wrapper commands in the spirit of those already present,
# but maybe once someone complains.
#
# Even a ptrace approach that spits out child exit statuses can get
# confused by things like prompts that execute commands.  So maybe
# we'd better keep it simple.

# symbols:
# ⏎ ⇒ newline
# ☠ ⇒ shell itself exits, with code
# → ⇒ user input
# ← ⇒ response on stdout
# ↵ ⇒ response on stdout delimited by a CR or LF
# ≠ ⇒ anything but this on stdout (at least one line)
# ✓ ⇒ expect zero exit status of previous command (not implemented)
# ✗ ⇒ expect nonzero exit status of previous command (not implemented)
# ⌛ ⇒ wait a little extra (0.5 seconds)

# --- Helpers ---
proc expand {s} {
    string map {
        ⏎ \r
        ⇑ \x1b\x5bA
        \\n \r\n
        ^C \x03
        ^D \x04
        ^I \x09
        ^R \x12
        ^Z \x1a
        ^\\ \x1c
    } $s
}

proc 1+ {x} { expr 1 + $x }
proc rest-of {s} {
    set i [1+ [string first { } $s]]
    if {-1 == $i} { return {} }
    string range $s $i end
}

proc cleanup {} {
    global temp_dir
    file delete -force $temp_dir
}

proc strip_spaces {s} {
    # Remove leading/trailing whitespace and compress internal whitespace to single spaces
    set cleaned [regsub -all {\s+} [string trim $s] { }]
    return $cleaned
}

# This assumes this script is in helpers/
set script_path [file dirname [file normalize [info script]]]

proc setup_execution_environment {} {
    global script_path temp_dir
    set temp_dir [exec mktemp -q -d -t "shell-workshop.XXXXXX"]
    exit -onexit cleanup
    cd $temp_dir
    file link -symbolic helpers $script_path
    set ::env(PATH) [join [list /bin /usr/bin [join [list [pwd] helpers] /]] :]
    set ::env(KNOWN_VARIABLE) {reindeer flotilla}
}

proc wait_for_exit {} {
    expect {
        eof {}
        timeout {}
    }
    foreach {pid spawn_id is_os_error code} [wait] break
    if {0 == $is_os_error} { return $code }
    not_ok
}

if {2 != [llength $argv]} {
    error "Arguments are the test description file, and the path to your shell."
}

set emit_tap 0
set test_path [lindex $argv 0]
set shell [lindex $argv 1]
set test_file [open $test_path]
fconfigure $test_file -encoding utf-8

# Count tests
set n_tests 0
while {![eof $test_file]} {
    switch -re [string index [gets $test_file] 0] {
        ←|↵|↵~|≠|✓|✗|☠ {incr n_tests}
        default {}
    }
}
seek $test_file 0 start

log_user 0
setup_execution_environment
if [catch {spawn $shell} err] {error $err}
set timeout 3
expect -re .+
set timeout 2

set line_num 0
set test_num 1
proc ok {} {
    global test_num test_path line_num emit_tap
    if {1 == $emit_tap} {puts "ok $test_num # $test_path:$line_num"}
    incr test_num
}
proc not_ok {} {
    global test_num test_path line_num command emit_tap
    if {0 == $emit_tap} {
        puts "\x1b\[91m$command\x1b\[31m"
        puts "failed at $test_path:$line_num\x1b\[m"
    } else {
        puts "not ok $test_num # $test_path:$line_num"
    }
    incr test_num
    exit $test_num
}
proc is {x} { uplevel 1 [list if $x {ok} {not_ok}]}

proc re_quote {s} {
    string map {
        \\ \\\\
        ^ \\^
        \$ \\\$
        \[ \\\[
        \( \\\(
        . \\.
        + \\+
        * \\*
        ? \\?
        \{ \\\{
    } $s
}

proc expect_line {line} {
    set rv [expect {
        -re $line {return 1}
        default {return 0}
    }]
    expect *
    return $rv
}

proc expect_line_raw {pattern} {
    expect {
        -re $pattern {
            set matched $expect_out(0,string)
            return $matched
        }
        default {return {}}
    }
    expect *
}

proc careful_send {m} {
    if {[catch {send -- $m} err]} { error $err }
    sleep 0.01
}

if {1 == $emit_tap} {puts "1..$n_tests"}
while {![eof $test_file]} {
    gets $test_file command
    incr line_num
    if {0 == [string length [string trim $command]]} { continue }
    set line [expand [rest-of $command]]
    switch [string index $command 0] {
        → {expect *; careful_send $line}

        ↵~ {
            set actual [expect_line_raw "\[\r\n]$line"]
            set actual_cleaned [strip_spaces $actual]
            set expected_cleaned [strip_spaces $line]
            is {$actual_cleaned eq $expected_cleaned}
        }

        ↵ {
            set actual [expect_line_raw "\[\r\n][re_quote $line]"]
            set actual_cleaned [strip_spaces $actual]
            set expected_cleaned [strip_spaces $line]
            is {$actual_cleaned eq $expected_cleaned}
        }

        ← {is {1 == [expect_line [re_quote $line]]}}
        ≠ {is {0 == [expect_line "\[\r\n][re_quote $line]"]}}
        ✓ {error "sorry we decided not to do this $line_num"}
        ✗ {error "sorry we decided not to do this $line_num"}
        ⌛ {sleep 0.2}
        ☠ {
            is {$line eq [wait_for_exit]}
            if {$test_num <= $n_tests} {
                error "☠ must always be the last test"
            }
            exit 0
        }
        \# {}
        default {error "unexpected command $command"}
    }
    if {0 == $emit_tap} {puts $command}
}

sleep 0.1; send "\x04"
if {0 != [wait_for_exit]} {error "shell didn't exit cleanly"}