# Two commands chained by pipe (|)
#
→ false | wc⏎
↵       0       0       0
→ date +%s |wc -c⏎
↵       11
→ true | wc; date +%s |wc -c⏎
↵       11
