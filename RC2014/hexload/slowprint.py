import sys
from time import sleep

for line in sys.stdin:

    for ch in line:
        sys.stdout.write( ch );
        sys.stdout.flush();
        sleep(0.005);

    sys.stdout.write('\r');
    sys.stdout.write('\n');
    sleep(0.03);


