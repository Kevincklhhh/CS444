 #!/bin/bash

shell=sshell
shell_source=$1

./script-a.sh $shell_source
./script-b.sh $shell_source
./script-cd.sh $shell_source
