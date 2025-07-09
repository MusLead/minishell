[31m00-basic-fork-exec.t
[31m01-commands-with-parameters.t
[31m02-commands-invoked-from-path.t
[31m03-cd-changes-directory.t
[31m04-ret-displays-exitstatus.t
[31m05-semicolon-separates-commands.t
[31m06-piped-commands.t
[32m07-exit-shell.t
[37m---------------------------
Final result:

# Invoke a command by its fully qualifiey name, without parameters
#
→ /bin/uname⏎
↵ Linux
→ /bin/wc⏎
→ foo bar baz⏎
→ ^D
↵       1       3      12
# Invoke a command by its fully qualified name, with parameters 
#
→ /bin/wc -c⏎
→ foo bar baz⏎
→ ^D
↵ 12
→ /bin/tr a-z n-za-m⏎
→ foo bar baz⏎
↵ sbb one onm
→ ^D
# Commands invoked from $PATH
#
→ uname⏎
↵ Linux
→ date +%Y⏎
↵ 2025
→ tr a-z n-za-m⏎
→ foo bar baz⏎
↵ sbb one onm
→ ^D
# Builtin command cd changes directory
#
→ cd /bin⏎
→ pwd⏎
≠ /tmp\n
→ cd /tmp⏎
→ cd ..⏎
→ pwd⏎
↵ /\n
→ true⏎
→ ret⏎
↵ 0
→ false⏎
→ ret⏎
↵ 1
# Invoke multiple commands separated by semicolon (;)
#
→ echo -n foo; echo -n bar; echo baz⏎
↵ foobarbaz
→ cd /tmp; pwd⏎
↵ /tmp
# Two commands chained by pipe (|)
#
→ false | wc⏎
↵       0       0       0
→ date +%s |wc -c⏎
↵ 11
→ true | wc; date +%s |wc -c⏎
↵ 11
Keep working!
