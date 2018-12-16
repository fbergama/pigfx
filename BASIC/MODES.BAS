1 rem choose a mode and display '0' to '9' sequences to check number of lines and columns
5 F$="0": rem font 0 (8x8), 1 (8x16) or 2 (8x24)
10 INPUT "Mode ";M$
20 PRINT CHR$(27);"[=";M$;"h";
25 PRINT CHR$(27);"[=";F$;"f";
30 FOR I=0 TO 59:rem 60 lines
40 FOR J=0 TO 7:rem 8 times "0123456789" 
50 FOR K=48 TO 57:rem "0" to "9"
60 PRINT CHR$(K);
70 NEXT K
80 NEXT J
90 NEXT I
100 RUN