#!/bin/bash

echo "#!/bin/bash

modsecs=\$(date --utc --reference=\"\$1\" +%s)
nowsecs=\$(date +%s)
delta=\$((\$nowsecs-\$modsecs))
if [[ \$delta -ge \"$2\" && \$delta -le \"$3\" ]]
then
    exit 0;
else
    exit 1;
fi" > /tmp/good_age

chmod +x /tmp/good_age

find "$1" -print0 | ./filter -z -- /tmp/good_age
