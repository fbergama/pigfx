1 REM QUEST BY ROGER CHAFFEE2 REM INSPIRED BY WILL CROWTHER'S "ADVENTURE"3 REM COPYRIGHT(C) 1978, PENINSULA SCHOOL, MENLO PARK, CA4 REM                    PERMISSION TO USE, NOT TO SELL6 REM THE ORIGINAL VERSION OF THIS PROGRAM WAS WRITTEN ON A7 REM COMMODORE PET 2001. THIS VERSION HAS BEEN CONSIDERABLY8 REM RECODED , AND IS IN "PLAIN VANILLA" BASIC , WITH THE9 REM EXCEPTION OF THE RANDOM NUMBER GENERATOR IN LINES10 REM 6600-6800 , THE STRING MANIPULATION , AND THE IF ... THEN11 REM SPATEMENTS WHICH GIVE A STATEMENT TO PERFORM INSTEAD OF12 REM A STATEMENT NUMBER TO GOTO.13 REM VARIABLES USED14 REM    N     NODE (CAVE) NUMBER15 REM    M0    MOVE COUNTER16 REM    T     CURRENT LOCATION OF TREASURE (- 1 FOR CARRYING)17 REM    T1,T2 FIRST AND SECOND HIDING PLACES ( NODE NUMBERS)18 REM    M6    SAVES THE MOVE NUMBER WHEN HE SAID NO, HE DIDNT14 REM          WANT TO TAKE THE TREASURE WITH HIM.20 REM    M     MAP OF INTERCONNECTIONS.21 REM          M (I,J) IS NEXT NODE FROM NODE J, WHEN YOU GO22 REM          N,E,U,D,W,S FOR I=1,2,3,4,5,623 REM24 REM          BOUNCES25 REM          TO GO TO NODE -2 MEANS BOUNCE BACK TO THE NODE YOU26 REM          CAME FROM.27 REM28 REM          FORCED AND/OR RANDOM MOVES29 REM          M(1,N)=-2 MEANS A FORCED MOVE AS SOON AS YOU REACH30 REM          NODE N. IN THAT CASE, M(2,N) OF THE TIME YOU GO31 REM          TO NODE M(3,N). IF YOU DON'T GO THERE, M(4,N)32 REM          OF THE TIME YOU GO TO NODE (5,N), AND THE REST OF33 REM          THE TIME YOU GO TO NODE (6,N).34 REM35 REM          NODES > 100:36 REM          NODE N+100 MEANS NODE N IF YOU DON'T HAVE THE37 REM          TREASURE, AND NODE N+1 IF YOU DO.38 REM          NODE N+200 MEANS NODE N+1 IF YOU HAVE THE TREASURE39 REM          THE SECOND TIME, AND NODE N OTHERWISE.40 REM          NODE N+500 MEANS RUN THROUGH A DELAY LOOP AND THEN41 REM          GO TO NODE N. THIS IS USED FOR PALLING DOWN THE42 REM          EXIT CHUTE, AND YOU MAY WANT TO ADJUST THE DELAY43 REM          TIME (LINE 6250).44 REM45 REM    Q$  INPUT STRING46 REM    A$  CHARACTERS TO MATCH IN THE INPUT ROUTINE47 REM    A2  NUMBER OF CHARACTERS IN A$48 REM    Al  OUTPUT FROM THE INPUT ROUTINE49 REM    P   PIRATE FLAG 1 IF PIRATE HAS GOT YOU, 0 OTHERWISE50 REM    P1  COUNTER FOR PIRATE ROUTINE51 REM    N9  SAVES OLD NODE IN MOVE ROUTINE, FOR BOUNCE52 REM    N8  SAVES NODE WE BOUNCED FROM IN MOVE ROUTINE,53 REM        FOR PRINT FLAG54 REM    N0  SAVES OLD NODE IN MOVE ROUTINE, FOR DEAD END55 REM    A0  SAVES OLD DIRECTION IN MOVE ROUTINE56 REM    D   DEBUG FLAG (NON-ZERO TO PRINT)57 REM    I,J MISC. COUNTERS58 REM    W   TRAVEL FLAG, USED IN SCORING. W(I)=1 IF HE'S59 REM        BEEN TO NODE I, 0 OTHERWISE60 REM    S   SCORE61 REM    M9  MAXIMUM NUMBER OF NODES70 REM ---------------------------------------------------------71 REM ADAPTED FOR RC2014 BY DQ, FEB/201772 REM CODE COPIED FROM 73 REM https://archive.org/details/byte-magazine-1979-0774 REM OCR ERRORS SHOULD BE FIXED...80 D=0100 REM ---------------------------------------------------------110 REM GIVE 'EM SOMETHING TO READ WHILE I GET THE DATABASE SET UP120 PRINT "          QUEST"130 PRINT140 PRINT "YOU WERE WALKING THROUGH THE"150 PRINT "WOODS, AND YOU CAME ACROSS THE ENTRANCE"160 PRINT "OF A CAVE, COVERED WITH BRUSH."170 PRINT180 PRINT "PEOPLE SAY THAT MANY YEARS AGO A"190 PRINT "PIRATE HID HIS TREASURE IN THESE"200 PRINT "WOODS, BUT NO ONE HAS EVER FOUND IT."210 PRINT "IT MAY STILL BE HERE, FOR ALL I KNOW."400 READ M9,T1,T2490 REM DIMENSION OF W, M IS M9, IF YOU HAVE DYNAMIC ASSIGNMENT500 DIM W(42),M(6,42)510 REM READ MAP INTO M ARRAY520 FOR I=1 TO M9530 READ N540 IF I=N THEN 570550 PRINT "DATABASE PROBLEM"I,N560 STOP570 FOR J=1 TO 6580 READ M(J,I)590 NEXT J600 NEXT I900 PRINT905 PRINT "WHEN YOU ANSWER A QUESTION, I LOOK AT"906 PRINT "ONLY THE FIRST LETTER, ALTHOUGH YOU CAN"907 PRINT "TYPE THE WHOLE WORD IF YOU WANT."920 GOSUB 75001000 REM -------------------------------- ------------------------1010 N=51020 M0=01030 M6=01040 T=T11050 P=01060 P1=01070 FOR J=1 TO M91080 W(J) =01090 NEXT J1100 PRINT1110 REM DESCRIBE1120 GOSUB 80001400 REM ---------------------------------------------------------1405 REM *** MAIN LOOP STARTS HERE ***1410 REM COUNT MOVES1420 M0=M0+11430 REM MOVE1440 GOSUB 60001450 REM CHECK FOR FINDING THE TREASURE1460 GOSUB 20001470 REM TRY THE PIRATE1480 GOSUB 40001490 REM LOOP UNLESS FINISHED1500 IF T>0 THEN 14001510 IF N<>5 THEN 14001700 REM CALCULATE SCORE1710 GOSUB 30001720 PRINT1730 PRINT "CONGRATULATIONS! YOU GOT THE TREASURE"1740 PRINT "OUT IN";M0;1750 PRINT "MOVES AND YOU GOT"S+10"POINTS!"1760 PRINT "WANT TO HUNT AGAIN";1770 A$="YN"1771 A2=21780 GOSUB 50001790 ON A1 GOTO 1000,9999,17602000 REM ---------------------------------------------------------2010 REM FOUND?2100 IF T<>N THEN RETURN2110 IF T<0 THEN RETURN2120 IF M6+5>M0 THEN RETURN2200 PRINT "DO YOU WANT TO TAKE IT WITH YOU";2210 A$="YN"2220 A2=22230 GOSUB 50002240 ON A1 GOTO 2300,24002250 PRINT "WELL?...."2260 GOTO 22102300 T=-12310 PRINT2320 PRINT "OK, LETS GET OUT OF HERE!"2330 RETURN2400 PRINT2410 PRINT "WE'LL LEAVE IT HERE AND YOU CAN EXPLORE"2420 PRINT "SOME MORE."2430 M6=M02440 RETURN3000 REM ---------------------------------------------------------3010 REM *** SCORE ***3020 S=O3030 IF T=-1 THEN S=S+53040 IF P=1 THEN S=S+103050 FOR J=2 TO M93060 S=S+W(J)3070 NEXT J3080 RETURN4000 REM ---------------------------------------------------------4010 REM *** PIRATE ***4020 IF N=T2 THEN RETURN4030 IF P=1 THEN RETURN4040 IF T1=T2 THEN RETURN4050 IF T<>-1 THEN RETURN4060 REM HES AT THE EXIT WITH THE TREASURE . ZAP HIM.4065 REM (ARRGH. HOW DID HE GET HERE, ANYWAY?)4070 IF N=16 THEN P=1604080 REM COUNT MOVES SINCE HITTING TIGHT TUNNEL WITH TREASURE4090 IF P1>0 THEN P1=P1+14100 IF N=3 THEN P1=P1+14110 REM GIVE HIM A FEW MORE MOVES , THEN ZAP HIM4120 IF P1<15 THEN RETURN4130 PRINT4140 PRINT"SUDDENLY THE PIRATE LEAPS OUT OF THE"4150 PRINT"GLOOM AND GRABS THE TREASURE FROM YOU!"4160 PRINT"'HAH!', HE SHOUTS, 'YOU FOUND MY"4170 PRINT"TREASURE, DID YOU?! WELL, I'LL HIDE"4180 PRINT"IT BETTER THIS TIME!'"4190 PRINT "AND HE DISAPPEARS INTO THE DARKNESS"4200 PRINT"WITH THE TREASURE."4210 P=14220 T=T24230 RETURN5000 REM ---------------------------------------------------------5010 REM *** INPUT ***5020 REM FIRST CHARACTER OF Al INPUT STRING IS COMPARED WITH5030 REM THE LETTERS OF AS, AND IF THERE IS A MATCH, THE INDEX5040 REM IN AS IS RETURNED IN Al. IF NO MATCH, SIZE(A$)+l IS5050 REM RETURNED.5060 REM GET INPUT STRING5070 INPUT Q$5080 REM USE ONLY FIRST CHARACTER5090 Q$=LEFT$(Q$,1)5100 REM SEARCH FOR THE CHARACTER Q$ IN THE STRING AS. IN THIS VERSION5110 REM OF BASIC , NDX IS THE INDEX FUNCTION , WHICH DOES EXACTLY THAT.5120 REM A1=NDX(A$,Q$)5130 REM BUT CHECK FOR THE CASE WHERE THE CHARACTER WAS NOT FOUND5140 REM IF A1=0 THEN Al=A2+15145 REM RETURN5150 REM IF YOUR VERSION OF BASIC DOESN -T HAVE THE NDX FUNCTION, BUT5160 REM DOES, FOR INSTANCE, HAVE A FUNCTION WHICH WILL PICK A5170 REM PARTICULAR CHARACTER FROM A STRING, SUCH AS MID(A$,A2,1)5180 REM PICKING THE A2-TH CHARACTER FROM AS, YOU MIGHT USE THE5190 REM FOLLOWING CODE.5200 FOR A1=1 TO A25210 IF Q$=MID$(A$,A1,1) THEN RETURN5220 NEXT A15230 Al=A2+15240 RETURN6000 REM --------------------------------------------------------6010 REM *** MOVE ***6020 REM REMEMBER WHERE WE ARE, FOR BOUNCE.6030 N9=N6040 REM SET N8 TO ANYTHING BUT YOU CANT GO THAT WAY6050 N8=06060 REM ASK WHICH WAY6070 GOSUB 70006080 REM REMEMBER WHERE WE ARE, UNLESS A DEAD END6090 IF N=1 THEN 61206100 N0=N6110 A0=A16120 PRINT6130 I=M(A1,N)6200 IF I=-2 THEN I=N96210 IF D<>O THEN PRINT '>>> DEBUG';N;'TO';I6220 IF I<500 THEN 63006230 REM DELAY LOOP TO WASTE SOME TIME6240 I=I-5006250 FOR J=0 TO 1006260 NEXT J6270 GOTO 62006300 ON I/100 GOTO 6340,63706310 REM NORMAL ROUTE-- LESS THAN 1006320 N=I6330 GOTO 64006340 REM N+100. ADD ONE IF CARRYING THE TREASURE6350 N=I-1006355 IF T=- 1 THEN N=N+16360 GOTO 64006370 REM N+200. ADD 1 IF CARRYING TREASURE THE SECOND TIME6380 N=I-2006390 IF T=-1 THEN N=N+P6400 IF N<>1 THEN 65006410 REM DEAD END. TURN IT SO YOU GET OUT THE OTHER WAY6420 FOR J=1 TO 66430 M(J,N)=26440 NEXT J6450 M(7-A0,N)=N06500 REM PRINT OUT THE NODE DESCRIPTION6510 IF N8<>2 THEN GOSUB 80006520 REM REMEMBER WEVE BEEN HERE6530 W(N)=16540 N8=N6600 IF M(1,N)<>-2 THEN 68006610 REM FORCED MOVE, WITH RANDOM DESTINATIONS6620 REM ON THIS VERSION OF BASIC, J=-1 FOLLOWED BY RND(J)6630 REM GETS YOU A NUMBER BETWEEN ZERO AND ONE.6640 REM YOUR VERSION WILL DIFFER, AND THE NEXT FIVE6650 REM LINES WILL HAVE TO BE CHANGED.6660 I=M(6,N)6670 J=16680 IF M(4,N)>100*RND(J) THEN I=M(5,N)6690 J=16700 IF M(2,N)>100*RND(J) THEN I=M(3,N)6710 IF D<>O THEN PRINT " >>> DEBUG BOUNCE TO";I6720 REM NOW HAVE A NEW DESTINATION. GO BACK AND REDO IT6730 GOTO 62006800 RETURN7000 REM --------------------------------------------------------7010 REM *** WHICH WAY? ***7100 PRINT7110 PRINT "WHICH WAY";7120 A$="NEUDWSP"7130 A2=77140 GOSUB 50007150 IF A1<8 THEN 73007160 PRINT "WHICH WAY DO YOU WANT TO GO"7170 REM GIVE INSTRUCTIONS7180 GOSUB 75007190 REM DESCRIBE THE LOCATION AGAIN7200 GOSUB 80007210 GOTO 71007300 IF A1<7 THEN 74007310 REM CALCULATE AND PRINT SCORE7320 GOSUB 30007330 PRINT "YOU HAVE"S"POINTS!"7340 REM START AGAIN7350 GOTO 71007400 RETURN7500 REM --------------------------------------------------------7510 REM SUBROUTINE TO GIVE INSTRUCTIONS7520 PRINT7530 PRINT "TYPE N,S,E,W,U, OR D FOR NORTH, SOUTH,"7550 PRINT "EAST, WEST, UP OR DOWN. TYPE P FOR SCORE"7560 PRINT7570 RETURN8000 REM ---------------------------------------------------------8010 REM DESCRIBE THE CURRENT LOCATION8050 I=INT (N/5)8060 J=N-5*I+18070 REM THERE ARE ENOUGH STATEMENT NUMBERS HERE TO HANDLE NODES8080 REM ZERO THROUGH 49. YOU WILL HAVE TO ADD MORE IF YOU ADD8090 REM NODES 50 AND BEYOND.8100 ON I+1 GOTO 8200,8210,8220,8230,8240,8250,8260,8270,8280,82908200 ON J GOTO 9000,9010,9020,9030,90408210 ON J GOTO 9050,9060,9070,9080,90908220 ON J GOTO 9100,9110,9120,9130,91408230 ON J GOTO 9150,9160,9170,9180,91908240 ON J GOTO 9200,9210,9220,9230,92408250 ON J GOTO 9250,9260,9270,9280,92908260 ON J GOTo 9300,9310,9320,9330,93408270 ON J GOTO 9350,9360,9370,9380,93908280 ON J GOTO 9400,9410,9420,9430,94408290 ON J GOTO 9450,9460,9470,9480,94908400 IF T<>N THEN 85008410 PRINT8420 PRINT " THE TREASURE IS HERE!"8500 IF T<>T2 THEN 86008510 IF T1=T2 THEN 86008520 IF T1<>N THEN 86008530 PRINT8540 PRINT "A NOTE ON THE WALL SAYS"8550 PRINT " 'PIRATES NEVER LEAVE THEIR TREASURE"8560 PRINT "  TWICE IN THE SAME PLACE!'"8600 RETURN9000 REM ---------------------------------------------------------9001 REM FIRST DATA STATEMENT IS NUMBER OF NODES, AND THE 29002 REM HIDING PLACES FOR THE TREASURE.9003 DATA 42,23,129010 DATA 1,0,0,0,0,0,09011 PRINT "YOU'RE AT A DEAD END!"9012 GOTO 84009020 DATA 2,-2,101,-2,0,0,09021 PRINT "YOU CAN'T GO IN THAT DIRECTION"9022 PRINT9023 GOTO 84009030 DATA 3,33,2,1,10,106,49031 PRINT "A TUNNEL GOES NORTH-SOUTH."9032 PRINT "THERE IS AN OPENING TO THE WEST."9037 GOTO 84009040 DATA 4,3,30,2,11,2,19041 PRINT "YOU'RE ON THE BRINK OF A PIT."9047 GOTO 84009050 DATA 5,8,8,15,10,8,169051 PRINT "YOU'RE OUTSIDE THE CAVE."9052 PRINT "GO SOUTH TO ENTER."9053 GOTO 84009060 DATA 6,16,3,2,10,2,29061 PRINT "YOU'RE AT THE HOME OF THE GNOME-KING."9062 PRINT "FORTUNATELY, HE'S GONE FOR THE DAY"9067 GOTO 84009070 DATA 7,-2,101,-2,0,0,09071 PRINT "THE GNOME KING IS HERE!"9072 PRINT "YOU'D BETTER GET OUT!"9073 PRINT9077 GOTO 84009080 DATA 8,18,18,15,10,18,99081 PRINT "YOU'RE LOST IN THE WOODS."9087 GOTO 84009090 DATA 9,-2,33,5,1,0,-29097 GOTO 84009100 DATA 10,- 2,101,-2,0,0,09101 PRINT "YOU'RE NOT GOING TO GET FAR, DIGGING"9102 PRINT "THROUGH ROCK."9103 PRINT9107 GOTO 84009110 DATA 11,1,13,4,2,1,29111 PRINT "YOU'RE AT THE BOTTOM OF A PIT. A LITTLE"9112 PRINT "STREAM FLOWS OVER THE ROCKS HERE."9117 GOTO 84009120 DATA 12,36,2,1,2,1,29121 PRINT "YOU'RE AT A DEAD END!"9127 GOTO 84009130 DATA 13,2,37, 2,1,11,149131 PRINT "YOU'RE AT A WIDE SPOT THERE IS A"9132 PRINT "SOOTY PATCH WHERE SOMEBODY HAS RESTED"9133 PRINT "A TORCH AGAINST THE WALL. THERE ARE"9134 PRINT "JAGGED ROCKS ABOVE YOU."9137 GOTO 84009140 DATA 14,13,1,19,2,31,319141 PRINT "YOU'RE IN A CANYON. HIGH ON THE WALL"9142 PRINT "ABOVE YOU IS SCRATCHED THE MESSAGE"9143 PRINT "  'BILBO WAS HERE'"9147 GOTO 84009150 DATA 15,-2,101,-2,0,0,09151 PRINT "YOU'RE NOT A BIRD. YOU CAN'T FLY!"9152 PRINT9157 GOTO 84009160 DATA 16,5,33,2, 10,1,1069161 PRINT "YOU'RE IN A LOW CHAMBER. A TIGHT TUNNEL"9162 PRINT "GOES EAST, AND YOU CAN WALK TO THE"9163 PRINT "SOUTH OR WEST. THERE IS LIGHT"9164 PRINT "TO THE NORTH."9167 GOTO 84009170 DATA 17,-2,101,-2,0,0,09171 PRINT "IT'S A TIGHT SQUEEZE. YOU CAN'T"9172 PRINT "GET PAST WITH THE TREASURE."9173 PRINT9177 GOTO 84009180 DATA 18,-2,101,8,0,0,09181 PRINT "I DON'T THINK YOU CAN FIND THE CAVE."9187 GOTO 84009190 DATA 19,224,2,2,14,1,429191 PRINT "YOU'RE AT THE TOP OF A CLIMB."9192 PRINT "BELOW YOU A MESSAGE SAYS"9193 PRINT "  'BILBO WAS HERE'"9197 GOTO 84009200 DATA 20,226,1,2,2,25,29201 PRINT "YOU'RE AT THE NORTH SIDE OF A CHASM,"9202 PRINT "TOO WIDE TO JUMP. RINGING ECHOES FROM"9203 PRINT "BELOW ARE THE ONLY INDICATION OF DEPTH."9207 GOTO 84009210 DATA 21,1,226,2,2,38,259211 PRINT "YOU'RE IN XANADU. BELOW YOU"9212 PRINT "ALPH , THE SACRED RIVER RUNS"9213 PRINT "THROUGH CAVERNS MEASURELESS TO MAN,"9214 PRINT "DOWN TO A SUNLESS SEA."9217 GOTO 84009220 DATA 22,-2,33,13,50,29,309227 GOTO 84009230 DATA 23,2,1,2,31,2,29231 PRINT "YOU'RE ON THE LEDGE ABOVE THE GUILLOTINE ROOM."9237 GOTO 84009240 DATA 24,-2,101,19,0,0,09241 PRINT "I HEAR THE GIANT THERE!!!"9242 PRINT "YOU'D BETTER GO BACK!"9243 PRINT9247 GOTO 84009250 DATA 25,21,20,2,2,1,199251 PRINT "YOU'RE IN THE GIANT'S CAVERN. BETTER"9252 PRINT "NOT BE HERE WHEN THE GIANT COMES!"9257 GOTO 84009260 DATA 26,-2,65,-2,50,11,149261 PRINT "YOU'RE IN THE QUEST RESEARCH AND"9262 PRINT "DEVELOPMENT AREA."9263 PRINT9264 PRINT "I'M SORRY, BUT VISITORS ARE NOT"9265 PRINT "ALLOWED. YOU'LL HAVE TO LEAVE."9266 PRINT9267 GOTO 84009270 DATA 27,2,40,2,2,21,209271 PRINT "YOU'RE IN THE CRYSTAL PALACE. THE"9272 PRINT "WALLS RESONATE WITH"9273 PRINT "AWESOME MUSIC."9280 DATA 28,-2,60,221,50,14,199287 GOTO 84009290 DATA 29,2,42,2,13,1,19291 PRINT "YOU'RE AT THE TOP OF A GIANT STALACTITE."9292 PRINT "YOU COULD SLIDE DOWN, BUT YOU COULDN'T"9293 PRINT "CLIMB BACK UP."9297 GOTO 84009300 DATA 30,34,34,2,1,4,29301 PRINT "YOU'RE IN A LITTLE GROTTO. THERE IS A"9302 PRINT "BOOK HERE CALLED JANE'S FIGHTING SHIPS,"9303 PRINT "DATED 1763."9307 GOTO 84009310 DATA 31,14,14,23,2,1,29311 PRINT "YOU'RE IN THE GUILLOTINE ROOM. A SHARP"9312 PRINT "ROCK BALANCES PRECARIOUSLY ON THE"9313 PRINT "LEDGE ABOVE YOU."9317 GOTO 84009320 DATA 32,-2,101,516,0,0,09321 PRINT "YOU'RE IN A CHUTE, SCRAMBLING DOWN THE"9322 PRINT "ROCKS! NO WAY TO STOP! HANG ON!"9323 PRINT9327 GOTO 84009330 DATA 33,2,1,2,1,116,39331 PRINT "THE TIGHT TUNNEL TURNS A CORNER."9332 GOTO 84009340 DATA 34,1,35,2,1,30,309341 PRINT "YOU'RE IN A LITTLE TWISTY MAZE"9347 GOTO 84009350 DATA 35,2,1,2,37,34,369351 PRINT "YOU'RE IN A LITTLE TWISTING MAZE"9357 GOTO 84009360 DATA 36,35,2,1,37,34,129361 PRINT "YOU'RE IN A TWISTING LITTLE MAZE"9367 GOTO 84009370 DATA 37,2,1,35,2,13,29371 PRINT "YOU'RE IN A TWISTY LITTLE MAZE"9377 GOTO 84009380 DATA 38,2,21,2,116,1,29381 PRINT "YOU'RE IN A PREHISTORIC DWELLING. ON"9382 PRINT "THE WALL ARE DRAWINGS OF BISON DONE IN"9383 PRINT "RED CLAY. THE FLOOR IS STREWN WITH"9384 PRINT "BONES, THE REMAINS OF ANCIENT RITUALS."9385 PRINT "A SMALL TUNNEL GOES THROUGH THE FLOOR."9387 GOTO 84009390 DATA 39,2,40,2,32,21,269391 PRINT "YOU'RE IN A BLACK HOLE. THE"9392 PRINT "FORCE OF GRAVITY IS OVERWHELMING."9397 GOTO 84009400 DATA 40,40,40,2,2,40,419401 PRINT "YOU'RE IN THE LABYRINTHE"9407 GOTO 84009410 DATA 41,40,40,40,2,40,399411 PRINT "YOU'RE IN THE LABYRINTHE"9412 PRINT "IT'S VERY DARK IN HERE."9417 GOTO 84009420 DATA 42,28,28,28,28,28,289421 PRINT "YOU'RE IN THE ASHRAM. INCENSE IS HEAVY"9422 PRINT "IN THE AIR, AND ALL DIRECTIONS"9423 PRINT "SEEM THE SAME."9427 GOTO 84009430 REM NO NODES SET UP FOR THESE VALUES . IF YOU GET HERE,9440 REM YOU HAVE A MISTAKE IN A DATA STATEMENT.9450 REM9460 REM9470 REM9480 REM9490 REM9500 REM9510 REM9520 REM9530 REM9999 END